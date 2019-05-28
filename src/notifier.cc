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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/notifier.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration::applier;

std::array<std::string, 8> const notifier::tab_notification_str {{
  "NORMAL",
  "ACKNOWLEDGEMENT",
  "FLAPPINGSTART",
  "FLAPPINGSTOP",
  "FLAPPINGDISABLED",
  "DOWNTIMESTART",
  "DOWNTIMEEND",
  "DOWNTIMECANCELLED",
}};

std::array<std::string, 2> const notifier::tab_state_type {{
  "SOFT",
  "HARD"
}};

uint64_t notifier::_next_notification_id{0};

notifier::notifier(int notification_type,
                   std::string const& display_name,
                   std::string const& check_command,
                   int initial_state,
                   double check_interval,
                   double retry_interval,
                   int max_attempts,
                   std::string const& notification_period,
                   std::string const& check_period,
                   std::string const& action_url,
                   std::string const& icon_image,
                   std::string const& icon_image_alt)
    : _notification_type{notification_type},
      _display_name{display_name},
      _check_command{check_command},
      _initial_state{initial_state},
      _check_interval{check_interval},
      _retry_interval{retry_interval},
      _max_attempts{max_attempts},
      _notification_period{notification_period},
      _check_period{check_period},
      _icon_image{icon_image},
      _icon_image_alt{icon_image_alt} {
  if (check_interval < 0) {
    logger(log_config_error, basic)
        << "Error: Invalid check_interval value for notifier '" << display_name
        << "'";
    throw engine_error() << "Could not register notifier '" << display_name
                         << "'";
  }

  if (check_interval < 0 || retry_interval <= 0) {
    logger(log_config_error, basic)
        << "Error: Invalid max_attempts, check_interval, retry_interval"
           ", or notification_interval value for notifier '"
        << display_name << "'";
    throw engine_error() << "Could not register notifier '" << display_name
                         << "'";
  }

  if (max_attempts <= 0) {
    logger(log_config_error, basic)
        << "Error: Invalid max_check_attempts value for notifier '"
        << display_name << "'";
    throw engine_error() << "Could not register notifier '" << display_name
                         << "'";
  }
}

std::string const& notifier::get_display_name() const {
  return _display_name;
}

void notifier::set_display_name(std::string const& display_name) {
  _display_name = display_name;
}

std::string const& notifier::get_check_command() const {
  return _check_command;
}

void notifier::set_check_command(std::string const& check_command) {
  _check_command = check_command;
}

int notifier::get_initial_state() const {
  return _initial_state;
}

void notifier::set_initial_state(int initial_state) {
  _initial_state = initial_state;
}

double notifier::get_check_interval() const {
  return _check_interval;
}

void notifier::set_check_interval(double check_interval) {
  _check_interval = check_interval;
}

double notifier::get_retry_interval() const {
  return _retry_interval;
}

