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

#include <cassert>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notification.hh"
#include "com/centreon/engine/notifier.hh"
#include "com/centreon/engine/timezone_locker.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration::applier;

// std::unordered_map<std::string, std::shared_ptr<contact>>
// notifier::current_notifications;

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
                   double check_interval,
                   double retry_interval,
                   double notification_interval,
                   int max_attempts,
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
                   std::string const& timezone)
    : checkable{
          display_name,           check_command,       checks_enabled,
          accept_passive_checks,  check_interval,      retry_interval,
          max_attempts,           check_period,        event_handler,
          event_handler_enabled,  notes,               notes_url,
          action_url,             icon_image,          icon_image_alt,
          flap_detection_enabled, low_flap_threshold,  high_flap_threshold,
          check_freshness,        freshness_threshold, obsess_over,
          timezone},
      notification_period_ptr{nullptr},
      _notifier_type{notifier_type},
      _notification_interval{notification_interval},
      _notification_period{notification_period},
      _first_notification_delay{first_notification_delay},
      _recovery_notification_delay{recovery_notification_delay},
      _notifications_enabled{notifications_enabled},
      _problem_has_been_acknowledged{false},
      _has_been_checked{false},
      _no_more_notifications{false},
      _stalk_type{0},
      _notification_number{0},
      _flap_type{0} {

  if (notification_interval < 0 || check_interval < 0 || retry_interval <= 0) {
    logger(log_config_error, basic)
        << "Error: Invalid notification_interval value for notifier '"
        << display_name << "'";
    throw engine_error() << "Could not register notifier '" << display_name
                         << "'";
  }
}

unsigned long notifier::get_current_event_id() const {
  return _current_event_id;
}

void notifier::set_current_event_id(unsigned long current_event_id) {
  _current_event_id = current_event_id;
}

unsigned long notifier::get_last_event_id() const { return _last_event_id; }

void notifier::set_last_event_id(unsigned long last_event_id) {
  _last_event_id = last_event_id;
}

unsigned long notifier::get_current_problem_id() const {
  return _current_notification_id;
}

void notifier::set_current_problem_id(unsigned long current_problem_id) {
  _current_problem_id = current_problem_id;
}

unsigned long notifier::get_last_problem_id() const { return _last_problem_id; }

void notifier::set_last_problem_id(unsigned long last_problem_id) {
  _last_problem_id = last_problem_id;
}

/**
 * @brief Set the current notification number and update the notifier status.
 *
 * @param num The notification number.
 */
void notifier::set_notification_number(int num) {
  /* set the notification number */
  _current_notification_number = num;

  /* update the status log with the host info */
  update_status(false);
}

bool notifier::_is_notification_viable_normal(
    reason_type type __attribute__((unused)),
    notification_option options) const {
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

  /* if this notifier is currently in a scheduled downtime period, don't send the notification */
  if (is_in_downtime()) {
    logger(dbg_notifications, more)
      << "This notifier is currently in a scheduled downtime, so "
      "we won't send notifications.";
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
      << "This notifier problem has been acknowledged, so we won't send notifications.";
    return false;
  }

  if (!get_notify_on_current_state()) {
    logger(dbg_notifications, more)
      << "This notifier is not configured to notify the state " << get_current_state_as_string();
    return false;
  }

  if (_first_notification_delay > 0 && _notification_number == 0 &&
      get_last_hard_state_change() +
              _first_notification_delay * config->interval_length() >
          now) {
    logger(dbg_notifications, more)
        << "This notifier is configured with a first notification delay, we "
           "won't send notification until timestamp "
        << (_first_notification_delay * config->interval_length());
    return false;
  }

  if (_notification_number >= 1 && _notification_interval > 0) {
    if (_last_notification + _notification_interval * config->interval_length()
        > now) {
      logger(dbg_notifications, more)
        << "This notifier problem has been sent at " << _last_notification
        << " so it won't be sent until "
        << (_notification_interval * config->interval_length());
      return false;
    }
  }

  return true;
}

