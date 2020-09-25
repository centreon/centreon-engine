/*
** Copyright 2011-2019 Centreon
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "com/centreon/engine/notifier.hh"

#include <cassert>

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/hostescalation.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notification.hh"
#include "com/centreon/engine/timezone_locker.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration::applier;

std::array<std::string, 9> const notifier::tab_notification_str{{
    "NORMAL",
    "RECOVERY",
    "ACKNOWLEDGEMENT",
    "FLAPPINGSTART",
    "FLAPPINGSTOP",
    "FLAPPINGDISABLED",
    "DOWNTIMESTART",
    "DOWNTIMEEND",
    "DOWNTIMECANCELLED",
}};

std::array<std::string, 2> const notifier::tab_state_type{{"SOFT", "HARD"}};

std::array<notifier::is_viable, 6> const notifier::_is_notification_viable{{
    &notifier::_is_notification_viable_normal,
    &notifier::_is_notification_viable_recovery,
    &notifier::_is_notification_viable_acknowledgement,
    &notifier::_is_notification_viable_flapping,
    &notifier::_is_notification_viable_downtime,
    &notifier::_is_notification_viable_custom,
}};

uint64_t notifier::_next_notification_id{1L};

notifier::notifier(notifier::notifier_type notifier_type,
                   std::string const& display_name,
                   std::string const& check_command,
                   bool checks_enabled,
                   bool accept_passive_checks,
                   uint32_t check_interval,
                   uint32_t retry_interval,
                   uint32_t notification_interval,
                   int max_attempts,
                   int32_t notify,
                   int32_t stalk,
                   uint32_t first_notification_delay,
                   uint32_t recovery_notification_delay,
                   std::string const& notification_period,
                   bool notifications_enabled,
                   std::string const& check_period,
                   std::string const& event_handler,
                   bool event_handler_enabled,
                   std::string const& notes,
                   std::string const& notes_url,
                   std::string const& action_url,
                   std::string const& icon_image,
                   std::string const& icon_image_alt,
                   bool flap_detection_enabled,
                   double low_flap_threshold,
                   double high_flap_threshold,
                   bool check_freshness,
                   int freshness_threshold,
                   bool obsess_over,
                   std::string const& timezone,
                   bool retain_status_information,
                   bool retain_nonstatus_information)
    : checkable{display_name,
                check_command,
                checks_enabled,
                accept_passive_checks,
                check_interval,
                retry_interval,
                max_attempts,
                check_period,
                event_handler,
                event_handler_enabled,
                notes,
                notes_url,
                action_url,
                icon_image,
                icon_image_alt,
                flap_detection_enabled,
                low_flap_threshold,
                high_flap_threshold,
                check_freshness,
                freshness_threshold,
                obsess_over,
                timezone},
      _notifier_type{notifier_type},
      _stalk_type{stalk},
      _flap_type{0},
      _current_event_id{0},
      _last_event_id{0},
      _current_problem_id{0},
      _last_problem_id{0},
      _initial_notif_time{0},
      _acknowledgement_timeout{0},
      _last_acknowledgement{0},
      _out_notification_type{notify},
      _current_notifications{0},
      _notification_interval{notification_interval},
      _modified_attributes{0},
      _current_notification_id{0UL},
      _next_notification{0UL},
      _last_notification{0UL},
      _notification_period{notification_period},
      _notification_period_ptr{nullptr},
      _first_notification_delay{first_notification_delay},
      _recovery_notification_delay{recovery_notification_delay},
      _notifications_enabled{notifications_enabled},
      _problem_has_been_acknowledged{false},
      _has_been_checked{false},
      _no_more_notifications{false},
      _flapping_comment_id{0},
      _check_options{CHECK_OPTION_NONE},
      _acknowledgement_type{ACKNOWLEDGEMENT_NONE},
      _retain_status_information{retain_status_information},
      _retain_nonstatus_information{retain_nonstatus_information},
      _is_being_freshened{false},
      _notification_number{0},
      _notification{{}},
      _state_history{{}},
      _pending_flex_downtime{0} {
  if (retry_interval <= 0) {
    logger(log_config_error, basic)
        << "Error: Invalid notification_interval value for notifier '"
        << display_name << "'";
    throw engine_error() << "Could not register notifier '" << display_name
                         << "'";
  }
}

notifier::~notifier() {
  checks::checker::forget(this);
}

unsigned long notifier::get_current_event_id() const {
  return _current_event_id;
}

void notifier::set_current_event_id(unsigned long current_event_id) noexcept {
  _current_event_id = current_event_id;
}

unsigned long notifier::get_last_event_id() const noexcept {
  return _last_event_id;
}

void notifier::set_last_event_id(unsigned long last_event_id) noexcept {
  _last_event_id = last_event_id;
}

unsigned long notifier::get_current_problem_id() const noexcept {
  return _current_problem_id;
}

void notifier::set_current_problem_id(
    unsigned long current_problem_id) noexcept {
  _current_problem_id = current_problem_id;
}

unsigned long notifier::get_last_problem_id() const noexcept {
  return _last_problem_id;
}

void notifier::set_last_problem_id(unsigned long last_problem_id) noexcept {
  _last_problem_id = last_problem_id;
}

/**
 * @brief Set the current notification number and update the notifier status.
 *
 * @param num The notification number.
 */