void notifier::set_retry_interval(double retry_interval) {
  _retry_interval = retry_interval;
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

int notifier::get_max_attempts() const {
  return _max_attempts;
}

void notifier::set_max_attempts(int max_attempts) {
  _max_attempts = max_attempts;
}

int notifier::notify(unsigned int type,
    std::string const& not_author,
    std::string const& not_data,
    int options) {
  bool increment_notification_number{false};
  time_t current_time;
  struct timeval start_time;
  struct timeval end_time;
  bool escalated{false};
  int contacts_notified{0};

  logger(dbg_functions, basic) << "notifier::notify()";

  /* get the current time */
  time(&current_time);
  gettimeofday(&start_time, nullptr);

  time_t last_notif{get_last_notification()};
  logger(dbg_notifications, basic)
      << "** Notifier Notification Attempt ** Notifier: '" << get_display_name()
      << "', Notification Type: " << _notification_type
      << ", Type: " << type
      << ", Options: " << options << ", Current State: " << this->get_current_state()
      << ", Last Notification: " << my_ctime(&last_notif);

  nagios_macros mac;
  /* check viability of sending out a host notification */
  if (check_notification_viability(type, options) == ERROR) {
    logger(dbg_notifications, basic)
        << "Notification viability test failed.  No notification will "
           "be sent out.";
    return OK;
  }

  logger(dbg_notifications, basic) << "Notification viability test passed.";

  /* should the notification number be increased? */
  if (type == NOTIFICATION_NORMAL ||
      (options & NOTIFICATION_OPTION_INCREMENT)) {
    _current_notification_number++;
    increment_notification_number = true;
  }

  logger(dbg_notifications, more)
      << "Current notification number: "
      << _current_notification_number << " ("
      << (increment_notification_number == true ? "incremented" : "unchanged")
      << ")";

  _current_notification_id = _next_notification_id;
  _next_notification_id++;

  logger(dbg_notifications, most)
      << "Creating list of contacts to be notified.";

  /* create the contact notification list for this service */
  memset(&mac, 0, sizeof(mac));
  create_notification_list(&mac, options, &escalated);

  /* send data to event broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  int neb_result{broker_notification_data(
      NEBTYPE_NOTIFICATION_START, NEBFLAG_NONE, NEBATTR_NONE,
      _notification_type, type, start_time, end_time, (void*)this, not_author.c_str(), not_data.c_str(), escalated,
      0, nullptr)};

  if (NEBERROR_CALLBACKCANCEL == neb_result) {
    free_notification_list();
    return ERROR;
  } else if (NEBERROR_CALLBACKOVERRIDE == neb_result) {
    free_notification_list();
    return OK;
  }

  /* we have contacts to notify... */
  if (notification_list) {
    /* grab the macro variables */
    grab_macros_r(&mac);

    /*
     * if this notification has an author, attempt to lookup the
     * associated contact
     */
    contact* temp_contact{nullptr};
    if (!not_author.empty()) {
      /* see if we can find the contact - first by name, then by alias */
      if ((temp_contact = state::instance().find_contact(not_author)) ==
          nullptr) {
        for (std::unordered_map<std::string,
                                std::shared_ptr<contact>>::const_iterator
                 it{state::instance().contacts().begin()},
                 end{state::instance().contacts().end()};
             it != end; ++it) {
          if (it->second->get_alias() == not_author) {
            temp_contact = it->second.get();
            break;
          }
        }
      }
    }

    /* get author and comment macros */
    string::setstr(mac.x[MACRO_NOTIFICATIONAUTHOR], not_author);
    string::setstr(mac.x[MACRO_NOTIFICATIONCOMMENT], not_data);
    if (temp_contact) {
      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORNAME],
                     temp_contact->get_name());
      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORALIAS],
                     temp_contact->get_alias());
    } else {
      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORNAME]);
      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORALIAS]);
    }

    /*
     * NOTE: these macros are deprecated and will likely disappear in next
     * major release
     * if this is an acknowledgement, get author and comment macros
     */
    if (type == NOTIFICATION_ACKNOWLEDGEMENT) {
      if (_notification_type == HOST_NOTIFICATION) {
        string::setstr(mac.x[MACRO_HOSTACKAUTHOR], not_author);
        string::setstr(mac.x[MACRO_HOSTACKCOMMENT], not_data);
      }
      else {
        string::setstr(mac.x[MACRO_SERVICEACKAUTHOR], not_author);
        string::setstr(mac.x[MACRO_SERVICEACKCOMMENT], not_data);
      }
      if (temp_contact) {
        string::setstr(mac.x[MACRO_SERVICEACKAUTHORNAME],
                       temp_contact->get_name());
        string::setstr(mac.x[MACRO_SERVICEACKAUTHORALIAS],
                       temp_contact->get_alias());
      } else {
        string::setstr(mac.x[MACRO_SERVICEACKAUTHORNAME]);
        string::setstr(mac.x[MACRO_SERVICEACKAUTHORALIAS]);
      }
    }

    /* set the notification type macro */
    if (type == NOTIFICATION_ACKNOWLEDGEMENT)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "ACKNOWLEDGEMENT");
    else if (type == NOTIFICATION_FLAPPINGSTART)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "FLAPPINGSTART");
    else if (type == NOTIFICATION_FLAPPINGSTOP)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "FLAPPINGSTOP");
    else if (type == NOTIFICATION_FLAPPINGDISABLED)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "FLAPPINGDISABLED");
    else if (type == NOTIFICATION_DOWNTIMESTART)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "DOWNTIMESTART");
    else if (type == NOTIFICATION_DOWNTIMEEND)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "DOWNTIMEEND");
    else if (type == NOTIFICATION_DOWNTIMECANCELLED)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "DOWNTIMECANCELLED");
    else if (type == NOTIFICATION_CUSTOM)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "CUSTOM");
    else if ((_notification_type == HOST_NOTIFICATION && get_current_state() == HOST_UP)
             || (_notification_type == SERVICE_NOTIFICATION && get_current_state() == STATE_OK))
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "RECOVERY");
    else
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "PROBLEM");

    /* set the notification number macro */
    if (_notification_type == HOST_NOTIFICATION)
      string::setstr(mac.x[MACRO_HOSTNOTIFICATIONNUMBER],
                     _current_notification_number);
    else
      string::setstr(mac.x[MACRO_SERVICENOTIFICATIONNUMBER],
                     _current_notification_number);

    /*
     * the $NOTIFICATIONNUMBER$ macro is maintained for backward compatibility
     */
    char const* notificationnumber{_notification_type == HOST_NOTIFICATION ?
      mac.x[MACRO_HOSTNOTIFICATIONNUMBER] : mac.x[MACRO_SERVICENOTIFICATIONNUMBER]};

    string::setstr(mac.x[MACRO_NOTIFICATIONNUMBER],
                   notificationnumber ? notificationnumber : "");

    /* set the notification id macro */
    if (_notification_type == HOST_NOTIFICATION)
      string::setstr(mac.x[MACRO_HOSTNOTIFICATIONID],
                     _current_notification_id);
    else
      string::setstr(mac.x[MACRO_SERVICENOTIFICATIONID],
                     _current_notification_id);

    notification* temp_notification{nullptr};
    /* notify each contact (duplicates have been removed) */
    for (temp_notification = notification_list; temp_notification != nullptr;
         temp_notification = temp_notification->next) {
      /* grab the macro variables for this contact */
      grab_contact_macros_r(&mac, temp_notification->cntct);

      /* clear summary macros (they are customized for each contact) */
      clear_summary_macros_r(&mac);

      /* notify this contact */
      int result = notify_contact(&mac, temp_notification->cntct, type,
                                  not_author.c_str(), not_data.c_str(), options,
                                  escalated);

      /* keep track of how many contacts were notified */
      if (result == OK)
        contacts_notified++;
    }

    /* free memory allocated to the notification list */
    free_notification_list();

    /*
     * clear summary macros so they will be regenerated without contact
     * filters when needednext
     */
    clear_summary_macros_r(&mac);

    if (type == NOTIFICATION_NORMAL) {
      /*
       * adjust last/next notification time and notification flags if we
       * notified someone
       */
      if (contacts_notified > 0) {
        /* calculate the next acceptable re-notification time */
        set_next_notification(
            get_next_notification_time(current_time));

        /*
         * update the last notification time for this host (this is needed for
         * scheduling the next problem notification)
         */
        set_last_notification(current_time);
        update_notification_flags();

        time_t time{get_next_notification()};
        logger(dbg_notifications, basic) << contacts_notified
          << " contacts were notified. "
             "Next possibile notification time: "
          << my_ctime(&time);
      }

      /* we didn't end up notifying anyone */
      else if (increment_notification_number) {
        /* adjust current notification number */
        _current_notification_number--;

        time_t time = get_next_notification();
        logger(dbg_notifications, basic)
            << "No contacts were notified.  Next possible "
               "notification time: "
            << my_ctime(&time);
      }
    }

    logger(dbg_notifications, basic)
      << contacts_notified << " contacts were notified.";
  }

  /* there were no contacts, so no notification really occurred... */
  else {
    /* adjust notification number, since no notification actually went out */
    if (increment_notification_number)
      _current_notification_number--;

    logger(dbg_notifications, basic)
        << "No contacts were found for notification purposes.  "
           "No notification was sent out.";
  }

  /* get the time we finished */
  gettimeofday(&end_time, nullptr);

  /* send data to event broker */
  broker_notification_data(NEBTYPE_NOTIFICATION_END, NEBFLAG_NONE, NEBATTR_NONE,
                           _notification_type, type, start_time, end_time,
                           (void*)this, not_author.c_str(), not_data.c_str(), escalated,
                           contacts_notified, nullptr);

  /* update the status log with the host info */
  update_status(false);

  /* clear volatile macros */
  clear_volatile_macros_r(&mac);

  /* Update recovery been sent parameter */
  if ((_notification_type == HOST_NOTIFICATION &&
       get_current_state() == HOST_UP) ||
      (_notification_type == SERVICE_NOTIFICATION &&
       get_current_state() == STATE_OK))
    _recovery_been_sent = true;

  return OK;
}