bool notifier::_is_notification_viable_recovery(
    reason_type type __attribute__((unused)),
    notification_option options __attribute__((unused))) const {
  logger(dbg_functions, basic) << "notifier::is_notification_viable_recovery()";

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

  timeperiod* tp{get_notification_timeperiod()};
  timezone_locker lock{get_timezone()};
  std::time_t now;
  std::time(&now);

  if (!check_time_against_period(now, tp)) {
    logger(dbg_notifications, more)
        << "This notifier shouldn't have notifications sent out "
           "at this time.";
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

  /* Recovery is sent on state OK or UP */
  if (get_current_state_int() != 0 || !get_notify_on(recovery)) {
    logger(dbg_notifications, more)
        << "This notifier state is not UP/OK are is not configured to send a "
           "recovery notification";
    return false;
  }

  if (get_last_hard_state_change() +
          _recovery_notification_delay * config->interval_length() >
      now) {
    logger(dbg_notifications, more)
        << "This notifier is configured with a recovery notification delay. "
        << "It won't send any recovery notification until timestamp "
        << " so it won't be sent until "
        << (get_last_hard_state_change() + _recovery_notification_delay);
    return false;
  }

  if (_notification_number == 0) {
    logger(dbg_notifications, more)
      << "No notification has been sent to announce a problem. So no recovery"
      << " notification will be sent";
    return false;
  }

  if (!_notification[cat_normal]) {
    logger(dbg_notifications, more)
      << "We should not send a notification since no normal notification has"
         " been sent before";
    return false;
  }
  return true;
}

bool notifier::_is_notification_viable_acknowledgement(
    reason_type type __attribute__((unused)),
    notification_option options) const {
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
    logger(dbg_notifications, more)
      << "The notifier is currently OK/UP, so we won't send an acknowledgement.";
    return false;
  }
  return true;
}

bool notifier::_is_notification_viable_flapping(
    reason_type type,
    notification_option options) const {
  logger(dbg_functions, basic)
      << "notifier::is_notification_viable_flapping()";
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

  /* Don't send a notification if is has already been sent */
  if (_notification[cat_flapping] &&
      _notification[cat_flapping]->get_reason() == type) {
    logger(dbg_notifications, more)
        << "We shouldn't notify about a " << tab_notification_str[type]
        << " event: already sent.";
    return false;
  }

  /* Don't send notifications during scheduled downtime */
  if (is_in_downtime()) {
    logger(dbg_notifications, more)
      << "We shouldn't notify about FLAPPING events during scheduled downtime.";
    return false;
  }
  return true;
}