void notifier::set_notification_number(int num) {
  /* set the notification number */
  _notification_number = num;

  /* update the status log with the host info */
  update_status(false);
}

bool notifier::_is_notification_viable_normal(reason_type type
                                              __attribute__((unused)),
                                              notification_option options) {
  logger(dbg_functions, basic) << "notifier::is_notification_viable_normal()";

  /* forced notifications bust through everything */
  if (options & notification_option_forced) {
    logger(dbg_notifications, more)
        << "This is a forced notification, so we'll send it out.";
    return true;
  }

  /* are notifications enabled? */
  if (!config->enable_notifications()) {
    logger(dbg_notifications, more)
        << "Notifications are disabled, so notifications will "
           "not be sent out.";
    return false;
  }

  /* are notifications temporarily disabled for this notifier? */
  if (!get_notifications_enabled()) {
    logger(dbg_notifications, more)
        << "Notifications are temporarily disabled for "
           "this notifier, so we won't send one out.";
    return false;
  }

  /* if this notifier is currently in a scheduled downtime period, don't send
   * the notification */
  if (is_in_downtime()) {
    logger(dbg_notifications, more)
        << "This notifier is currently in a scheduled downtime, so "
           "we won't send notifications.";
    return false;
  }

  /* On volatile services notifications are always sent */
  if (get_is_volatile()) {
    logger(dbg_notifications, more)
        << "This is a volatile service notification, so it is sent.";
    return true;
  }

  timeperiod* tp{get_notification_timeperiod()};
  timezone_locker lock{get_timezone()};
  time_t now;
  time(&now);

  if (!check_time_against_period(now, tp)) {
    logger(dbg_notifications, more)
        << "This notifier shouldn't have notifications sent out "
           "at this time.";
    return false;
  }

  /* if this notifier is flapping, don't send the notification */
  if (get_is_flapping()) {
    logger(dbg_notifications, more)
        << "This notifier is flapping, so we won't send notifications.";
    return false;
  }

  if (get_state_type() != hard) {
    logger(dbg_notifications, more)
        << "This notifier is in soft state, so we won't send notifications.";
    return false;
  }

  if (get_problem_has_been_acknowledged()) {
    logger(dbg_notifications, more)
        << "This notifier problem has been acknowledged, so we won't send "
           "notifications.";
    return false;
  }

  if (get_current_state_int() == 0) {
    logger(dbg_notifications, more)
        << "We don't send a normal notification when the state is ok/up";
    return false;
  }

  if (!get_notify_on_current_state()) {
    logger(dbg_notifications, more)
        << "This notifier is unable to notify the state "
        << get_current_state_as_string()
        << ": not configured for that or, for a service, its host may be down";
    return false;
  }

  if (_first_notification_delay > 0 && !_notification[cat_normal] &&
      get_last_hard_state_change() +
              _first_notification_delay * config->interval_length() >
          now) {
    logger(dbg_notifications, more)
        << "This notifier is configured with a first notification delay, we "
           "won't send notification until timestamp "
        << (_first_notification_delay * config->interval_length());
    return false;
  }

  if (!authorized_by_dependencies(dependency::notification)) {
    logger(dbg_notifications, more)
        << "This notifier won't send any notification since it depends on"
           " another notifier that has already sent one";
    return false;
  }

  if (_notification[cat_normal]) {
    /* In the case of a state change, we don't care of the notification interval
     * and we notify as soon as we can */
    if (get_last_hard_state_change() <= _last_notification) {
      uint32_t notification_interval =
          !_notification[cat_normal]
              ? _notification_interval
              : _notification[cat_normal]->get_notification_interval();

      if (notification_interval == 0) {
        logger(dbg_notifications, more)
            << "This notifier problem has already been sent at "
            << _last_notification
            << " so, since the notification interval is 0, it won't be sent"
            << " anymore";
        return false;
      } else if (notification_interval > 0) {
        if (_last_notification +
                notification_interval * config->interval_length() >
            now) {
          logger(dbg_notifications, more)
              << "This notifier problem has been sent at " << _last_notification
              << " so it won't be sent until "
              << (notification_interval * config->interval_length());
          return false;
        }
      }
    }
  }
  return true;
}