void notifier::inc_next_notification_id() {
  _next_notification_id++;
}

void notifier::set_current_notification_id(uint64_t id) {
  _current_notification_id = id;
}

uint64_t notifier::get_current_notification_id() const {
  return _current_notification_id;
}

time_t notifier::get_next_notification() const {
  return _next_notification;
}

void notifier::set_next_notification(time_t next_notification) {
  _next_notification = next_notification;
}

time_t notifier::get_last_notification() const {
  return _last_notification;
}

void notifier::set_last_notification(time_t last_notification) {
  _last_notification = last_notification;
}

int notifier::get_current_state() const {
  return _current_state;
}

void notifier::set_current_state(int current_state) {
  _current_state = current_state;
}

void notifier::set_initial_notif_time(time_t notif_time) {
  _initial_notif_time = notif_time;
}

time_t notifier::get_initial_notif_time() const {
  return _initial_notif_time;
}

void notifier::set_acknowledgement_timeout(int timeout) {
  _acknowledgement_timeout = timeout;
}

void notifier::set_last_acknowledgement(time_t ack) {
  _last_acknowledgement = ack;
}

void notifier::set_recovery_notification_delay(uint32_t delay) {
  _recovery_notification_delay = delay;
}

void notifier::set_recovery_been_sent(bool sent) {
  _recovery_been_sent = sent;
}

time_t notifier::get_last_acknowledgement() const {
  return _last_acknowledgement;
}

bool notifier::get_recovery_been_sent() const {
  return _recovery_been_sent;
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

void  notifier::set_notification_period(std::string const& notification_period)
{
  _notification_period = notification_period;
}

std::string const& notifier::get_check_period() const {
  return _check_period;
}

void notifier::set_check_period(std::string const& check_period) {
  _check_period = check_period;
}

std::string const& notifier::get_action_url() const {
  return _action_url;
}

void notifier::set_action_url(std::string const& action_url) {
  _action_url = action_url;
}

std::string const& notifier::get_icon_image() const {
  return _icon_image;
}

void notifier::set_icon_image(std::string const& icon_image) {
  _icon_image = icon_image;
}

std::string const& notifier::get_icon_image_alt() const {
  return _icon_image_alt;
}

void notifier::set_icon_image_alt(std::string const& icon_image_alt) {
  _icon_image_alt = icon_image_alt;
}