bool notifier::_is_notification_viable_downtime(
    reason_type type __attribute__((unused)),
    notification_option options) const {
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

bool notifier::_is_notification_viable_custom(
    reason_type type __attribute__((unused)),
    notification_option options) const {
  logger(dbg_functions, basic)
      << "notifier::is_notification_viable_custom()";
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
    int state __attribute__((unused))) {
  std::unordered_set<contact*> retval;

  /* Let's start looking at escalations */
  for (std::list<std::shared_ptr<escalation>>::const_iterator
         it{_escalations.begin()},
         end{_escalations.end()};
       it != end;
       ++it) {
    if ((*it)->is_viable(get_current_state_int(), _notification_number)) {
      /* Construction of the set containing contacts to notify. We don't know
       * for the moment if those contacts accept notification. */
      for (contact_map_unsafe::const_iterator cit{(*it)->contacts().begin()},
           cend{(*it)->contacts().end()};
           cit != cend; ++cit)
        if (cit->second->should_be_notified(cat, type, *this))
          retval.insert(cit->second);

      /* For each contact group, we also add its contacts. */
      for (contactgroup_map_unsafe::const_iterator
               cgit{(*it)->contact_groups().begin()},
           cgend{(*it)->contact_groups().end()};
           cgit != cgend; ++cgit) {
        for (contact_map_unsafe::const_iterator
                 cit{cgit->second->get_members().begin()},
             cend{cgit->second->get_members().end()};
             cit != cend; ++cit)
          if (cit->second->should_be_notified(cat, type, *this))
            retval.insert(cit->second);
      }
    }
  }

  if (retval.empty()) {
    /* Construction of the set containing contacts to notify. We don't know
     * for the moment if those contacts accept notification. */
    for (contact_map_unsafe::const_iterator it{get_contacts().begin()},
         end{get_contacts().end()};
         it != end; ++it)
      retval.insert(it->second);

    /* For each contact group, we also add its contacts. */
    for (contactgroup_map_unsafe::const_iterator
             it{get_contactgroups().begin()},
         end{get_contactgroups().end()};
         it != end; ++it) {
      for (contact_map_unsafe::const_iterator
               cit{it->second->get_members().begin()},
           cend{it->second->get_members().end()};
           cit != cend; ++cit)
        retval.insert(cit->second);
    }
  }
  return retval;
}

notifier::notification_category notifier::get_category(reason_type type) const {
  if (type == 99)
    return cat_custom;
  notification_category cat[] = {cat_normal, cat_recovery, cat_acknowledgement, cat_flapping, cat_flapping, cat_flapping, cat_downtime, cat_downtime, cat_downtime, cat_custom};
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
  if (_notification_number == 0)
    ++_notification_number;

  std::shared_ptr<notification> notif{
      new notification(this, type, not_author, not_data, options,
                       _next_notification_id++, _notification_number)};

  /* What are the contacts to notify? */
  std::unordered_set<contact*> to_notify{
      get_contacts_to_notify(cat, type, get_current_state_int())};

  /* Let's make the notification. */
  int retval{notif->execute(std::move(to_notify))};

  if (retval == OK) {
    _last_notification = std::time(nullptr);
    _notification[cat] = notif;
    /* The notification has been sent.
     * Should we increment the notification number? */
    if (cat == cat_normal)
      _notification_number++;
    else if (cat == cat_recovery)
      _notification[cat_normal].reset();
    else
      _notification_number = 0;
  }

  return retval;
}

// int notifier::notify(notifier::reason_type type,
//                     std::string const& not_author,
//                     std::string const& not_data,
//                     int options) {
//  bool increment_notification_number{false};
//  time_t current_time;
//  struct timeval start_time;
//  struct timeval end_time;
//  bool escalated{false};
//  int contacts_notified{0};
//
//  logger(dbg_functions, basic) << "notifier::notify()";
//
//  /* get the current time */
//  time(&current_time);
//  gettimeofday(&start_time, nullptr);
//
//  time_t last_notif{get_last_notification()};
//  logger(dbg_notifications, basic)
//      << "** Notifier Notification Attempt ** Notifier: '" <<
// get_display_name()
//      << "', Notification Type: " << _notifier_type << ", Type: " << type
//      << ", Options: " << options
//      << ", Current State: " << get_current_state_int()
//      << ", Last Notification: " << my_ctime(&last_notif);
//
//  nagios_macros mac;
//  /* Are notifications available for this notifier? */
//  if (!notifications_available(options))
//    return OK;
//
//  /* check viability of sending out a host notification */
//  if (!check_notification_viability(type, options)) {
//    logger(dbg_notifications, basic)
//        << "Notification viability test failed.  No notification will "
//           "be sent out.";
//    return OK;
//  }
//
//  logger(dbg_notifications, basic) << "Notification viability test passed.";
//
//  /* should the notification number be increased? */
//  if (type == notification_normal ||
//      (options & NOTIFICATION_OPTION_INCREMENT)) {
//    _current_notification_number++;
//    increment_notification_number = true;
//  }
//
//  logger(dbg_notifications, more)
//      << "Current notification number: " << _current_notification_number << "
// ("
//      << (increment_notification_number == true ? "incremented" : "unchanged")
//      << ")";
//
//  _current_notification_id = _next_notification_id;
//  _next_notification_id++;
//
//  logger(dbg_notifications, most)
//      << "Creating list of contacts to be notified.";
//
//  /* create the contact notification list for this service */
//  create_notification_list(&mac, options, &escalated);
//
//  /* send data to event broker */
//  end_time.tv_sec = 0L;
//  end_time.tv_usec = 0L;
//  int neb_result{broker_notification_data(
//      NEBTYPE_NOTIFICATION_START, NEBFLAG_NONE, NEBATTR_NONE, _notifier_type,
//      type, start_time, end_time, (void*)this, not_author.c_str(),
//      not_data.c_str(), escalated, 0, nullptr)};
//
//  if (NEBERROR_CALLBACKCANCEL == neb_result) {
//    notifier::current_notifications.clear();
//    return ERROR;
//  } else if (NEBERROR_CALLBACKOVERRIDE == neb_result) {
//    notifier::current_notifications.clear();
//    return OK;
//  }
//
//  /* we have contacts to notify... */
//  if (!notifier::current_notifications.empty()) {
//    /* grab the macro variables */
//    grab_macros_r(&mac);
//
//    /*
//     * if this notification has an author, attempt to lookup the
//     * associated contact
//     */
//    std::shared_ptr<contact> temp_contact;
//    if (!not_author.empty()) {
//      /* see if we can find the contact - first by name, then by alias */
//      contact_map::const_iterator ct_it{contact::contacts.find(not_author)};
//      if (ct_it == contact::contacts.end()) {
//        for (contact_map::const_iterator
//               it{contact::contacts.begin()},
//               end{contact::contacts.end()};
//             it != end; ++it) {
//          if (it->second->get_alias() == not_author) {
//            temp_contact = it->second;
//            break;
//          }
//        }
//      }
//      else
//        temp_contact = ct_it->second;
//    }
//
//    /* get author and comment macros */
//    string::setstr(mac.x[MACRO_NOTIFICATIONAUTHOR], not_author);
//    string::setstr(mac.x[MACRO_NOTIFICATIONCOMMENT], not_data);
//    if (temp_contact) {
//      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORNAME],
//                     temp_contact->get_name());
//      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORALIAS],
//                     temp_contact->get_alias());
//    } else {
//      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORNAME]);
//      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORALIAS]);
//    }
//
//    /*
//     * NOTE: these macros are deprecated and will likely disappear in next
//     * major release
//     * if this is an acknowledgement, get author and comment macros
//     */
//    if (type == notification_acknowledgement) {
//      if (_notifier_type == host_notification) {
//        string::setstr(mac.x[MACRO_HOSTACKAUTHOR], not_author);
//        string::setstr(mac.x[MACRO_HOSTACKCOMMENT], not_data);
//      } else {
//        string::setstr(mac.x[MACRO_SERVICEACKAUTHOR], not_author);
//        string::setstr(mac.x[MACRO_SERVICEACKCOMMENT], not_data);
//      }
//      if (temp_contact) {
//        string::setstr(mac.x[MACRO_SERVICEACKAUTHORNAME],
//                       temp_contact->get_name());
//        string::setstr(mac.x[MACRO_SERVICEACKAUTHORALIAS],
//                       temp_contact->get_alias());
//      } else {
//        string::setstr(mac.x[MACRO_SERVICEACKAUTHORNAME]);
//        string::setstr(mac.x[MACRO_SERVICEACKAUTHORALIAS]);
//      }
//    }
//
//    /* set the notification type macro */
//    if (type == notification_acknowledgement)
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "ACKNOWLEDGEMENT");
//    else if (type == notification_flappingstart)
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "FLAPPINGSTART");
//    else if (type == notification_flappingstop)
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "FLAPPINGSTOP");
//    else if (type == notification_flappingdisabled)
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "FLAPPINGDISABLED");
//    else if (type == notification_downtimestart)
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "DOWNTIMESTART");
//    else if (type == notification_downtimeend)
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "DOWNTIMEEND");
//    else if (type == notification_downtimecancelled)
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "DOWNTIMECANCELLED");
//    else if (type == notification_custom)
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "CUSTOM");
//    else if (recovered())
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "RECOVERY");
//    else
//      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "PROBLEM");
//
//    /* set the notification number macro */
//    if (_notifier_type == host_notification)
//      string::setstr(mac.x[MACRO_HOSTNOTIFICATIONNUMBER],
//                     _current_notification_number);
//    else
//      string::setstr(mac.x[MACRO_SERVICENOTIFICATIONNUMBER],
//                     _current_notification_number);
//
//    /*
//     * the $NOTIFICATIONNUMBER$ macro is maintained for backward compatibility
//     */
//    char const* notificationnumber{
//        _notifier_type == host_notification
//            ? mac.x[MACRO_HOSTNOTIFICATIONNUMBER]
//            : mac.x[MACRO_SERVICENOTIFICATIONNUMBER]};
//
//    string::setstr(mac.x[MACRO_NOTIFICATIONNUMBER],
//                   notificationnumber ? notificationnumber : "");
//
//    /* set the notification id macro */
//    if (_notifier_type == host_notification)
//      string::setstr(mac.x[MACRO_HOSTNOTIFICATIONID],
// _current_notification_id);
//    else
//      string::setstr(mac.x[MACRO_SERVICENOTIFICATIONID],
//                     _current_notification_id);
//
//    /* notify each contact (duplicates have been removed) */
//    for (contact_map::const_iterator
//           it{notifier::current_notifications.begin()},
//           end{notifier::current_notifications.end()};
//         it != end;
//         ++it) {
//      /* grab the macro variables for this contact */
//      grab_contact_macros_r(&mac, it->second.get());
//
//      /* clear summary macros (they are customized for each contact) */
//      clear_summary_macros_r(&mac);
//
//      /* notify this contact */
//      int result = notify_contact(&mac, it->second.get(), type,
//                                  not_author.c_str(), not_data.c_str(),
// options,
//                                  escalated);
//
//      /* keep track of how many contacts were notified */
//      if (result == OK)
//        contacts_notified++;
//    }
//
//    /* free memory allocated to the notification list */
//    notifier::current_notifications.clear();
//    notifier::current_notifications.clear();
//
//    /*
//     * clear summary macros so they will be regenerated without contact
//     * filters when needednext
//     */
//    clear_summary_macros_r(&mac);
//
//    if (type == notification_normal) {
//      /*
//       * adjust last/next notification time and notification flags if we
//       * notified someone
//       */
//      if (contacts_notified > 0) {
//        /* calculate the next acceptable re-notification time */
//        set_next_notification(get_next_notification_time(current_time));
//
//        /*
//         * update the last notification time for this host (this is needed for
//         * scheduling the next problem notification)
//         */
//        set_last_notification(current_time);
//        update_notification_flags();
//
//        time_t time{get_next_notification()};
//        logger(dbg_notifications, basic) << contacts_notified
//                                         << " contacts were notified. "
//                                            "Next possibile notification time:
// "
//                                         << my_ctime(&time);
//      }
//
//      /* we didn't end up notifying anyone */
//      else if (increment_notification_number) {
//        /* adjust current notification number */
//        _current_notification_number--;
//
//        time_t time = get_next_notification();
//        logger(dbg_notifications, basic)
//            << "No contacts were notified.  Next possible "
//               "notification time: "
//            << my_ctime(&time);
//      }
//    }
//
//    logger(dbg_notifications, basic)
//        << contacts_notified << " contacts were notified.";
//  }
//
//  /* there were no contacts, so no notification really occurred... */
//  else {
//    /* adjust notification number, since no notification actually went out */
//    if (increment_notification_number)
//      _current_notification_number--;
//
//    logger(dbg_notifications, basic)
//        << "No contacts were found for notification purposes.  "
//           "No notification was sent out.";
//  }
//
//  /* get the time we finished */
//  gettimeofday(&end_time, nullptr);
//
//  /* send data to event broker */
//  broker_notification_data(NEBTYPE_NOTIFICATION_END, NEBFLAG_NONE,
// NEBATTR_NONE,
//                           _notifier_type, type, start_time, end_time,
//                           (void*)this, not_author.c_str(), not_data.c_str(),
//                           escalated, contacts_notified, nullptr);
//
//  /* update the status log with the host info */
//  update_status(false);
//
//  /* clear volatile macros */
//  clear_volatile_macros_r(&mac);
//
//  /* Update recovery been sent parameter */
//  if(recovered())
//    _recovery_been_sent = true;
//
//  return OK;
//}