bool notifier::_is_notification_viable_recovery(reason_type type
                                                __attribute__((unused)),
                                                notification_option options
                                                __attribute__((unused))) {
  logger(dbg_functions, basic) << "notifier::is_notification_viable_recovery()";
  bool retval{true};
  bool send_later{false};

  /* are notifications enabled? */
  if (!config->enable_notifications()) {
    logger(dbg_notifications, more)
        << "Notifications are disabled, so notifications will "
           "not be sent out.";
    retval = false;
  }
  /* are notifications temporarily disabled for this notifier? */
  else if (!get_notifications_enabled()) {
    logger(dbg_notifications, more)
        << "Notifications are temporarily disabled for "
           "this notifier, so we won't send one out.";
    retval = false;
  } else {
    timeperiod* tp{get_notification_timeperiod()};
    timezone_locker lock{get_timezone()};
    std::time_t now;
    std::time(&now);

    if (!check_time_against_period(now, tp)) {
      logger(dbg_notifications, more)
          << "This notifier shouldn't have notifications sent out "
             "at this time.";
      retval = false;
    }
    /* if this notifier is currently in a scheduled downtime period, don't send
     * the notification */
    else if (is_in_downtime()) {
      logger(dbg_notifications, more)
          << "This notifier is currently in a scheduled downtime, so "
             "we won't send notifications.";
      retval = false;
    }
    /* if this notifier is flapping, don't send the notification */
    else if (get_is_flapping()) {
      logger(dbg_notifications, more)
          << "This notifier is flapping, so we won't send notifications.";
      retval = false;
      send_later = true;
    } else if (get_state_type() != hard) {
      logger(dbg_notifications, more)
          << "This notifier is in soft state, so we won't send notifications.";
      retval = false;
      send_later = true;
    }
    /* Recovery is sent on state OK or UP */
    else if (get_current_state_int() != 0) {
      logger(dbg_notifications, more)
          << "This notifier state is not UP/OK to send a recovery notification";
      retval = false;
      send_later = true;
    } else if (!(get_notify_on(up) || get_notify_on(ok))) {
      logger(dbg_notifications, more)
          << "This notifier is not configured to send a recovery notification";
      retval = false;
      send_later = false;
    } else if (get_last_hard_state_change() +
                   _recovery_notification_delay * config->interval_length() >
               now) {
      logger(dbg_notifications, more)
          << "This notifier is configured with a recovery notification delay. "
          << "It won't send any recovery notification until timestamp "
          << " so it won't be sent until "
          << (get_last_hard_state_change() + _recovery_notification_delay);
      retval = false;
      send_later = true;
    } else if (_notification_number == 0) {
      logger(dbg_notifications, more) << "No notification has been sent to "
                                         "announce a problem. So no recovery"
                                      << " notification will be sent";
      retval = false;
    } else if (!_notification[cat_normal]) {
      logger(dbg_notifications, more) << "We should not send a notification "
                                         "since no normal notification has"
                                         " been sent before";
      retval = false;
    }
  }

  if (!retval) {
    if (!send_later) {
      _notification[cat_normal].reset();
      _notification_number = 0;
    }
  }

  return retval;
}

bool notifier::_is_notification_viable_acknowledgement(
    reason_type type __attribute__((unused)),
    notification_option options) {
  logger(dbg_functions, basic)
      << "notifier::is_notification_viable_acknowledgement()";
  /* forced notifications bust through everything */
  if (options & notification_option_forced) {
    logger(dbg_notifications, more)
        << "This is a forced notification, so we'll send it out.";
    return true;
  }

  /* are notifications enabled? */
  if (!config->enable_notifications()) {
    logger(dbg_notifications, more)
        << "Notifications are disabled, so notifications will "
           "not be sent out.";
    return false;
  }

  /* are notifications temporarily disabled for this notifier? */
  if (!get_notifications_enabled()) {
    logger(dbg_notifications, more)
        << "Notifications are temporarily disabled for "
           "this notifier, so we won't send one out.";
    return false;
  }

  if (get_current_state_int() == 0) {
    logger(dbg_notifications, more) << "The notifier is currently OK/UP, so we "
                                       "won't send an acknowledgement.";
    return false;
  }
  return true;
}

bool notifier::_is_notification_viable_flapping(reason_type type,
                                                notification_option options) {
  logger(dbg_functions, basic) << "notifier::is_notification_viable_flapping()";
  /* forced notifications bust through everything */
  if (options & notification_option_forced) {
    logger(dbg_notifications, more)
        << "This is a forced notification, so we'll send it out.";
    return true;
  }

  /* are notifications enabled? */
  if (!config->enable_notifications()) {
    logger(dbg_notifications, more)
        << "Notifications are disabled, so notifications will "
           "not be sent out.";
    return false;
  }

  /* are notifications temporarily disabled for this notifier? */
  if (!get_notifications_enabled()) {
    logger(dbg_notifications, more)
        << "Notifications are temporarily disabled for "
           "this notifier, so we won't send one out.";
    return false;
  }

  /* Don't send a notification if we are not supposed to */
  notification_flag f;
  if (type == reason_flappingstart)
    f = flappingstart;
  else if (type == reason_flappingstop)
    f = flappingstop;
  else
    f = flappingdisabled;

  if (!get_notify_on(f)) {
    logger(dbg_notifications, more)
        << "We shouldn't notify about " << tab_notification_str[type]
        << " events for this notifier.";
    return false;
  }

  /* Don't send a start notification if a flapping notification is already there
   */
  if (type == reason_flappingstart && _notification[cat_flapping]) {
    logger(dbg_notifications, more)
        << "A flapping notification is already running, we can not send "
           "a start notification now.";
    return false;
    /* Don't send a stop/cancel notification if the previous flapping
     * notification is not a start flapping */
  } else if (type == reason_flappingstop || type == reason_flappingdisabled) {
    if (!_notification[cat_flapping] ||
        _notification[cat_flapping]->get_reason() != reason_flappingstart) {
      logger(dbg_notifications, more)
          << "A stop or cancellation flapping notification can only be sent "
             "after a start flapping notification.";
      return false;
    }
  }

  /* Don't send a notification if the same has already been sent previously. */
  if (_notification[cat_flapping] &&
      _notification[cat_flapping]->get_reason() == type) {
    logger(dbg_notifications, more)
        << "We shouldn't notify about a " << tab_notification_str[type]
        << " event: already sent.";
    return false;
  }

  /* Don't send notifications during scheduled downtime */
  if (is_in_downtime()) {
    logger(dbg_notifications, more) << "We shouldn't notify about FLAPPING "
                                       "events during scheduled downtime.";
    return false;
  }
  return true;
}