// void notifier::inc_next_notification_id() {
//  _next_notification_id++;
//}

void notifier::set_current_notification_id(uint64_t id) {
  _current_notification_id = id;
}

uint64_t notifier::get_current_notification_id() const {
  return _current_notification_id;
}

time_t notifier::get_next_notification() const { return _next_notification; }

void notifier::set_next_notification(time_t next_notification) {
  _next_notification = next_notification;
}

time_t notifier::get_last_notification() const { return _last_notification; }

void notifier::set_last_notification(time_t last_notification) {
  _last_notification = last_notification;
}

void notifier::set_initial_notif_time(time_t notif_time) {
  _initial_notif_time = notif_time;
}

time_t notifier::get_initial_notif_time() const { return _initial_notif_time; }

void notifier::set_acknowledgement_timeout(int timeout) {
  _acknowledgement_timeout = timeout;
}

void notifier::set_last_acknowledgement(time_t ack) {
  _last_acknowledgement = ack;
}

time_t notifier::get_last_acknowledgement() const {
  return _last_acknowledgement;
}

double notifier::get_notification_interval(void) const {
  return _notification_interval;
}

void notifier::set_notification_interval(double notification_interval) {
  _notification_interval = notification_interval;
}

std::string const& notifier::get_notification_period() const {
  return _notification_period;
}

void notifier::set_notification_period(std::string const& notification_period) {
  _notification_period = notification_period;
}

bool notifier::get_notify_on(notification_flag type) const {
  return _out_notification_type & type;
}

uint32_t notifier::get_notify_on() const { return _out_notification_type; }

void notifier::add_notify_on(notification_flag type) {
  _out_notification_type |= type;
}

void notifier::set_notify_on(uint32_t type) { _out_notification_type = type; }

void notifier::remove_notify_on(notification_flag type) {
  _out_notification_type &= ~type;
}

uint32_t notifier::get_first_notification_delay(void) const {
  return _first_notification_delay;
}

void notifier::set_first_notification_delay(uint32_t first_notification_delay) {
  _first_notification_delay = first_notification_delay;
}

uint32_t notifier::get_recovery_notification_delay(void) const {
  return _recovery_notification_delay;
}

void notifier::set_recovery_notification_delay(
    uint32_t recovery_notification_delay) {
  _recovery_notification_delay = recovery_notification_delay;
}

bool notifier::get_notifications_enabled() const {
  return _notifications_enabled;
}

void notifier::set_notifications_enabled(bool notifications_enabled) {
  _notifications_enabled = notifications_enabled;
}

bool notifier::get_notified_on(notification_flag type) const {
  return _current_notifications & type;
}