bool notifier::_is_notification_viable_downtime(reason_type type
                                                __attribute__((unused)),
                                                notification_option options) {
  logger(dbg_functions, basic) << "notifier::is_notification_viable_downtime()";

  /* forced notifications bust through everything */
  if (options & notification_option_forced) {
    logger(dbg_notifications, more)
        << "This is a forced notification, so we'll send it out.";
    return true;
  }

  /* are notifications enabled? */
  if (!config->enable_notifications()) {
    logger(dbg_notifications, more)
        << "Notifications are disabled, so notifications will "
           "not be sent out.";
    return false;
  }

  /* are notifications temporarily disabled for this notifier? */
  if (!get_notifications_enabled()) {
    logger(dbg_notifications, more)
        << "Notifications are temporarily disabled for "
           "this notifier, so we won't send one out.";
    return false;
  }

  if (!config->enable_notifications()) {
    logger(dbg_notifications, more)
        << "Notifications are disabled, so notifications won't be sent out.";
    return false;
  }

  /* Don't send a notification if we are not supposed to */
  if (!get_notify_on(downtime)) {
    logger(dbg_notifications, more)
        << "We shouldn't notify about DOWNTIME events for this notifier.";
    return false;
  }

  /* Don't send notifications during scheduled downtime (in the case of a
   * service, we don't care of the host, so the use of
   * get_scheduled_downtime_depth()) */
  if (get_scheduled_downtime_depth() > 0) {
    logger(dbg_notifications, more) << "We shouldn't notify about DOWNTIME "
                                       "events during scheduled downtime.";
    return false;
  }
  return true;
}

bool notifier::_is_notification_viable_custom(reason_type type
                                              __attribute__((unused)),
                                              notification_option options) {
  logger(dbg_functions, basic) << "notifier::is_notification_viable_custom()";
  /* forced notifications bust through everything */
  if (options & notification_option_forced) {
    logger(dbg_notifications, more)
        << "This is a forced notification, so we'll send it out.";
    return true;
  }

  /* are notifications enabled? */
  if (!config->enable_notifications()) {
    logger(dbg_notifications, more)
        << "Notifications are disabled, so notifications will "
           "not be sent out.";
    return false;
  }

  /* are notifications temporarily disabled for this notifier? */
  if (!get_notifications_enabled()) {
    logger(dbg_notifications, more)
        << "Notifications are temporarily disabled for "
           "this notifier, so we won't send one out.";
    return false;
  }

  /* Don't send notifications during scheduled downtime */
  if (is_in_downtime()) {
    logger(dbg_notifications, more)
        << "We shouldn't send a CUSTOM notification during scheduled downtime.";
    return false;
  }
  return true;
}

std::unordered_set<contact*> notifier::get_contacts_to_notify(
    notification_category cat,
    reason_type type,
    uint32_t& notification_interval) {
  std::unordered_set<contact*> retval;
  bool escalated{false};
  uint32_t notif_interv{_notification_interval};

  /* Let's start looking at escalations */
  for (std::list<escalation*>::const_iterator it{_escalations.begin()},
       end{_escalations.end()};
       it != end; ++it) {
    if ((*it)->is_viable(get_current_state_int(), _notification_number)) {
      /* Among escalations, we choose the smallest notification interval. */
      if (escalated) {
        if ((*it)->get_notification_interval() < notif_interv)
          notif_interv = (*it)->get_notification_interval();
      } else {
        /* Here is the first escalation, so we take its notification_interval.
         */
        escalated = true;
        notif_interv = (*it)->get_notification_interval();
      }

      /* For each contact group, we also add its contacts. */
      for (contactgroup_map_unsafe::const_iterator
               cgit{(*it)->get_contactgroups().begin()},
           cgend{(*it)->get_contactgroups().end()};
           cgit != cgend; ++cgit) {
        for (contact_map_unsafe::const_iterator
                 cit{cgit->second->get_members().begin()},
             cend{cgit->second->get_members().end()};
             cit != cend; ++cit) {
          assert(cit->second);
          if (cit->second->should_be_notified(cat, type, *this))
            retval.insert(cit->second);
        }
      }
    }
  }

  if (!escalated) {
    /* Construction of the set containing contacts to notify. We don't know
     * for the moment if those contacts accept notification. */
    for (contact_map_unsafe::const_iterator it{get_contacts().begin()},
         end{get_contacts().end()};
         it != end; ++it) {
      assert(it->second);
      retval.insert(it->second);
    }

    /* For each contact group, we also add its contacts. */
    for (contactgroup_map_unsafe::const_iterator
             it{get_contactgroups().begin()},
         end{get_contactgroups().end()};
         it != end; ++it) {
      for (contact_map_unsafe::const_iterator
               cit{it->second->get_members().begin()},
           cend{it->second->get_members().end()};
           cit != cend; ++cit) {
        assert(cit->second);
        retval.insert(cit->second);
      }
    }
  }

  notification_interval = notif_interv;
  return retval;
}