uint32_t notifier::get_notified_on() const { return _current_notifications; }

void notifier::add_notified_on(notification_flag type) {
  _current_notifications |= type;
}

void notifier::set_notified_on(uint32_t type) { _current_notifications = type; }

void notifier::remove_notified_on(notification_flag type) {
  _current_notifications &= ~type;
}

bool notifier::get_flap_detection_on(notification_flag type) const {
  return _flap_type & type;
}

uint32_t notifier::get_flap_detection_on() const { return _flap_type; }

void notifier::set_flap_detection_on(uint32_t type) { _flap_type = type; }

void notifier::add_flap_detection_on(notification_flag type) {
  _flap_type |= type;
}

bool notifier::get_stalk_on(notification_flag type) const {
  return _stalk_type & type;
}

uint32_t notifier::get_stalk_on() const { return _stalk_type; }

void notifier::set_stalk_on(uint32_t type) { _stalk_type = type; }

void notifier::add_stalk_on(notification_flag type) { _stalk_type |= type; }

uint32_t notifier::get_modified_attributes() const {
  return _modified_attributes;
}

void notifier::set_modified_attributes(uint32_t modified_attributes) {
  _modified_attributes = modified_attributes;
}

void notifier::add_modified_attributes(uint32_t attr) {
  _modified_attributes |= attr;
}

std::list<std::shared_ptr<escalation> >& notifier::get_escalations() {
  return _escalations;
}

std::list<std::shared_ptr<escalation> > const& notifier::get_escalations()
    const {
  return _escalations;
}

void notifier::add_escalation(std::shared_ptr<escalation> e) {
  _escalations.push_back(e);
}

/**
 *  Tests whether or not a contact is an escalated contact for a
 *  particular host.
 *
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
bool notifier::is_escalated_contact(contact* cntct) const {
  if (!cntct)
    return false;

  for (std::shared_ptr<escalation> const& e : get_escalations()) {
    // Search all contacts of this host escalation.
    contact_map_unsafe::const_iterator itt{e->contacts().find(cntct->get_name())};
    if (itt != e->contacts().end()) {
      assert(itt->second == cntct);
      return true;
    }

    // Search all contactgroups of this host escalation.
    for (contactgroup_map_unsafe::iterator itt(e->contact_groups().begin()),
         end(e->contact_groups().begin());
         itt != end;
         ++itt)
      if (itt->second->get_members().find(cntct->get_name()) !=
          itt->second->get_members().end())
        return true;
  }
  return false;
}

/**
 * @brief Create a list of contacts to be notified for this notifier.
 * Remove also duplicates.
 *
 * @param mac
 * @param options
 * @param escalated
 *
 */