notifier::notification_category notifier::get_category(reason_type type) {
  if (type == 99)
    return cat_custom;
  notification_category cat[] = {
      cat_normal,   cat_recovery, cat_acknowledgement, cat_flapping,
      cat_flapping, cat_flapping, cat_downtime,        cat_downtime,
      cat_downtime, cat_custom};
  return cat[static_cast<size_t>(type)];
}

bool notifier::is_notification_viable(notification_category cat,
                                      reason_type type,
                                      notification_option options) {
  return (this->*(_is_notification_viable[cat]))(type, options);
}

int notifier::notify(notifier::reason_type type,
                     std::string const& not_author,
                     std::string const& not_data,
                     notification_option options) {
  logger(dbg_functions, basic) << "notifier::notify()";
  notification_category cat{get_category(type)};

  /* Has this notification got sense? */
  if (!is_notification_viable(cat, type, options))
    return OK;

  /* For a first notification, we store what type of notification we try to
   * send and we fix the notification number to 1. */
  if (type != reason_recovery)
    ++_notification_number;

  /* What are the contacts to notify? */
  uint32_t notification_interval;
  std::unordered_set<contact*> to_notify{
      get_contacts_to_notify(cat, type, notification_interval)};

  _current_notification_id = _next_notification_id++;
  std::shared_ptr<notification> notif{std::make_shared<notification>(
      this, type, not_author, not_data, options, _current_notification_id,
      _notification_number, notification_interval)};

  /* Let's make the notification. */
  int retval{notif->execute(to_notify)};

  if (retval == OK) {
    _last_notification = std::time(nullptr);
    _notification[cat] = notif;
    /* The notification has been sent.
     * Should we increment the notification number? */
    if (cat != cat_normal) {
      switch (cat) {
        case cat_recovery:
          _notification[cat_normal].reset();
          _notification[cat_recovery].reset();
          break;
        case cat_flapping:
          if (type == reason_flappingstop || type == reason_flappingdisabled)
            _notification[cat_flapping].reset();
          break;
        case cat_downtime:
          if (type == reason_downtimeend || type == reason_downtimecancelled)
            _notification[cat_downtime].reset();
          break;
        default:
          _notification[cat].reset();
      }
      /* In case of an acknowledgement, we must keep the _notification_number
       * otherwise the recovery notification won't be sent when needed. */
      if (cat != cat_acknowledgement)
        _notification_number = 0;
    }
  }

  return retval;
}

void notifier::set_current_notification_id(uint64_t id) noexcept {
  _current_notification_id = id;
}

uint64_t notifier::get_current_notification_id() const noexcept {
  return _current_notification_id;
}

time_t notifier::get_next_notification() const noexcept {
  return _next_notification;
}

void notifier::set_next_notification(time_t next_notification) noexcept {
  _next_notification = next_notification;
}

time_t notifier::get_last_notification() const noexcept {
  return _last_notification;
}

void notifier::set_last_notification(time_t last_notification) noexcept {
  _last_notification = last_notification;
}

void notifier::set_initial_notif_time(time_t notif_time) noexcept {
  _initial_notif_time = notif_time;
}

time_t notifier::get_initial_notif_time() const noexcept {
  return _initial_notif_time;
}

void notifier::set_acknowledgement_timeout(int timeout) noexcept {
  _acknowledgement_timeout = timeout;
}

void notifier::set_last_acknowledgement(time_t ack) noexcept {
  _last_acknowledgement = ack;
}

time_t notifier::get_last_acknowledgement() const noexcept {
  return _last_acknowledgement;
}

uint32_t notifier::get_notification_interval(void) const noexcept {
  return _notification_interval;
}

void notifier::set_notification_interval(
    uint32_t notification_interval) noexcept {
  _notification_interval = notification_interval;
}

std::string const& notifier::get_notification_period() const noexcept {
  return _notification_period;
}

void notifier::set_notification_period(
    std::string const& notification_period) noexcept {
  _notification_period = notification_period;
}

bool notifier::get_notify_on(notification_flag type) const noexcept {
  return _out_notification_type & type;
}

uint32_t notifier::get_notify_on() const noexcept {
  return _out_notification_type;
}

void notifier::add_notify_on(notification_flag type) noexcept {
  _out_notification_type |= type;
}

void notifier::set_notify_on(uint32_t type) noexcept {
  _out_notification_type = type;
}

void notifier::remove_notify_on(notification_flag type) noexcept {
  _out_notification_type &= ~type;
}

uint32_t notifier::get_first_notification_delay(void) const noexcept {
  return _first_notification_delay;
}

void notifier::set_first_notification_delay(
    uint32_t first_notification_delay) noexcept {
  _first_notification_delay = first_notification_delay;
}

uint32_t notifier::get_recovery_notification_delay(void) const noexcept {
  return _recovery_notification_delay;
}

void notifier::set_recovery_notification_delay(
    uint32_t recovery_notification_delay) noexcept {
  _recovery_notification_delay = recovery_notification_delay;
}

bool notifier::get_notifications_enabled() const noexcept {
  return _notifications_enabled;
}

void notifier::set_notifications_enabled(bool notifications_enabled) noexcept {
  _notifications_enabled = notifications_enabled;
}

bool notifier::get_notified_on(notification_flag type) const noexcept {
  return _current_notifications & type;
}

uint32_t notifier::get_notified_on() const noexcept {
  return _current_notifications;
}

void notifier::add_notified_on(notification_flag type) noexcept {
  _current_notifications |= type;
}

void notifier::set_notified_on(uint32_t type) noexcept {
  _current_notifications = type;
}

void notifier::remove_notified_on(notification_flag type) noexcept {
  _current_notifications &= ~type;
}

bool notifier::get_flap_detection_on(notification_flag type) const noexcept {
  return _flap_type & type;
}

uint32_t notifier::get_flap_detection_on() const noexcept {
  return _flap_type;
}

void notifier::set_flap_detection_on(uint32_t type) noexcept {
  _flap_type = type;
}

void notifier::add_flap_detection_on(notification_flag type) noexcept {
  _flap_type |= type;
}

bool notifier::get_stalk_on(notification_flag type) const noexcept {
  return _stalk_type & type;
}

uint32_t notifier::get_stalk_on() const noexcept {
  return _stalk_type;
}

void notifier::set_stalk_on(uint32_t type) noexcept {
  _stalk_type = type;
}

void notifier::add_stalk_on(notification_flag type) noexcept {
  _stalk_type |= type;
}

uint32_t notifier::get_modified_attributes() const noexcept {
  return _modified_attributes;
}

void notifier::set_modified_attributes(uint32_t modified_attributes) noexcept {
  _modified_attributes = modified_attributes;
}

void notifier::add_modified_attributes(uint32_t attr) noexcept {
  _modified_attributes |= attr;
}

std::list<escalation*>& notifier::get_escalations() noexcept {
  return _escalations;
}

std::list<escalation*> const& notifier::get_escalations() const noexcept {
  return _escalations;
}

uint64_t notifier::get_flapping_comment_id(void) const noexcept {
  return _flapping_comment_id;
}

void notifier::set_flapping_comment_id(uint64_t comment_id) noexcept {
  _flapping_comment_id = comment_id;
}

int notifier::get_check_options(void) const noexcept {
  return _check_options;
}

void notifier::set_check_options(int option) noexcept {
  _check_options = option;
}

int notifier::get_acknowledgement_type(void) const noexcept {
  return _acknowledgement_type;
}

void notifier::set_acknowledgement_type(int acknowledge_type) noexcept {
  _acknowledgement_type = acknowledge_type;
}

int notifier::get_retain_status_information(void) const noexcept {
  return _retain_status_information;
}

void notifier::set_retain_status_information(
    bool retain_status_informations) noexcept {
  _retain_status_information = retain_status_informations;
}

bool notifier::get_retain_nonstatus_information(void) const noexcept {
  return _retain_nonstatus_information;
}

void notifier::set_retain_nonstatus_information(
    bool retain_non_status_informations) noexcept {
  _retain_nonstatus_information = retain_non_status_informations;
}

bool notifier::get_is_being_freshened(void) const noexcept {
  return _is_being_freshened;
}

void notifier::set_is_being_freshened(bool freshened) noexcept {
  _is_being_freshened = freshened;
}

bool notifier::get_problem_has_been_acknowledged() const noexcept {
  return _problem_has_been_acknowledged;
}

void notifier::set_problem_has_been_acknowledged(
    bool problem_has_been_acknowledged) noexcept {
  _problem_has_been_acknowledged = problem_has_been_acknowledged;
}

bool notifier::get_no_more_notifications() const noexcept {
  return _no_more_notifications;
}

void notifier::set_no_more_notifications(bool no_more_notifications) noexcept {
  _no_more_notifications = no_more_notifications;
}

int notifier::get_notification_number() const noexcept {
  return _notification_number;
}

/**
 *  Get the next notification id.
 *
 * @return a long unsigned integer.
 */
uint64_t notifier::get_next_notification_id() const noexcept {
  return _next_notification_id;
}

notifier::notifier_type notifier::get_notifier_type() const noexcept {
  return _notifier_type;
}

std::unordered_map<std::string, contact*>& notifier::get_contacts() noexcept {
  return _contacts;
}

std::unordered_map<std::string, contact*> const& notifier::get_contacts() const
    noexcept {
  return _contacts;
}

contactgroup_map_unsafe& notifier::get_contactgroups() noexcept {
  return _contact_groups;
}

contactgroup_map_unsafe const& notifier::get_contactgroups() const noexcept {
  return _contact_groups;
}

/**
 *  Tests whether a contact is a contact for a particular notifier.
 *
 *  @param[in] notif Target notifier.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
bool is_contact_for_notifier(com::centreon::engine::notifier* notif,
                             contact* cntct) {
  if (!notif || !cntct)
    return false;

  // Search all individual contacts of this host.
  for (contact_map_unsafe::const_iterator it{notif->get_contacts().begin()},
       end{notif->get_contacts().end()};
       it != end; ++it)
    if (it->second == cntct)
      return true;

  for (contactgroup_map_unsafe::const_iterator
           it{notif->get_contactgroups().begin()},
       end{notif->get_contactgroups().end()};
       it != end; ++it) {
    assert(it->second);
    if (it->second->get_members().find(cntct->get_name()) ==
        it->second->get_members().end())
      return true;
  }

  return false;
}

/**
 *  This method resolves pointers involved in this notifier life. If a pointer
 *  cannot be resolved, an exception is thrown.
 *
 * @param w Warnings given by the method.
 * @param e Errors given by the method. An exception is thrown is at less an
 * error is rised.
 */