// void notifier::create_notification_list(nagios_macros* mac,
//                                        int options,
//                                        bool* escalated) {
//  logger(dbg_functions, basic) << "notifier::create_notification_list()";
//
//  /* see if this notification should be escalated */
//  bool escalate_notification{should_notification_be_escalated()};
//
//  /* set the escalation flag */
//  *escalated = escalate_notification;
//
//  /* make sure there aren't any leftover contacts */
//  current_notifications.clear();
//
//  /* set the escalation macro */
//  string::setstr(mac->x[MACRO_NOTIFICATIONISESCALATED],
// escalate_notification);
//
//  if (options & NOTIFICATION_OPTION_BROADCAST)
//    logger(dbg_notifications, more)
//        << "This notification will be BROADCAST to all (escalated and "
//           "normal) contacts...";
//
//  /* use escalated contacts for this notification */
//  if (escalate_notification || (options & NOTIFICATION_OPTION_BROADCAST)) {
//    logger(dbg_notifications, more)
//        << "Adding contacts from notifier escalation(s) to "
//           "notification list.";
//
//    for (std::shared_ptr<escalation> const& e : get_escalations()) {
//      /* see if this escalation if valid for this notification */
//      if (!is_valid_escalation_for_notification(e, options))
//        continue;
//
//      logger(dbg_notifications, most)
//          << "Adding individual contacts from notifier escalation(s) "
//             "to notification list.";
//
//      /* add all individual contacts for this escalation entry */
//      for (contact_map::const_iterator itt{e->contacts().begin()},
//           end{e->contacts().end()};
//           itt != end; ++itt)
//        add_notification(mac, itt->second);
//
//      logger(dbg_notifications, most)
//          << "Adding members of contact groups from notifier escalation(s) "
//             "to notification list.";
//
//      /* add all contacts that belong to contactgroups for this escalation */
//      for (contactgroup_map::iterator itt(e->contact_groups.begin()),
//           end(e->contact_groups.end());
//           itt != end; ++itt) {
//        logger(dbg_notifications, most)
//            << "Adding members of contact group '" << itt->first
//            << "' for notifier escalation to notification list.";
//
//        if (!itt->second)
//          continue;
//        for (contact_map::const_iterator
//               itm{itt->second->get_members().begin()},
//               endm{itt->second->get_members().end()};
//             itm != endm; ++itm) {
//          if (!itm->second)
//            continue;
//          add_notification(mac, itm->second);
//        }
//      }
//    }
//  }
//
//  /* use normal, non-escalated contacts for this notification */
//  if (!escalate_notification || (options & NOTIFICATION_OPTION_BROADCAST)) {
//    logger(dbg_notifications, more)
//        << "Adding normal contacts for notifier to notification list.";
//
//    /* add all individual contacts for this notifier */
//    for (contact_map::iterator it(this->contacts.begin()),
//         end(this->contacts.end());
//         it != end; ++it)
//      add_notification(mac, it->second);
//
//    /* add all contacts that belong to contactgroups for this notifier */
//    for (contactgroup_map::iterator it{this->contact_groups.begin()},
//         end{this->contact_groups.end()};
//         it != end; ++it) {
//      logger(dbg_notifications, most)
//          << "Adding members of contact group '" << it->first
//          << "' for notifier to notification list.";
//
//      if (!it->second)
//        continue;
//      for (contact_map::const_iterator
//             itm{it->second->get_members().begin()},
//             endm{it->second->get_members().end()};
//           itm != endm; ++itm) {
//        if (!itm->second)
//          continue;
//        add_notification(mac, itm->second);
//      }
//    }
//  }
//}

/**
 *  Checks to see whether a notification should be escalated.
 *
 *  @param[in] svc Service.
 *
 *  @return true if the notification should be escalated, false if
 *          it should not.
 */
bool notifier::should_notification_be_escalated() const {
  // Debug.
  logger(dbg_functions, basic)
      << "notifier::should_notification_be_escalated()";

  for (std::shared_ptr<escalation> const& e : get_escalations()) {
    // We found a matching entry, so escalate this notification!
    if (is_valid_escalation_for_notification(e, notification_option_none)) {
      logger(dbg_notifications, more)
          << "Notifier notification WILL be escalated.";
      return true;
    }
  }

  logger(dbg_notifications, more)
      << "Notifier notification will NOT be escalated.";
  return false;
}

bool notifier::get_problem_has_been_acknowledged() const {
  return _problem_has_been_acknowledged;
}

void notifier::set_problem_has_been_acknowledged(
    bool problem_has_been_acknowledged) {
  _problem_has_been_acknowledged = problem_has_been_acknowledged;
}

bool notifier::get_no_more_notifications() const {
  return _no_more_notifications;
}

void notifier::set_no_more_notifications(bool no_more_notifications) {
  _no_more_notifications = no_more_notifications;
}

///* add a new notification to the list in memory */
// int notifier::add_notification(nagios_macros* mac, std::shared_ptr<contact>
// cntct) {
//  logger(dbg_functions, basic)
//    << "add_notification()";
//
//  if (!cntct)
//    return ERROR;
//
//  logger(dbg_notifications, most)
//    << "Adding contact '" << cntct->get_name() << "' to notification list.";
//
//  /* don't add anything if this contact is already on the notification list */
//  if (notifier::current_notifications.find(cntct->get_name()) !=
// notifier::current_notifications.end())
//    return OK;
//
//  /* Add contact to notification list */
//  current_notifications.insert({cntct->get_name(), cntct});
//
//  /* add contact to notification recipients macro */
//  if (!mac->x[MACRO_NOTIFICATIONRECIPIENTS])
//    string::setstr(mac->x[MACRO_NOTIFICATIONRECIPIENTS], cntct->get_name());
//  else {
//    std::string buffer(mac->x[MACRO_NOTIFICATIONRECIPIENTS]);
//    buffer += ",";
//    buffer += cntct->get_name();
//    string::setstr(mac->x[MACRO_NOTIFICATIONRECIPIENTS], buffer);
//  }
//
//  return OK;
//}