void notifier::resolve(int& w, int& e) {
  int warnings{0}, errors{0};

  /* This list will be filled in {hostescalation,serviceescalation}::resolve */
  _escalations.clear();

  /* check the event handler command */
  if (!get_event_handler().empty()) {
    size_t pos{get_event_handler().find_first_of('!')};
    std::string cmd_name{get_event_handler().substr(0, pos)};

    command_map::iterator cmd_found{commands::command::commands.find(cmd_name)};

    if (cmd_found == commands::command::commands.end() || !cmd_found->second) {
      logger(log_verification_error, basic)
          << "Error: Event handler command '" << cmd_name
          << "' specified for host '" << get_display_name()
          << "' not defined anywhere";
      errors++;
    } else
      /* save the pointer to the event handler command for later */
      set_event_handler_ptr(cmd_found->second.get());
  }

  /* hosts that don't have check commands defined shouldn't ever be checked...
   */
  if (!get_check_command().empty()) {
    size_t pos{get_check_command().find_first_of('!')};
    std::string cmd_name{get_check_command().substr(0, pos)};

    command_map::iterator cmd_found{commands::command::commands.find(cmd_name)};

    if (cmd_found == commands::command::commands.end() || !cmd_found->second) {
      logger(log_verification_error, basic)
          << "Error: Notifier check command '" << cmd_name
          << "' specified for host '" << get_display_name()
          << "' is not defined anywhere!",
          errors++;
    } else
      /* save the pointer to the check command for later */
      set_check_command_ptr(cmd_found->second.get());
  }

  if (get_check_period().empty()) {
    logger(log_verification_error, basic)
        << "Warning: Notifier '" << get_display_name()
        << "' has no check time period defined!";
    warnings++;
    check_period_ptr = nullptr;
  } else {
    timeperiod_map::const_iterator found_it{
        timeperiod::timeperiods.find(get_check_period())};

    if (found_it == timeperiod::timeperiods.end() || !found_it->second) {
      logger(log_verification_error, basic)
          << "Error: Check period '" << get_check_period()
          << "' specified for host '" << get_display_name()
          << "' is not defined anywhere!";
      errors++;
      check_period_ptr = nullptr;
    } else
      /* save the pointer to the check timeperiod for later */
      check_period_ptr = found_it->second.get();
  }

  /* check all contacts */
  for (contact_map_unsafe::iterator it{get_contacts().begin()},
       end{get_contacts().end()};
       it != end; ++it) {
    contact_map::const_iterator found_it{contact::contacts.find(it->first)};
    if (found_it == contact::contacts.end() || !found_it->second.get()) {
      logger(log_verification_error, basic)
          << "Error: Contact '" << it->first << "' specified in notifier '"
          << get_display_name() << "' is not defined anywhere!";
      errors++;
    } else
      /* save the pointer to the contact */
      it->second = found_it->second.get();
  }

  /* check all contact groups */
  for (contactgroup_map_unsafe::iterator it{get_contactgroups().begin()},
       end{get_contactgroups().end()};
       it != end; ++it) {
    // Find the contact group.
    contactgroup_map::const_iterator found_it{
        contactgroup::contactgroups.find(it->first)};

    if (found_it == contactgroup::contactgroups.end()) {
      logger(log_verification_error, basic)
          << "Error: Contact group '" << it->first << "' specified in host '"
          << get_display_name() << "' is not defined anywhere!";
      errors++;
    } else
      it->second = found_it->second.get();
  }

  // Check notification timeperiod.
  if (!get_notification_period().empty()) {
    timeperiod_map::const_iterator found_it{
        timeperiod::timeperiods.find(get_notification_period())};

    if (found_it == timeperiod::timeperiods.end() || !found_it->second.get()) {
      logger(log_verification_error, basic)
          << "Error: Notification period '" << get_notification_period()
          << "' specified for notifier '" << get_display_name()
          << "' is not defined anywhere!";
      errors++;
      _notification_period_ptr = nullptr;
    } else
      // Save the pointer to the notification timeperiod for later.
      _notification_period_ptr = found_it->second.get();
  } else if (get_notifications_enabled()) {
    logger(log_verification_error, basic)
        << "Warning: Notifier '" << get_display_name()
        << "' has no notification time period defined!";
    warnings++;
    _notification_period_ptr = nullptr;
  }

  w += warnings;
  e += errors;

  if (e)
    throw engine_error() << "Cannot resolve host '" << get_display_name()
                         << "'";
}

std::array<int, MAX_STATE_HISTORY_ENTRIES> const& notifier::get_state_history()
    const {
  return _state_history;
}

std::array<int, MAX_STATE_HISTORY_ENTRIES>& notifier::get_state_history() {
  return _state_history;
}

std::array<std::shared_ptr<notification>, 6> const&
notifier::get_current_notifications() const {
  return _notification;
}

int notifier::get_pending_flex_downtime() const {
  return _pending_flex_downtime;
}

void notifier::inc_pending_flex_downtime() noexcept {
  ++_pending_flex_downtime;
}

void notifier::dec_pending_flex_downtime() noexcept {
  --_pending_flex_downtime;
}

/**
 * @brief Calculates next acceptable re-notification time for this notifier.
 *
 * @param offset
 *
 * @return a timestamp
 */