int notifier::get_current_notification_number() const {
  return _current_notification_number;
}

void notifier::set_current_notification_number(int number) {
  _current_notification_number = number;
}

/**
 *  Get the next notification id.
 *
 * @return a long unsigned integer.
 */
uint64_t notifier::get_next_notification_id() const {
  return _next_notification_id;
}

notifier::notifier_type notifier::get_notifier_type() const {
  return _notifier_type;
}

std::unordered_map<std::string, contact*>& notifier::get_contacts() {
  return _contacts;
}

std::unordered_map<std::string, contact*> const& notifier::get_contacts() const {
  return _contacts;
}

contactgroup_map_unsafe& notifier::get_contactgroups() {
  return _contact_groups;
}

contactgroup_map_unsafe const& notifier::get_contactgroups() const {
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
bool is_contact_for_notifier(com::centreon::engine::notifier* notif, contact* cntct) {
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
       it != end; ++it)
    if (it->second->get_members().find(cntct->get_name()) ==
        it->second->get_members().end())
      return true;

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

  /* check the event handler command */
  if (!get_event_handler().empty()) {
    size_t pos{get_event_handler().find_first_of('!')};
    std::string cmd_name{get_event_handler().substr(0, pos)};

    command_map::iterator cmd_found{
        commands::command::commands.find(cmd_name)};

    if (cmd_found == commands::command::commands.end() || !cmd_found->second) {
        logger(log_verification_error, basic)
        << "Error: Event handler command '" << cmd_name
        << "' specified for host '" << get_display_name()
        << "' not defined anywhere";
        errors++;
    }
    else
      /* save the pointer to the event handler command for later */
      set_event_handler_ptr(cmd_found->second.get());
  }

  /* hosts that don't have check commands defined shouldn't ever be checked... */
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
    }
    else
      /* save the pointer to the check command for later */
      set_check_command_ptr(cmd_found->second.get());
  }

  if (get_check_period().empty()) {
    logger(log_verification_error, basic)
      << "Warning: Notifier '" << get_display_name()
      << "' has no check time period defined!";
    warnings++;
    check_period_ptr = nullptr;
  }
  else {
    timeperiod_map::const_iterator
      found_it{timeperiod::timeperiods.find(get_check_period())};

    if (found_it == timeperiod::timeperiods.end() || !found_it->second) {
      logger(log_verification_error, basic)
        << "Error: Check period '" << get_check_period()
        << "' specified for host '" << get_display_name()
        << "' is not defined anywhere!";
      errors++;
      check_period_ptr = nullptr;
    }
    else
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
    }
    else
      /* save the pointer to the contact */
      it->second = found_it->second.get();
  }

  /* check all contact groups */
  for (contactgroup_map_unsafe::const_iterator it{get_contactgroups().begin()},
       end{get_contactgroups().end()};
       it != end; ++it) {
    // Find the contact group.
    contactgroup_map::const_iterator found_it{
        contactgroup::contactgroups.find(it->first)};

    if (found_it == contactgroup::contactgroups.end() ||
        !found_it->second.get()) {
      logger(log_verification_error, basic)
          << "Error: Contact group '" << it->first << "' specified in host '"
          << get_display_name() << "' is not defined anywhere!";
      errors++;
    }
  }

  // Check notification timeperiod.
  if (!get_notification_period().empty()) {
    timeperiod_map::const_iterator
      found_it{timeperiod::timeperiods.find(get_notification_period())};

    if (found_it == timeperiod::timeperiods.end() || !found_it->second.get()) {
      logger(log_verification_error, basic)
        << "Error: Notification period '" << get_notification_period()
        << "' specified for notifier '" << get_display_name()
        << "' is not defined anywhere!";
      errors++;
      notification_period_ptr = nullptr;
    }
    else
      // Save the pointer to the notification timeperiod for later.
      notification_period_ptr = found_it->second.get();
  }
  else if (get_notifications_enabled()) {
    logger(log_verification_error, basic)
      << "Warning: Notifier '" << get_display_name()
      << "' has no notification time period defined!";
    warnings++;
    notification_period_ptr = nullptr;
  }

  w += warnings;
  e += errors;

  if (e)
    throw engine_error() << "Cannot resolve host '" << get_display_name() << "'";
}