time_t notifier::get_next_notification_time(time_t offset) {
  bool have_escalated_interval{false};

  logger(dbg_functions, basic) << "notifier::get_next_notification_time()";
  logger(dbg_notifications, most)
      << "Calculating next valid notification time...";

  /* default notification interval */
  uint32_t interval_to_use{_notification_interval};

  logger(dbg_notifications, most) << "Default interval: " << interval_to_use;

  /*
   * search all the escalation entries for valid matches for this service (at
   * its current notification number)
   */
  for (escalation const* e : get_escalations()) {
    /* interval < 0 means to use non-escalated interval */
    if (e->get_notification_interval() < 0.0)
      continue;

    /* skip this entry if it isn't appropriate */
    if (!is_valid_escalation_for_notification(e, notification_option_none))
      continue;

    logger(dbg_notifications, most)
        << "Found a valid escalation w/ interval of "
        << e->get_notification_interval();

    /*
     * if we haven't used a notification interval from an escalation yet,
     * use this one
     */
    if (!have_escalated_interval) {
      have_escalated_interval = true;
      interval_to_use = e->get_notification_interval();
    }
    /* else use the shortest of all valid escalation intervals */
    else if (e->get_notification_interval() < interval_to_use)
      interval_to_use = e->get_notification_interval();

    logger(dbg_notifications, most) << "New interval: " << interval_to_use;
  }

  /*
   * if notification interval is 0, we shouldn't send any more problem
   * notifications (unless service is volatile)
   */
  if (interval_to_use == 0.0 && !get_is_volatile())
    set_no_more_notifications(true);
  else
    set_no_more_notifications(false);

  logger(dbg_notifications, most) << "Interval used for calculating next valid "
                                     "notification time: "
                                  << interval_to_use;

  /* calculate next notification time */
  time_t next_notification{
      offset +
      static_cast<time_t>(interval_to_use * config->interval_length())};

  return next_notification;
}

void notifier::set_flap_type(uint32_t type) noexcept {
  _flap_type = type;
}

timeperiod* notifier::get_notification_period_ptr() const noexcept {
  return _notification_period_ptr;
}

int notifier::get_acknowledgement_timeout() const noexcept {
  return _acknowledgement_timeout;
}

void notifier::set_notification_period_ptr(timeperiod* tp) noexcept {
  _notification_period_ptr = tp;
}

/**
 *  This method is called by the retention to restitute a notification.
 *
 * @param idx The index of the notification
 * @param value The notification under the form of a string
 */
void notifier::set_notification(int32_t idx, std::string const& value) {
  if (value.empty())
    return;

  char const* v = value.c_str();
  if (strncmp(v, "type: ", 6)) {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the line should start "
           "with 'type: '";
    return;
  }

  v += 6;
  char* next;
  reason_type type = static_cast<reason_type>(strtol(v, &next, 10));
  if (next == v || *next != ',' || next[1] != ' ') {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the separator between "
        << "two fields is ', '";
    return;
  }

  v = next + 2;
  if (strncmp(v, "author: ", 8)) {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the expected field "
           " after 'type' is 'author'";
    return;
  }

  v += 8;
  for (next = const_cast<char*>(v); *next && *next != ','; next++)
    ;
  std::string author(v, next - v);

  v = next + 2;
  if (strncmp(v, "options: ", 9)) {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the expected field "
           " after 'author' is 'options'";
    return;
  }

  v += 9;
  int options = strtol(v, &next, 10);
  if (next == v || *next != ',' || next[1] != ' ') {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the separator between "
        << "two fields is ', '";
    return;
  }

  v = next + 2;
  if (strncmp(v, "escalated: ", 11)) {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the expected field "
           " after 'options' is 'escalated'";
    return;
  }

  v += 11;
  bool escalated = static_cast<bool>(strtol(v, &next, 10));
  if (next == v || *next != ',' || next[1] != ' ') {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the separator between "
        << "two fields is ', '";
    return;
  }

  v = next + 2;
  if (strncmp(v, "id: ", 4)) {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the expected field "
           " after 'escalated' is 'id'";
    return;
  }

  v += 4;
  int id = strtol(v, &next, 10);
  if (next == v || *next != ',' || next[1] != ' ') {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the separator between "
        << "two fields is ', '";
    return;
  }

  v = next + 2;
  if (strncmp(v, "number: ", 8)) {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the expected field "
           " after 'id' is 'number'";
    return;
  }

  v += 8;
  int number = strtol(v, &next, 10);
  if (next == v || *next != ',' || next[1] != ' ') {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the separator between "
        << "two fields is ', '";
    return;
  }

  v = next + 2;
  if (strncmp(v, "interval: ", 10)) {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the expected field "
           " after 'number' is 'interval'";
    return;
  }

  v += 10;
  int interval = strtol(v, &next, 10);
  if (next == v) {
    logger(log_config_error, basic)
        << "Error: Bad format in the notification part, the 'interval' value "
           "should be an integer";
    return;
  }

  v += 13;
  std::set<std::string> contacts;
  for (const char* s = v; *s; ++s) {
    if ((*s == ',' || *s == '\n') && v != s) {
      contacts.emplace(v, s);
      v = s + 1;
    }
  }

  std::shared_ptr<notification> notif{
      std::make_shared<notification>(this, type, author, "", options, id,
                                     number, interval, escalated, contacts)};
  _notification[idx] = notif;
}
