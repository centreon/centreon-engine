/*
** Copyright 1999-2008 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "com/centreon/engine/timezone_locker.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;

static char const* tab_notification_str[] = {
  "NORMAL",
  "ACKNOWLEDGEMENT",
  "FLAPPINGSTART",
  "FLAPPINGSTOP",
  "FLAPPINGDISABLED",
  "DOWNTIMESTART",
  "DOWNTIMEEND",
  "DOWNTIMECANCELLED",
};

static char const* tab_host_state_str[] = {
  "UP",
  "DOWN",
  "UNREACHABLE"
};

static char const* tab_service_state_str[] = {
  "OK",
  "WARNING",
  "CRITICAL"
};

/******************************************************************/
/***************** SERVICE NOTIFICATION FUNCTIONS *****************/
/******************************************************************/

/* notify contacts about a service problem or recovery */
int service_notification(
      com::centreon::engine::service2* svc,
      unsigned int type,
      char const* not_author,
      char const* not_data,
      int options) {
  host* temp_host = nullptr;
  notification* temp_notification = nullptr;
  contact* temp_contact = nullptr;
  time_t current_time;
  struct timeval start_time;
  struct timeval end_time;
  int escalated = false;
  int contacts_notified = 0;
  int increment_notification_number = false;
  nagios_macros mac;
  int neb_result;

  logger(dbg_functions, basic)
    << "service_notification()";

  /* get the current time */
  time(&current_time);
  gettimeofday(&start_time, nullptr);

  logger(dbg_notifications, basic)
    << "** Service Notification Attempt ** Host: '" << svc->host_name
    << "', Service: '" << svc->description
    << "', Type: " << type << ", Options: " << options
    << ", Current State: " << svc->current_state
    << ", Last Notification: " << my_ctime(&svc->last_notification);

  /* if we couldn't find the host, return an error */
  if ((temp_host = svc->host_ptr) == nullptr) {
    logger(dbg_notifications, basic)
      << "Couldn't find the host associated with this service, so we "
      "won't send a notification!";
    return ERROR;
  }

  /* check the viability of sending out a service notification */
  if (check_service_notification_viability(
        svc,
        type,
        options) == ERROR) {
    logger(dbg_notifications, basic)
      << "Notification viability test failed.  No notification will "
      "be sent out.";
    return OK;
  }

  logger(dbg_notifications, basic)
    << "Notification viability test passed.";

  /* should the notification number be increased? */
  if (type == NOTIFICATION_NORMAL
      || (options & NOTIFICATION_OPTION_INCREMENT)) {
    svc->current_notification_number++;
    increment_notification_number = true;
  }

  logger(dbg_notifications, more)
    << "Current notification number: "
    << svc->current_notification_number
    << " ("
    << (increment_notification_number == true ? "incremented)" : "unchanged)");

  /* save and increase the current notification id */
  svc->current_notification_id = next_notification_id;
  next_notification_id++;

  logger(dbg_notifications, most)
    << "Creating list of contacts to be notified.";

  /* create the contact notification list for this service */
  memset(&mac, 0, sizeof(mac));
  create_notification_list_from_service(&mac, svc, options, &escalated);

  /* send data to event broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_notification_data(
    NEBTYPE_NOTIFICATION_START,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_NOTIFICATION,
    type,
    start_time,
    end_time,
    (void*)svc,
    not_author,
    not_data,
    escalated,
    0,
    nullptr);
  if (NEBERROR_CALLBACKCANCEL == neb_result) {
    free_notification_list();
    return ERROR;
  }
  else if (NEBERROR_CALLBACKOVERRIDE == neb_result) {
    free_notification_list();
    return OK;
  }

  /* we have contacts to notify... */
  if (notification_list != nullptr) {

    /* grab the macro variables */
    grab_host_macros_r(&mac, temp_host);
    grab_service_macros_r(&mac, svc);

    /*
     * if this notification has an author, attempt to lookup the
     * associated contact
     */
    if (not_author != nullptr) {

      /* see if we can find the contact - first by name, then by alias */

      if ((temp_contact = configuration::applier::state::instance().find_contact(not_author)) == nullptr) {
        for (std::unordered_map<std::string,
                                std::shared_ptr<contact> >::const_iterator
               it(state::instance().contacts().begin()),
               end(state::instance().contacts().end());
             it != end;
             ++it) {
          if (!strcmp(it->second->get_alias().c_str(), not_author)) {
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
    }
    else {
      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORNAME]);
      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORALIAS]);
    }

    /*
     * NOTE: these macros are deprecated and will likely disappear in next
     * major release, If this is an acknowledgement, get author and comment
     * macros
     */
    if (type == NOTIFICATION_ACKNOWLEDGEMENT) {
      string::setstr(mac.x[MACRO_SERVICEACKAUTHOR], not_author);
      string::setstr(mac.x[MACRO_SERVICEACKCOMMENT], not_data);
      if (temp_contact) {
        string::setstr(mac.x[MACRO_SERVICEACKAUTHORNAME],
          temp_contact->get_name());
        string::setstr(mac.x[MACRO_SERVICEACKAUTHORALIAS],
          temp_contact->get_alias());
      }
      else {
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
    else if (svc->current_state == STATE_OK)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "RECOVERY");
    else
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "PROBLEM");

    /* set the notification number macro */
    string::setstr(mac.x[MACRO_SERVICENOTIFICATIONNUMBER],
      svc->current_notification_number);

    /*
     * the $NOTIFICATIONNUMBER$ macro is maintained for backward compatability
     */
    char const* notificationnumber(mac.x[MACRO_SERVICENOTIFICATIONNUMBER]);
    string::setstr(mac.x[MACRO_NOTIFICATIONNUMBER],
      notificationnumber ? notificationnumber : "");

    /* set the notification id macro */
    string::setstr(mac.x[MACRO_SERVICENOTIFICATIONID],
      svc->current_notification_id);

    /* notify each contact (duplicates have been removed) */
    for (temp_notification = notification_list;
         temp_notification != nullptr;
         temp_notification = temp_notification->next) {

      /* grab the macro variables for this contact */
      grab_contact_macros_r(&mac, temp_notification->cntct);

      /* clear summary macros (they are customized for each contact) */
      clear_summary_macros_r(&mac);

      /* notify this contact */
      int result = notify_contact_of_service(
                     &mac,
                     temp_notification->cntct,
                     svc,
                     type,
                     not_author,
                     not_data,
                     options,
                     escalated);

      /* keep track of how many contacts were notified */
      if (result == OK)
        contacts_notified++;
    }

    /* free memory allocated to the notification list */
    free_notification_list();

    /*
     * clear summary macros so they will be regenerated without contact
     * filters when needed next
     */
    clear_summary_macros_r(&mac);

    if (type == NOTIFICATION_NORMAL) {

      /*
       * adjust last/next notification time and notification flags if we
       * notified someone
       */
      if (contacts_notified > 0) {

        /* calculate the next acceptable re-notification time */
        svc->next_notification = get_next_service_notification_time(
                                   svc,
                                   current_time);

        logger(dbg_notifications, basic)
          << contacts_notified << " contacts were notified.  "
          "Next possible notification time: "
          << my_ctime(&svc->next_notification);

        /*
         * update the last notification time for this service (this is needed
         * for rescheduling later notifications)
         * */
        svc->last_notification = current_time;

        /* update notifications flags */
        if (svc->current_state == STATE_UNKNOWN)
          svc->notified_on_unknown = true;
        else if (svc->current_state == STATE_WARNING)
          svc->notified_on_warning = true;
        else if (svc->current_state == STATE_CRITICAL)
          svc->notified_on_critical = true;
      }

      /* we didn't end up notifying anyone */
      else if (increment_notification_number == true) {

        /* adjust current notification number */
        svc->current_notification_number--;

        logger(dbg_notifications, basic)
          << "No contacts were notified.  Next possible "
          "notification time: " << my_ctime(&svc->next_notification);
      }
    }

    logger(dbg_notifications, basic)
      << contacts_notified << " contacts were notified.";
  }

  /* there were no contacts, so no notification really occurred... */
  else {
    /* readjust current notification number, since one didn't go out */
    if (increment_notification_number == true)
      svc->current_notification_number--;

    logger(dbg_notifications, basic)
      << "No contacts were found for notification purposes.  "
      "No notification was sent out.";
  }

  /* get the time we finished */
  gettimeofday(&end_time, nullptr);

  /* send data to event broker */
  broker_notification_data(
    NEBTYPE_NOTIFICATION_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_NOTIFICATION,
    type,
    start_time,
    end_time,
    (void*)svc,
    not_author,
    not_data,
    escalated,
    contacts_notified,
    nullptr);

  /* update the status log with the service information */
  update_service_status(svc, false);

  /* clear volatile macros */
  clear_volatile_macros_r(&mac);

  /* Update recovery been sent parameter */
  if (svc->current_state == STATE_OK)
    service_other_props[std::make_pair(
                        svc->host_ptr->get_name(),
                        svc->description)].recovery_been_sent = true;

  return OK;
}

/* checks the viability of sending out a service alert (top level filters) */
int check_service_notification_viability(
      com::centreon::engine::service2* svc,
      unsigned int type,
      int options) {
  host* temp_host;
  timeperiod* temp_period;
  time_t current_time;
  time_t timeperiod_start;

  logger(dbg_functions, basic)
    << "check_service_notification_viability()";

  /* forced notifications bust through everything */
  if (options & NOTIFICATION_OPTION_FORCED) {
    logger(dbg_notifications, more)
      << "This is a forced service notification, so we'll send it out.";
    return OK;
  }

  /* get current time */
  time(&current_time);

  /* are notifications enabled? */
  if (config->enable_notifications() == false) {
    logger(dbg_notifications, more)
      << "Notifications are disabled, so service notifications will "
      "not be sent out.";
    return ERROR;
  }

  /* find the host this service is associated with */
  if ((temp_host = (host*)svc->host_ptr) == nullptr) {
    logger(dbg_notifications, more)
      << "Couldn't find the host associated with this service, "
      "so we won't send a notification.";
    return ERROR;
  }

  /* if the service has no notification period, inherit one from the host */
  temp_period = svc->notification_period_ptr;
  if (temp_period == nullptr)
    temp_period = svc->host_ptr->notification_period_ptr;

  // See if the service can have notifications sent out at this time.
  {
    timezone_locker lock(get_service_timezone(
                           svc->host_name,
                           svc->description));
    if (check_time_against_period(current_time, temp_period) == ERROR) {
      logger(dbg_notifications, more)
        << "This service shouldn't have notifications sent out "
        "at this time.";

      // Calculate the next acceptable notification time,
      // once the next valid time range arrives...
      if (type == NOTIFICATION_NORMAL) {
        get_next_valid_time(
          current_time,
          &timeperiod_start,
          svc->notification_period_ptr);

        // Looks like there are no valid notification times defined, so
        // schedule the next one far into the future (one year)...
        if (timeperiod_start == (time_t)0)
          svc->next_notification
            = (time_t)(current_time + (60 * 60 * 24 * 365));
        // Else use the next valid notification time.
        else
          svc->next_notification = timeperiod_start;
        logger(dbg_notifications, more)
          << "Next possible notification time: "
          << my_ctime(&svc->next_notification);
      }

      return ERROR;
    }
  }

  /* are notifications temporarily disabled for this service? */
  if (svc->notifications_enabled == false) {
    logger(dbg_notifications, more)
      << "Notifications are temporarily disabled for "
      "this service, so we won't send one out.";
    return ERROR;
  }

  /*********************************************/
  /*** SPECIAL CASE FOR CUSTOM NOTIFICATIONS ***/
  /*********************************************/

  /* custom notifications are good to go at this point... */
  if (type == NOTIFICATION_CUSTOM) {
    if (svc->scheduled_downtime_depth > 0
        || temp_host->get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
        << "We shouldn't send custom notification during "
        "scheduled downtime.";
      return ERROR;
    }
    return OK;
  }

  /****************************************/
  /*** SPECIAL CASE FOR ACKNOWLEGEMENTS ***/
  /****************************************/

  /*
   * acknowledgements only have to pass three general filters, although they
   * have another test of their own...
   */
  if (type == NOTIFICATION_ACKNOWLEDGEMENT) {

    /* don't send an acknowledgement if there isn't a problem... */
    if (svc->current_state == STATE_OK) {
      logger(dbg_notifications, more)
        << "The service is currently OK, so we won't send an "
        "acknowledgement.";
      return ERROR;
    }

    /*
     * acknowledgement viability test passed, so the notification can be sent
     * out
     */
    return OK;
  }


  /****************************************/
  /*** SPECIAL CASE FOR FLAPPING ALERTS ***/
  /****************************************/

  /* flapping notifications only have to pass three general filters */
  if (type == NOTIFICATION_FLAPPINGSTART
      || type == NOTIFICATION_FLAPPINGSTOP
      || type == NOTIFICATION_FLAPPINGDISABLED) {

    /* don't send a notification if we're not supposed to... */
    if (svc->notify_on_flapping == false) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about FLAPPING events for this "
        "service.";
      return ERROR;
    }

    /* don't send notifications during scheduled downtime */
    if (svc->scheduled_downtime_depth > 0
        || temp_host->get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about FLAPPING events during "
        "scheduled downtime.";
      return ERROR;
    }

    /* flapping viability test passed, so the notification can be sent out */
    return OK;
  }

  /****************************************/
  /*** SPECIAL CASE FOR DOWNTIME ALERTS ***/
  /****************************************/

  /* downtime notifications only have to pass three general filters */
  if (type == NOTIFICATION_DOWNTIMESTART
      || type == NOTIFICATION_DOWNTIMEEND
      || type == NOTIFICATION_DOWNTIMECANCELLED) {

    /* don't send a notification if we're not supposed to... */
    if (svc->notify_on_downtime == false) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about DOWNTIME events for "
        "this service.";
      return ERROR;
    }

    /*
     * don't send notifications during scheduled downtime (for service only,
     * not host)
     */
    if (svc->scheduled_downtime_depth > 0) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about DOWNTIME events during "
        "scheduled downtime.";
      return ERROR;
    }

    /* downtime viability test passed, so the notification can be sent out */
    return OK;
  }

  /****************************************/
  /*** NORMAL NOTIFICATIONS ***************/
  /****************************************/

  /* is this a hard problem/recovery? */
  if (svc->state_type == SOFT_STATE) {
    logger(dbg_notifications, more)
      << "This service is in a soft state, so we won't send a "
      "notification out.";
    return ERROR;
  }

  /* has this problem already been acknowledged? */
  if (svc->problem_has_been_acknowledged == true) {
    logger(dbg_notifications, more)
      << "This service problem has already been acknowledged, "
      "so we won't send a notification out.";
    return ERROR;
  }

  /* check service notification dependencies */
  if (check_service_dependencies(
        svc,
        hostdependency::notification) == DEPENDENCIES_FAILED) {
    logger(dbg_notifications, more)
      << "Service notification dependencies for this service "
      "have failed, so we won't sent a notification out.";
    return ERROR;
  }

  /* check host notification dependencies */
  if (check_host_dependencies(
        temp_host,
        hostdependency::notification) == DEPENDENCIES_FAILED) {
    logger(dbg_notifications, more)
      << "Host notification dependencies for this service have failed, "
      "so we won't sent a notification out.";
    return ERROR;
  }

  /* see if we should notify about problems with this service */
  if (svc->current_state == STATE_UNKNOWN
      && svc->notify_on_unknown == false) {
    logger(dbg_notifications, more)
      << "We shouldn't notify about UNKNOWN states for this service.";
    return ERROR;
  }
  if (svc->current_state == STATE_WARNING
      && svc->notify_on_warning == false) {
    logger(dbg_notifications, more)
      << "We shouldn't notify about WARNING states for this service.";
    return ERROR;
  }
  if (svc->current_state == STATE_CRITICAL
      && svc->notify_on_critical == false) {
    logger(dbg_notifications, more)
      << "We shouldn't notify about CRITICAL states for this service.";
    return ERROR;
  }
  if (svc->current_state == STATE_OK) {
    if (svc->notify_on_recovery == false) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about RECOVERY states for this service.";
      return ERROR;
    }
    if (!(svc->notified_on_unknown == true
          || svc->notified_on_warning == true
          || svc->notified_on_critical == true)) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about this recovery.";
      return ERROR;
    }
  }

  /* see if enough time has elapsed for first notification */
  if (type == NOTIFICATION_NORMAL
      && (svc->current_notification_number == 0
          || (svc->current_state == STATE_OK
                && !service_other_props[std::make_pair(
                    svc->host_ptr->get_name(),
                    svc->description)].recovery_been_sent))) {

    /* get the time at which a notification should have been sent */
    time_t& initial_notif_time(
              service_other_props[std::make_pair(
                                         svc->host_ptr->get_name(),
                                         svc->description)].initial_notif_time);

    /* if not set, set it to now */
    if (!initial_notif_time)
      initial_notif_time = time(nullptr);

    double notification_delay = (svc->current_state != STATE_OK ?
             svc->first_notification_delay
             : service_other_props[std::make_pair(
                 svc->host_ptr->get_name(),
                 svc->description)].recovery_notification_delay)
        * config->interval_length();

    if (current_time
        < (time_t)(initial_notif_time
                   + (time_t)(notification_delay))) {
      if (svc->current_state == STATE_OK)
        logger(dbg_notifications, more)
          << "Not enough time has elapsed since the service changed to a "
          "OK state, so we should not notify about this problem yet";
      else
        logger(dbg_notifications, more)
          << "Not enough time has elapsed since the service changed to a "
          "non-OK state, so we should not notify about this problem yet";
      return ERROR;
    }
  }

  /* if this service is currently flapping, don't send the notification */
  if (svc->is_flapping == true) {
    logger(dbg_notifications, more)
      << "This service is currently flapping, so we won't send "
      "notifications.";
    return ERROR;
  }

  /*
   * if this service is currently in a scheduled downtime period, don't send
   * the notification
   */
  if (svc->scheduled_downtime_depth > 0) {
    logger(dbg_notifications, more)
      << "This service is currently in a scheduled downtime, so "
      "we won't send notifications.";
    return ERROR;
  }

  /*
   * if this host is currently in a scheduled downtime period, don't send the
   * notification
   * */
  if (temp_host->get_scheduled_downtime_depth() > 0) {
    logger(dbg_notifications, more)
      << "The host this service is associated with is currently in "
      "a scheduled downtime, so we won't send notifications.";
    return ERROR;
  }

  /*
   ***** RECOVERY NOTIFICATIONS ARE GOOD TO GO AT THIS POINT IF ANY OTHER *****
   ***** NOTIFICATION WAS SENT                                            *****
   */
  if (svc->current_state == STATE_OK)
    return ((svc->current_notification_number > 0)
            ? OK
            : ERROR);

  /*
   * don't notify contacts about this service problem again if the notification
   * interval is set to 0
   */
  if (svc->no_more_notifications == true) {
    logger(dbg_notifications, more)
      << "We shouldn't re-notify contacts about this service problem.";
    return ERROR;
  }

  /*
   * if the host is down or unreachable, don't notify contacts about service
   * failures
   */
  if (temp_host->get_current_state() != HOST_UP) {
    logger(dbg_notifications, more)
      << "The host is either down or unreachable, so we won't "
      "notify contacts about this service.";
    return ERROR;
  }

  /*
   * don't notify if we haven't waited long enough since the last time (and
   * the service is not marked as being volatile)
   */
  if ((current_time < svc->next_notification)
      && svc->is_volatile == false) {
    logger(dbg_notifications, more)
      << "We haven't waited long enough to re-notify contacts "
      "about this service.";
    logger(dbg_notifications, more)
      << "Next valid notification time: "
      << my_ctime(&svc->next_notification);
    return ERROR;
  }

  return OK;
}

/*
 * check viability of sending out a service notification to a specific contact
 * (contact-specific filters)
 */
int check_contact_service_notification_viability(
      contact* cntct,
      com::centreon::engine::service2* svc,
      unsigned int type,
      int options) {

  logger(dbg_functions, basic)
    << "check_contact_service_notification_viability()";

  logger(dbg_notifications, most)
    << "** Checking service notification viability "
    "for contact '" << cntct->get_name() << "'...";

  /* forced notifications bust through everything */
  if (options & NOTIFICATION_OPTION_FORCED) {
    logger(dbg_notifications, more)
      << "This is a forced service notification, so we'll "
      "send it out to this contact.";
    return OK;
  }

  /* are notifications enabled? */
  if (!cntct->get_service_notifications_enabled()) {
    logger(dbg_notifications, most)
      << "Service notifications are disabled for this contact.";
    return ERROR;
  }

  // See if the contact can be notified at this time.
  {
    timezone_locker lock(cntct->get_timezone().c_str());
    if (check_time_against_period(
          time(nullptr),
          cntct->service_notification_period_ptr) == ERROR) {
      logger(dbg_notifications, most)
        << "This contact shouldn't be notified at this time.";
      return ERROR;
    }
  }

  /*********************************************/
  /*** SPECIAL CASE FOR CUSTOM NOTIFICATIONS ***/
  /*********************************************/

  /* custom notifications are good to go at this point... */
  if (type == NOTIFICATION_CUSTOM)
    return OK;

  /****************************************/
  /*** SPECIAL CASE FOR FLAPPING ALERTS ***/
  /****************************************/

  if (type == NOTIFICATION_FLAPPINGSTART
      || type == NOTIFICATION_FLAPPINGSTOP
      || type == NOTIFICATION_FLAPPINGDISABLED) {

    if (!cntct->notify_on_service_flapping()) {
      logger(dbg_notifications, most)
        << "We shouldn't notify this contact about FLAPPING "
        "service events.";
      return ERROR;
    }

    return OK;
  }

  /****************************************/
  /*** SPECIAL CASE FOR DOWNTIME ALERTS ***/
  /****************************************/

  if (type == NOTIFICATION_DOWNTIMESTART
      || type == NOTIFICATION_DOWNTIMEEND
      || type == NOTIFICATION_DOWNTIMECANCELLED) {

    if (!cntct->notify_on_service_downtime()) {
      logger(dbg_notifications, most)
        << "We shouldn't notify this contact about DOWNTIME "
        "service events.";
      return ERROR;
    }

    return OK;
  }

  /*************************************/
  /*** ACKS AND NORMAL NOTIFICATIONS ***/
  /*************************************/

  /* see if we should notify about problems with this service */
  if (svc->current_state == STATE_UNKNOWN
      && !cntct->notify_on_service_unknown()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about UNKNOWN "
      "service states.";
    return ERROR;
  }

  if (svc->current_state == STATE_WARNING
      && !cntct->notify_on_service_warning()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about WARNING "
      "service states.";
    return ERROR;
  }

  if (svc->current_state == STATE_CRITICAL
      && !cntct->notify_on_service_critical()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about CRITICAL "
      "service states.";
    return ERROR;
  }

  if (svc->current_state == STATE_OK) {

    if (!cntct->notify_on_service_recovery()) {
      logger(dbg_notifications, most)
        << "We shouldn't notify this contact about RECOVERY "
        "service states.";
      return ERROR;
    }

    if (!((svc->notified_on_unknown == true
           && cntct->notify_on_service_unknown())
          || (svc->notified_on_warning == true
              && cntct->notify_on_service_warning())
          || (svc->notified_on_critical == true
              && cntct->notify_on_service_critical()))) {
      logger(dbg_notifications, most)
        << "We shouldn't notify about this recovery.";
      return ERROR;
    }
  }

  logger(dbg_notifications, most)
    << "** Service notification viability for contact '"
    << cntct->get_name() <<"' PASSED.";

  return OK;
}

/* notify a specific contact about a service problem or recovery */
int notify_contact_of_service(
      nagios_macros* mac,
      contact* cntct,
      com::centreon::engine::service2* svc,
      int type,
      char const* not_author,
      char const* not_data,
      int options,
      int escalated) {
  char* command_name_ptr(nullptr);
  char* raw_command = nullptr;
  char* processed_command = nullptr;
  int early_timeout = false;
  double exectime;
  struct timeval start_time, end_time;
  struct timeval method_start_time, method_end_time;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  int neb_result;

  logger(dbg_functions, basic)
    << "notify_contact_of_service()";
  logger(dbg_notifications, most)
    << "** Attempting to notifying contact '" << cntct->get_name() << "'...";

  /*
   * check viability of notifying this user
   * acknowledgements are no longer excluded from this test -
   * added 8/19/02 Tom Bertelson
   */
  if (check_contact_service_notification_viability(
        cntct,
        svc,
        type,
        options) == ERROR)
    return ERROR;

  logger(dbg_notifications, most)
    << "** Notifying contact '" << cntct->get_name() << "'";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* send data to event broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_contact_notification_data(
                 NEBTYPE_CONTACTNOTIFICATION_START,
                 NEBFLAG_NONE,
                 NEBATTR_NONE,
                 SERVICE_NOTIFICATION,
                 type,
                 start_time,
                 end_time,
                 (void*)svc,
                 cntct,
                 not_author,
                 not_data,
                 escalated,
                 nullptr);
  if (NEBERROR_CALLBACKCANCEL == neb_result)
    return ERROR;
  else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
    return OK;

  /* process all the notification commands this user has */
  for (std::shared_ptr<commands::command> const& cmd :
        cntct->get_service_notification_commands()) {
    /* get start time */
    gettimeofday(&method_start_time, nullptr);

    /* send data to event broker */
    method_end_time.tv_sec = 0L;
    method_end_time.tv_usec = 0L;
    neb_result = broker_contact_notification_method_data(
                   NEBTYPE_CONTACTNOTIFICATIONMETHOD_START,
                   NEBFLAG_NONE,
                   NEBATTR_NONE,
                   SERVICE_NOTIFICATION,
                   type,
                   method_start_time,
                   method_end_time,
                   (void*)svc,
                   cntct,
                   cmd->get_command_line().c_str(),
                   not_author,
                   not_data,
                   escalated,
                   nullptr);
    if (NEBERROR_CALLBACKCANCEL == neb_result)
      break;
    else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
      continue;

    /* get the raw command line */
    get_raw_command_line_r(
      mac,
      cmd.get(),
      cmd->get_command_line().c_str(),
      &raw_command,
      macro_options);
    if (raw_command == nullptr)
      continue;

    logger(dbg_notifications, most)
      << "Raw notification command: " << raw_command;

    /* process any macros contained in the argument */
    process_macros_r(
      mac,
      raw_command,
      &processed_command,
      macro_options);
    if (processed_command == nullptr)
      continue;

    /* run the notification command... */

    logger(dbg_notifications, most)
      << "Processed notification command: " << processed_command;

    /* log the notification to program log file */
    if (config->log_notifications() == true) {
      char const* service_state_str("UNKNOWN");
      if ((unsigned int)svc->current_state <
           sizeof(tab_service_state_str) / sizeof(*tab_service_state_str))
        service_state_str = tab_service_state_str[svc->current_state];

      char const* notification_str("");
      if ((unsigned int)type <
           sizeof(tab_notification_str) / sizeof(*tab_notification_str))
        notification_str = tab_notification_str[type];

      std::string info;
      switch (type) {
      case NOTIFICATION_CUSTOM:
        notification_str = "CUSTOM";

      case NOTIFICATION_ACKNOWLEDGEMENT:
        info
          .append(";").append(not_author ? not_author : "")
          .append(";").append(not_data ? not_data : "");
        break;
      }

      std::string service_notification_state;
      if (strcmp(notification_str, "NORMAL") == 0)
        service_notification_state.append(service_state_str);
      else
        service_notification_state
          .append(notification_str)
          .append(" (")
          .append(service_state_str)
          .append(")");

      logger(log_service_notification, basic)
        << "SERVICE NOTIFICATION: " << cntct->get_name() << ';'
        << svc->host_name << ';' << svc->description << ';'
        << service_notification_state << ";"
        << cmd->get_name() << ';'
        << (svc->plugin_output ? svc->plugin_output : "")
        << info;
    }

    /* run the notification command */
    try {
      my_system_r(
        mac,
        processed_command,
        config->notification_timeout(),
        &early_timeout,
        &exectime,
        nullptr,
        0);
    } catch (std::exception const& e) {
      logger(log_runtime_error, basic)
        << "Error: can't execute service notification '"
        << cntct->get_name() << "' : " << e.what();
    }

    /* check to see if the notification command timed out */
    if (early_timeout == true) {
      logger(log_service_notification | log_runtime_warning, basic)
        << "Warning: Contact '" << cntct->get_name()
        << "' service notification command '" << processed_command
        << "' timed out after " << config->notification_timeout()
        << " seconds";
    }

    /* free memory */
    delete[] raw_command;
    delete[] processed_command;

    /* get end time */
    gettimeofday(&method_end_time, nullptr);

    /* send data to event broker */
    broker_contact_notification_method_data(
      NEBTYPE_CONTACTNOTIFICATIONMETHOD_END,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      SERVICE_NOTIFICATION,
      type,
      method_start_time,
      method_end_time,
      (void*)svc,
      cntct,
      cmd->get_command_line().c_str(),
      not_author,
      not_data,
      escalated,
      nullptr);
  }

  /* get end time */
  gettimeofday(&end_time, nullptr);

  /* update the contact's last service notification time */
  cntct->set_last_service_notification(start_time.tv_sec);

  /* send data to event broker */
  broker_contact_notification_data(
    NEBTYPE_CONTACTNOTIFICATION_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_NOTIFICATION,
    type,
    start_time,
    end_time,
    (void*)svc,
    cntct,
    not_author,
    not_data,
    escalated,
    nullptr);
  return OK;
}

/*
 * checks to see if a service escalation entry is a match for the current
 * service notification
 */
int is_valid_escalation_for_service_notification(
      com::centreon::engine::service2* svc,
      serviceescalation* se,
      int options) {
  int notification_number = 0;
  time_t current_time = 0L;
  com::centreon::engine::service2* temp_service = nullptr;

  logger(dbg_functions, basic)
    << "is_valid_escalation_for_service_notification()";

  /* get the current time */
  time(&current_time);

  /*
   * if this is a recovery, really we check for who got notified about a
   * previous problem
   */
  if (svc->current_state == STATE_OK)
    notification_number = svc->current_notification_number - 1;
  else
    notification_number = svc->current_notification_number;

  /* this entry if it is not for this service */
  temp_service = se->service_ptr;
  if (temp_service == nullptr || temp_service != svc)
    return false;

  /*** EXCEPTION ***/
  /* broadcast options go to everyone, so this escalation is valid */
  if (options & NOTIFICATION_OPTION_BROADCAST)
    return true;

  /* skip this escalation if it happens later */
  if (se->first_notification > notification_number)
    return false;

  /* skip this escalation if it has already passed */
  if (se->last_notification != 0
      && se->last_notification < notification_number)
    return false;

  /*
   * skip this escalation if it has a timeperiod and the current time isn't
   * valid
   */
  if (se->escalation_period != nullptr
      && check_time_against_period(
           current_time,
           se->escalation_period_ptr) == ERROR)
    return false;

  /* skip this escalation if the state options don't match */
  if (svc->current_state == STATE_OK
      && se->escalate_on_recovery == false)
    return false;
  else if (svc->current_state == STATE_WARNING
           && se->escalate_on_warning == false)
    return false;
  else if (svc->current_state == STATE_UNKNOWN
           && se->escalate_on_unknown == false)
    return false;
  else if (svc->current_state == STATE_CRITICAL
           && se->escalate_on_critical == false)
    return false;

  return true;
}

/**
 *  Checks to see whether a service notification should be escalated.
 *
 *  @param[in] svc Service.
 *
 *  @return true if service notification should be escalated, false if
 *          it should not.
 */
int should_service_notification_be_escalated(com::centreon::engine::service2* svc) {
  // Debug.
  logger(dbg_functions, basic)
    << "should_service_notification_be_escalated()";

  // Browse service escalations related to this service.
  typedef umultimap<std::pair<std::string, std::string>,
                    std::shared_ptr<serviceescalation> > collection;
  std::pair<collection::iterator, collection::iterator> p;
  p = state::instance().serviceescalations().equal_range(
        std::make_pair(svc->host_name, svc->description));
  while (p.first != p.second) {
    serviceescalation* temp_se(p.first->second.get());

    // We found a matching entry, so escalate this notification!
    if (is_valid_escalation_for_service_notification(
          svc,
          temp_se,
          NOTIFICATION_OPTION_NONE) == true) {
      logger(dbg_notifications, more)
        << "Service notification WILL be escalated.";
      return true;
    }

    ++p.first;
  }
  logger(dbg_notifications, more)
    << "Service notification will NOT be escalated.";
  return false;
}

/*
 * given a service, create a list of contacts to be notified, removing
 * duplicates
 */
int create_notification_list_from_service(
      nagios_macros* mac,
      com::centreon::engine::service2* svc, int options,
      int* escalated) {
  int escalate_notification = false;

  logger(dbg_functions, basic)
    << "create_notification_list_from_service()";

  /* see if this notification should be escalated */
  escalate_notification = should_service_notification_be_escalated(svc);

  /* set the escalation flag */
  *escalated = escalate_notification;

  /* make sure there aren't any leftover contacts */
  free_notification_list();

  /* set the escalation macro */
  string::setstr(mac->x[MACRO_NOTIFICATIONISESCALATED], escalate_notification);

  if (options & NOTIFICATION_OPTION_BROADCAST)
    logger(dbg_notifications, more)
      << "This notification will be BROADCAST to all "
      "(escalated and normal) contacts...";

  /* use escalated contacts for this notification */
  if (escalate_notification == true
      || (options & NOTIFICATION_OPTION_BROADCAST)) {

    logger(dbg_notifications, more)
      << "Adding contacts from service escalation(s) to "
      "notification list.";

    std::pair<std::string, std::string>
      id(std::make_pair(svc->host_name, svc->description));
    umultimap<std::pair<std::string, std::string>,
              std::shared_ptr<serviceescalation> > const&
      escalations(state::instance().serviceescalations());
    for (umultimap<std::pair<std::string, std::string>,
                   std::shared_ptr<serviceescalation> >::const_iterator
           it(escalations.find(id)), end(escalations.end());
         it != end && it->first == id;
         ++it) {
      serviceescalation* temp_se(&*it->second);

      /* skip this entry if it isn't appropriate */
      if (is_valid_escalation_for_service_notification(
            svc,
            temp_se,
            options) == false)
        continue;

      logger(dbg_notifications, most)
        << "Adding individual contacts from service escalation(s) "
        "to notification list.";

      /* add all individual contacts for this escalation entry */
      for (contact_map::iterator
             it(temp_se->contacts.begin()),
             end(temp_se->contacts.end());
           it != end;
           ++it)
        add_notification(mac, it->second.get());

      logger(dbg_notifications, most)
        << "Adding members of contact groups from service escalation(s) "
        "to notification list.";

      /* add all contacts that belong to contactgroups for this escalation */
      for (contactgroup_map::iterator
             it(temp_se->contact_groups.begin()),
             end(temp_se->contact_groups.end());
           it != end;
           ++it) {
        logger(dbg_notifications, most)
          << "Adding members of contact group '"
          << it->first
          << "' for service escalation to notification list.";

        if (it->second == nullptr)
          continue;
        for (std::unordered_map<std::string, contact *>::const_iterator
               itm(it->second->get_members().begin()),
               mend(it->second->get_members().end());
              itm != mend;
              ++itm) {
          if (itm->second == nullptr)
            continue;
          add_notification(mac, itm->second);
        }
      }
    }
  }

  /* else use normal, non-escalated contacts */
  if (escalate_notification == false
      || (options & NOTIFICATION_OPTION_BROADCAST)) {

    logger(dbg_notifications, more)
      << "Adding normal contacts for service to notification list.";

    /* add all individual contacts for this service */
    for (contact_map::iterator
           it(svc->contacts.begin()),
           end(svc->contacts.end());
         it != end;
         ++it) {
      add_notification(mac, it->second.get());
    }

    /* add all contacts that belong to contactgroups for this service */
    for (contactgroup_map::iterator
           it(svc->contact_groups.begin()),
           end(svc->contact_groups.end());
         it != end;
         ++it) {
      logger(dbg_notifications, most)
        << "Adding members of contact group '"
        << it->first
        << "' for service to notification list.";

      if (it->second == nullptr)
        continue;
      for (std::unordered_map<std::string, contact *>::const_iterator
             itm(it->second->get_members().begin()),
             mend(it->second->get_members().end());
            itm != mend;
            ++itm) {
        if (itm->second == nullptr)
          continue;
        add_notification(mac, itm->second);
      }
    }
  }

  return OK;
}

/******************************************************************/
/******************* HOST NOTIFICATION FUNCTIONS ******************/
/******************************************************************/

/* notify all contacts for a host that the entire host is down or up */
int host_notification(
      host* hst,
      unsigned int type,
      char const* not_author,
      char const* not_data,
      int options) {
  notification* temp_notification = nullptr;
  contact* temp_contact = nullptr;
  time_t current_time;
  struct timeval start_time;
  struct timeval end_time;
  int escalated = false;
  int contacts_notified = 0;
  int increment_notification_number = false;
  nagios_macros mac;
  int neb_result;

  /* get the current time */
  time(&current_time);
  gettimeofday(&start_time, nullptr);

  time_t time = hst->get_last_host_notification();
  logger(dbg_notifications, basic)
    << "** Host Notification Attempt ** Host: '" << hst->get_name()
    << "', Type: " << type << ", Options: " << options
    << ", Current State: " << hst->get_current_state()
    << ", Last Notification: "
    << my_ctime(&time);


  /* check viability of sending out a host notification */
  if (check_host_notification_viability(hst, type, options) == ERROR) {
    logger(dbg_notifications, basic)
      << "Notification viability test failed.  No notification will "
      "be sent out.";
    return OK;
  }

  /* allocate memory for local macro */
  memset(&mac, 0, sizeof(mac));

  logger(dbg_notifications, basic)
    << "Notification viability test passed.";

  /* should the notification number be increased? */
  if (type == NOTIFICATION_NORMAL
      || (options & NOTIFICATION_OPTION_INCREMENT)) {
    hst->set_current_notification_number(hst->get_current_notification_number()
                                         + 1);
    increment_notification_number = true;
  }

  logger(dbg_notifications, more)
    << "Current notification number: "
    << hst->get_current_notification_number()
    << " ("
    << (increment_notification_number == true ? "incremented" : "unchanged")
    << ")";

  /* save and increase the current notification id */
  hst->set_current_notification_id(next_notification_id);
  next_notification_id++;

  logger(dbg_notifications, most)
    << "Creating list of contacts to be notified.";

  /* create the contact notification list for this host */
  create_notification_list_from_host(&mac, hst, options, &escalated);

  /* send data to event broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_notification_data(
    NEBTYPE_NOTIFICATION_START,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_NOTIFICATION,
    type,
    start_time,
    end_time,
    (void*)hst,
    not_author,
    not_data,
    escalated,
    0,
    nullptr);
  if (NEBERROR_CALLBACKCANCEL == neb_result) {
    free_notification_list();
    return ERROR;
  }
  else if (NEBERROR_CALLBACKOVERRIDE == neb_result) {
    free_notification_list();
    return OK;
  }

  /* there are contacts to be notified... */
  if (notification_list != nullptr) {

    /* grab the macro variables */
    grab_host_macros_r(&mac, hst);

    /*
     * if this notification has an author, attempt to lookup the associated
     * contact
     */
    if (not_author != nullptr) {

      /* see if we can find the contact - first by name, then by alias */
      if ((temp_contact = configuration::applier::state::instance().find_contact(not_author)) == nullptr) {
        for (std::unordered_map<std::string, std::shared_ptr<contact> >::const_iterator
               it(state::instance().contacts().begin()),
               end(state::instance().contacts().end());
             it != end;
             ++it) {
          if (!strcmp(it->second->get_alias().c_str(), not_author)) {
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
    }
    else {
      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORNAME]);
      string::setstr(mac.x[MACRO_NOTIFICATIONAUTHORALIAS]);
    }

    /*
     * NOTE: these macros are deprecated and will likely disappear in next
     * major release
     * if this is an acknowledgement, get author and comment macros
     */
    if (type == NOTIFICATION_ACKNOWLEDGEMENT) {
      string::setstr(mac.x[MACRO_HOSTACKAUTHOR], not_author);
      string::setstr(mac.x[MACRO_HOSTACKCOMMENT], not_data);
      if (temp_contact) {
        string::setstr(mac.x[MACRO_SERVICEACKAUTHORNAME],
          temp_contact->get_name());
        string::setstr(mac.x[MACRO_SERVICEACKAUTHORALIAS],
          temp_contact->get_alias());
      }
      else {
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
    else if (hst->get_current_state() == HOST_UP)
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "RECOVERY");
    else
      string::setstr(mac.x[MACRO_NOTIFICATIONTYPE], "PROBLEM");

    /* set the notification number macro */
    string::setstr(mac.x[MACRO_HOSTNOTIFICATIONNUMBER],
      hst->get_current_notification_number());

    /*
     * the $NOTIFICATIONNUMBER$ macro is maintained for backward compatability
     */
    char const* notificationnumber(mac.x[MACRO_HOSTNOTIFICATIONNUMBER]);
    string::setstr(mac.x[MACRO_NOTIFICATIONNUMBER],
      notificationnumber ? notificationnumber : "");

    /* set the notification id macro */
    string::setstr(mac.x[MACRO_HOSTNOTIFICATIONID],
      hst->get_current_notification_id());

    /* notify each contact (duplicates have been removed) */
    for (temp_notification = notification_list;
         temp_notification != nullptr;
         temp_notification = temp_notification->next) {

      /* grab the macro variables for this contact */
      grab_contact_macros_r(&mac, temp_notification->cntct);

      /* clear summary macros (they are customized for each contact) */
      clear_summary_macros_r(&mac);

      /* notify this contact */
      int result = notify_contact_of_host(
                     &mac,
                     temp_notification->cntct,
                     hst,
                     type,
                     not_author,
                     not_data,
                     options,
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
        hst->set_next_host_notification(get_next_host_notification_time(
                                        hst,
                                        current_time));

        /*
         * update the last notification time for this host (this is needed for
         * scheduling the next problem notification)
         */
        hst->set_last_host_notification(current_time);

        /* update notifications flags */
        if (hst->get_current_state() == HOST_DOWN)
          hst->set_notified_on_down(true);
        else if (hst->get_current_state() == HOST_UNREACHABLE)
          hst->set_notified_on_unreachable(true);

        time_t time = hst->get_next_host_notification();
        logger(dbg_notifications, basic)
          << contacts_notified << " contacts were notified.  "
          "Next possible notification time: "
          << my_ctime(&time);
      }

      /* we didn't end up notifying anyone */
      else if (increment_notification_number == true) {

        /* adjust current notification number */
        hst->set_current_notification_number(
          hst->get_current_notification_number() - 1);

        time_t time = hst->get_next_host_notification();
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
    if (increment_notification_number == true)
      hst->set_current_notification_number(
        hst->get_current_notification_number() - 1);

    logger(dbg_notifications, basic)
      << "No contacts were found for notification purposes.  "
      "No notification was sent out.";
  }

  /* get the time we finished */
  gettimeofday(&end_time, nullptr);

  /* send data to event broker */
  broker_notification_data(
    NEBTYPE_NOTIFICATION_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_NOTIFICATION,
    type,
    start_time,
    end_time,
    (void*)hst,
    not_author,
    not_data,
    escalated,
    contacts_notified,
    nullptr);

  /* update the status log with the host info */
  update_host_status(hst, false);

  /* clear volatile macros */
  clear_volatile_macros_r(&mac);

  /* Update recovery been sent parameter */
  if (hst->get_current_state() == HOST_UP)
    host_other_props[hst->get_name()].recovery_been_sent = true;

  return OK;
}

/* checks viability of sending a host notification */
int check_host_notification_viability(
      host* hst,
      unsigned int type,
      int options) {
  time_t current_time;
  time_t timeperiod_start;

  logger(dbg_functions, basic)
    << "check_host_notification_viability()";

  /* forced notifications bust through everything */
  if (options & NOTIFICATION_OPTION_FORCED) {
    logger(dbg_notifications, more)
      << "This is a forced host notification, so we'll send it out.";
    return OK;
  }

  /* get current time */
  time(&current_time);

  /* are notifications enabled? */
  if (config->enable_notifications() == false) {
    logger(dbg_notifications, more)
      << "Notifications are disabled, so host notifications will not "
      "be sent out.";
    return ERROR;
  }

  // See if the host can have notifications sent out at this time.
  {
    timezone_locker lock(get_host_timezone(hst->get_name()));
    if (check_time_against_period(
          current_time,
          hst->notification_period_ptr) == ERROR) {
      logger(dbg_notifications, more)
        << "This host shouldn't have notifications sent out at "
           "this time.";

      // If this is a normal notification, calculate the next acceptable
      // notification time, once the next valid time range arrives...
      if (type == NOTIFICATION_NORMAL) {
        get_next_valid_time(
          current_time,
          &timeperiod_start,
          hst->notification_period_ptr);

        // It looks like there is no notification time defined, so
        // schedule next one far into the future (one year)...
        if (timeperiod_start == (time_t)0)
          hst->set_next_host_notification(
            (time_t)(current_time + (60 * 60 * 24 * 365)));
        // Else use the next valid notification time.
        else
          hst->set_next_host_notification(timeperiod_start);

        time_t time = hst->get_next_host_notification();
        logger(dbg_notifications, more)
          << "Next possible notification time: "
          << my_ctime(&time);
      }
      return ERROR;
    }
  }

  /* are notifications temporarily disabled for this host? */
  if (!hst->get_notifications_enabled()) {
    logger(dbg_notifications, more)
      << "Notifications are temporarily disabled for this host, "
      "so we won't send one out.";
    return ERROR;
  }

  /*********************************************/
  /*** SPECIAL CASE FOR CUSTOM NOTIFICATIONS ***/
  /*********************************************/

  /* custom notifications are good to go at this point... */
  if (type == NOTIFICATION_CUSTOM) {
    if (hst->get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
        << "We shouldn't send custom notification during "
        "scheduled downtime.";
      return ERROR;
    }
    return OK;
  }

  /****************************************/
  /*** SPECIAL CASE FOR ACKNOWLEGEMENTS ***/
  /****************************************/

  /*
   * acknowledgements only have to pass three general filters, although they
   * have another test of their own...
   */
  if (type == NOTIFICATION_ACKNOWLEDGEMENT) {

    /* don't send an acknowledgement if there isn't a problem... */
    if (hst->get_current_state() == HOST_UP) {
      logger(dbg_notifications, more)
        << "The host is currently UP, so we won't send "
        "an acknowledgement.";
      return ERROR;
    }

    /*
     * acknowledgement viability test passed, so the notification can be sent
     * out
     */
    return OK;
  }

  /*****************************************/
  /*** SPECIAL CASE FOR FLAPPING ALERTS ***/
  /*****************************************/

  /* flapping notifications only have to pass three general filters */
  if (type == NOTIFICATION_FLAPPINGSTART
      || type == NOTIFICATION_FLAPPINGSTOP
      || type == NOTIFICATION_FLAPPINGDISABLED) {

    /* don't send a notification if we're not supposed to... */
    if (!hst->get_notify_on_flapping()) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about FLAPPING events for this host.";
      return ERROR;
    }

    /* don't send notifications during scheduled downtime */
    if (hst->get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about FLAPPING events during "
        "scheduled downtime.";
      return ERROR;
    }

    /* flapping viability test passed, so the notification can be sent out */
    return OK;
  }

  /*****************************************/
  /*** SPECIAL CASE FOR DOWNTIME ALERTS ***/
  /*****************************************/

  /* flapping notifications only have to pass three general filters */
  if (type == NOTIFICATION_DOWNTIMESTART
      || type == NOTIFICATION_DOWNTIMEEND
      || type == NOTIFICATION_DOWNTIMECANCELLED) {

    /* don't send a notification if we're not supposed to... */
    if (!hst->get_notify_on_downtime()) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about DOWNTIME events for this host.";
      return ERROR;
    }

    /* don't send notifications during scheduled downtime */
    if (hst->get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about DOWNTIME events during "
        "scheduled downtime!";
      return ERROR;
    }

    /* downtime viability test passed, so the notification can be sent out */
    return OK;
  }

  /****************************************/
  /*** NORMAL NOTIFICATIONS ***************/
  /****************************************/

  /* is this a hard problem/recovery? */
  if (hst->get_state_type() == SOFT_STATE) {
    logger(dbg_notifications, more)
      << "This host is in a soft state, so we won't send "
      "a notification out.";
    return ERROR;
  }

  /* has this problem already been acknowledged? */
  if (hst->get_problem_has_been_acknowledged()) {
    logger(dbg_notifications, more)
      << "This host problem has already been acknowledged, "
      "so we won't send a notification out!";
    return ERROR;
  }

  /* check notification dependencies */
  if (check_host_dependencies(
        hst,
        hostdependency::notification) == DEPENDENCIES_FAILED) {
    logger(dbg_notifications, more)
      << "Notification dependencies for this host have failed, "
      "so we won't sent a notification out!";
    return ERROR;
  }

  /* see if we should notify about problems with this host */
  if (hst->get_current_state() == HOST_UNREACHABLE
      && !hst->get_notify_on_unreachable()) {
    logger(dbg_notifications, more)
      << "We shouldn't notify about UNREACHABLE status for this host.";
    return ERROR;
  }
  if (hst->get_current_state() == HOST_DOWN
      && !hst->get_notify_on_down()) {
    logger(dbg_notifications, more)
      << "We shouldn't notify about DOWN states for this host.";
    return ERROR;
  }
  if (hst->get_current_state() == HOST_UP) {

    if (!hst->get_notify_on_recovery()) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about RECOVERY states for this host.";
      return ERROR;
    }
    if (!(hst->get_notified_on_down()
          || hst->get_notified_on_unreachable())) {
      logger(dbg_notifications, more)
        << "We shouldn't notify about this recovery.";
      return ERROR;
    }

  }

  /* see if enough time has elapsed for first notification */
  if (type == NOTIFICATION_NORMAL
      && (hst->get_current_notification_number() == 0
         || (hst->get_current_state() == HOST_UP &&
             !host_other_props[
               std::string(hst->get_name())].recovery_been_sent))) {

    /* get the time at which a notification should have been sent */
    time_t& initial_notif_time(
      host_other_props[hst->get_name()].initial_notif_time);

    /* if not set, set it to now */
    if (!initial_notif_time)
      initial_notif_time = time(nullptr);

    double notification_delay = (hst->get_current_state() != HOST_UP ?
             hst->get_first_notification_delay()
             : host_other_props[hst->get_name()].recovery_notification_delay)
        * config->interval_length();

    if (current_time
        < (time_t)(initial_notif_time
                   + (time_t)(notification_delay))) {
      if (hst->get_current_state() == HOST_UP)
        logger(dbg_notifications, more)
          << "Not enough time has elapsed since the host changed to an "
          "UP state (or since program start), so we shouldn't notify "
          "about this problem yet.";
      else
        logger(dbg_notifications, more)
          << "Not enough time has elapsed since the host changed to a "
          "non-UP state (or since program start), so we shouldn't notify "
          "about this problem yet.";
      return ERROR;
    }
  }

  /* if this host is currently flapping, don't send the notification */
  if (hst->get_is_flapping()) {
    logger(dbg_notifications, more)
      << "This host is currently flapping, so we won't "
      "send notifications.";
    return ERROR;
  }

  /*
   * if this host is currently in a scheduled downtime period,
   * don't send the notification
   */
  if (hst->get_scheduled_downtime_depth() > 0) {
    logger(dbg_notifications, more)
      << "This host is currently in a scheduled downtime, "
      "so we won't send notifications.";
    return ERROR;
  }

  /***** RECOVERY NOTIFICATIONS ARE GOOD TO GO AT THIS POINT *****/
  if (hst->get_current_state() == HOST_UP)
    return OK;

  /* check if we shouldn't renotify contacts about the host problem */
  if (hst->get_no_more_notifications()) {
    logger(dbg_notifications, more)
      << "We shouldn't re-notify contacts about this host problem.";
    return ERROR;
  }

  /* check if its time to re-notify the contacts about the host... */
  if (current_time < hst->get_next_host_notification()) {
    logger(dbg_notifications, more)
      << "Its not yet time to re-notify the contacts "
      "about this host problem...";
    time_t time = hst->get_next_host_notification();
    logger(dbg_notifications, more)
      << "Next acceptable notification time: "
      << my_ctime(&time);
    return ERROR;
  }

  return OK;
}

/* checks the viability of notifying a specific contact about a host */
int check_contact_host_notification_viability(
      contact* cntct,
      host* hst,
      unsigned int type,
      int options) {
  logger(dbg_functions, basic)
    << "check_contact_host_notification_viability()";
  logger(dbg_notifications, most)
    << "** Checking host notification viability for contact '"
    << cntct->get_name() << "'...";

  /* forced notifications bust through everything */
  if (options & NOTIFICATION_OPTION_FORCED) {
    logger(dbg_notifications, most)
      << "This is a forced host notification, so we'll "
      "send it out for this contact.";
    return OK;
  }

  /* are notifications enabled? */
  if (!cntct->get_host_notifications_enabled()) {
    logger(dbg_notifications, most)
      << "Host notifications are disabled for this contact.";
    return ERROR;
  }

  // See if the contact can be notified at this time.
  {
    timezone_locker lock(cntct->get_timezone().c_str());
    if (check_time_against_period(
          time(nullptr),
          cntct->host_notification_period_ptr) == ERROR) {
      logger(dbg_notifications, most)
        << "This contact shouldn't be notified at this time.";
      return ERROR;
    }
  }

  /*********************************************/
  /*** SPECIAL CASE FOR CUSTOM NOTIFICATIONS ***/
  /*********************************************/

  /* custom notifications are good to go at this point... */
  if (type == NOTIFICATION_CUSTOM)
    return OK;

  /****************************************/
  /*** SPECIAL CASE FOR FLAPPING ALERTS ***/
  /****************************************/

  if (type == NOTIFICATION_FLAPPINGSTART
      || type == NOTIFICATION_FLAPPINGSTOP
      || type == NOTIFICATION_FLAPPINGDISABLED) {

    if (!cntct->notify_on_host_flapping()) {
      logger(dbg_notifications, most)
        << "We shouldn't notify this contact about FLAPPING "
        "host events.";
      return ERROR;
    }

    return OK;
  }

  /****************************************/
  /*** SPECIAL CASE FOR DOWNTIME ALERTS ***/
  /****************************************/

  if (type == NOTIFICATION_DOWNTIMESTART
      || type == NOTIFICATION_DOWNTIMEEND
      || type == NOTIFICATION_DOWNTIMECANCELLED) {

    if (!cntct->notify_on_host_downtime()) {
      logger(dbg_notifications, most)
        << "We shouldn't notify this contact about DOWNTIME "
        "host events.";
      return ERROR;
    }

    return OK;
  }

  /*************************************/
  /*** ACKS AND NORMAL NOTIFICATIONS ***/
  /*************************************/

  /* see if we should notify about problems with this host */
  if (hst->get_current_state() == HOST_DOWN
      && !cntct->notify_on_host_down()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about DOWN states.";
    return ERROR;
  }

  if (hst->get_current_state() == HOST_UNREACHABLE
      && !cntct->notify_on_host_unreachable()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about UNREACHABLE states,";
    return ERROR;
  }

  if (hst->get_current_state() == HOST_UP) {

    if (!cntct->notify_on_host_recovery()) {
      logger(dbg_notifications, most)
        << "We shouldn't notify this contact about RECOVERY states.";
      return ERROR;
    }

    if (!((hst->get_notified_on_down()
           && cntct->notify_on_host_down())
          || (hst->get_notified_on_unreachable()
              && cntct->notify_on_host_unreachable()))) {
      logger(dbg_notifications, most)
        << "We shouldn't notify about this recovery.";
      return ERROR;
    }

  }

  logger(dbg_notifications, most)
    << "** Host notification viability for contact '"
    << cntct->get_name() << "' PASSED.";

  return OK;
}

/* notify a specific contact that an entire host is down or up */
int notify_contact_of_host(
      nagios_macros* mac,
      contact* cntct,
      host* hst,
      int type,
      char const* not_author,
      char const* not_data,
      int options,
      int escalated) {
  char* command_name = nullptr;
  char* raw_command = nullptr;
  char* processed_command = nullptr;
  int early_timeout = false;
  double exectime;
  struct timeval start_time;
  struct timeval end_time;
  struct timeval method_start_time;
  struct timeval method_end_time;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  int neb_result;

  logger(dbg_functions, basic)
    << "notify_contact_of_host()";
  logger(dbg_notifications, most)
    << "** Attempting to notifying contact '" << cntct->get_name() << "'...";

  /*
   * check viability of notifying this user about the host
   * acknowledgements are no longer excluded from this test -
   * added 8/19/02 Tom Bertelson
   */
  if (check_contact_host_notification_viability(
        cntct,
        hst,
        type,
        options) == ERROR)
    return ERROR;

  logger(dbg_notifications, most)
    << "** Notifying contact '" << cntct->get_name() << "'";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* send data to event broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_contact_notification_data(
                 NEBTYPE_CONTACTNOTIFICATION_START,
                 NEBFLAG_NONE,
                 NEBATTR_NONE,
                 HOST_NOTIFICATION,
                 type,
                 start_time,
                 end_time,
                 (void*)hst,
                 cntct,
                 not_author,
                 not_data,
                 escalated,
                 nullptr);
  if (NEBERROR_CALLBACKCANCEL == neb_result)
    return ERROR;
  else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
    return OK;

  /* process all the notification commands this user has */
  for (std::shared_ptr<commands::command> const& cmd :
        cntct->get_host_notification_commands()) {

    /* get start time */
    gettimeofday(&method_start_time, nullptr);

    /* send data to event broker */
    method_end_time.tv_sec = 0L;
    method_end_time.tv_usec = 0L;
    neb_result = broker_contact_notification_method_data(
                   NEBTYPE_CONTACTNOTIFICATIONMETHOD_START,
                   NEBFLAG_NONE,
                   NEBATTR_NONE,
                   HOST_NOTIFICATION,
                   type,
                   method_start_time,
                   method_end_time,
                   (void*)hst,
                   cntct,
                   cmd->get_command_line().c_str(),
                   not_author,
                   not_data,
                   escalated,
                   nullptr);
    if (NEBERROR_CALLBACKCANCEL == neb_result)
      break;
    else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
      continue;

    /* get the raw command line */
    get_raw_command_line_r(
      mac,
      cmd.get(),
      cmd->get_command_line().c_str(),
      &raw_command,
      macro_options);
    if (raw_command == nullptr)
      continue;

    logger(dbg_notifications, most)
      << "Raw notification command: " << raw_command;

    /* process any macros contained in the argument */
    process_macros_r(
      mac,
      raw_command,
      &processed_command,
      macro_options);
    if (processed_command == nullptr)
      continue;

    /* get the command name */
    command_name = string::dup(cmd->get_command_line());

    /* run the notification command... */

    logger(dbg_notifications, most)
      << "Processed notification command: " << processed_command;

    /* log the notification to program log file */
    if (config->log_notifications() == true) {
      char const* host_state_str("UP");
      if ((unsigned int)hst->get_current_state() <
           sizeof(tab_host_state_str) / sizeof(*tab_host_state_str))
        host_state_str = tab_host_state_str[hst->get_current_state()];

      char const* notification_str("");
      if ((unsigned int)type <
           sizeof(tab_notification_str) / sizeof(*tab_notification_str))
        notification_str = tab_notification_str[type];

      std::string info;
      switch (type) {
      case NOTIFICATION_CUSTOM:
        notification_str = "CUSTOM";

      case NOTIFICATION_ACKNOWLEDGEMENT:
        info
          .append(";").append(not_author ? not_author : "")
          .append(";").append(not_data ? not_data : "");
        break;
      }

      std::string host_notification_state;
      if (strcmp(notification_str, "NORMAL") == 0)
        host_notification_state.append(host_state_str);
      else
        host_notification_state
          .append(notification_str)
          .append(" (")
          .append(host_state_str)
          .append(")");

      logger(log_host_notification, basic)
        << "HOST NOTIFICATION: " << cntct->get_name()
        << ';' << hst->get_name() << ';' << host_notification_state
        << ";" << cmd->get_name() << ';' << hst->get_plugin_output()
        << info;
    }

    /* run the notification command */
    try {
      my_system_r(
        mac,
        processed_command,
        config->notification_timeout(),
        &early_timeout,
        &exectime,
        nullptr,
        0);
    } catch (std::exception const& e) {
      logger(log_runtime_error, basic)
        << "Error: can't execute host notification '"
        << cntct->get_name() << "' : " << e.what();
    }

    /* check to see if the notification timed out */
    if (early_timeout == true) {
      logger(log_host_notification | log_runtime_warning, basic)
        << "Warning: Contact '" << cntct->get_name()
        << "' host notification command '" << processed_command
        << "' timed out after " << config->notification_timeout()
        << " seconds";
    }

    /* free memory */
    delete[] raw_command;
    delete[] processed_command;

    /* get end time */
    gettimeofday(&method_end_time, nullptr);

    /* send data to event broker */
    broker_contact_notification_method_data(
      NEBTYPE_CONTACTNOTIFICATIONMETHOD_END,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      HOST_NOTIFICATION,
      type,
      method_start_time,
      method_end_time,
      (void*)hst,
      cntct,
      cmd->get_command_line().c_str(),
      not_author,
      not_data,
      escalated,
      nullptr);
  }

  /* get end time */
  gettimeofday(&end_time, nullptr);

  /* update the contact's last host notification time */
  cntct->set_last_host_notification(start_time.tv_sec);

  /* send data to event broker */
  broker_contact_notification_data(
    NEBTYPE_CONTACTNOTIFICATION_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_NOTIFICATION,
    type,
    start_time,
    end_time,
    (void*)hst,
    cntct,
    not_author,
    not_data,
    escalated,
    nullptr);

  return OK;
}

/*
 * checks to see if a host escalation entry is a match for the current host
 * notification
 */
int is_valid_escalation_for_host_notification(
      host* hst,
      hostescalation* he,
      int options) {
  int notification_number = 0;
  time_t current_time = 0L;
  host* temp_host = nullptr;

  logger(dbg_functions, basic)
    << "is_valid_escalation_for_host_notification()";

  /* get the current time */
  time(&current_time);

  /*
   * if this is a recovery, really we check for who got notified about a
   * previous problem
   */
  if (hst->get_current_state() == HOST_UP)
    notification_number = hst->get_current_notification_number() - 1;
  else
    notification_number = hst->get_current_notification_number();

  /* find the host this escalation entry is associated with */
  temp_host = he->host_ptr;
  if (temp_host == nullptr || temp_host != hst)
    return false;

  /*** EXCEPTION ***/
  /* broadcast options go to everyone, so this escalation is valid */
  if (options & NOTIFICATION_OPTION_BROADCAST)
    return true;

  /* skip this escalation if it happens later */
  if (he->get_first_notification() > notification_number)
    return false;

  /* skip this escalation if it has already passed */
  if (he->get_last_notification() != 0
      && he->get_last_notification() < notification_number)
    return false;

  /*
   * skip this escalation if it has a timeperiod and the current time
   * isn't valid
   */
  if (!he->get_escalation_period().empty()
      && check_time_against_period(
           current_time,
           he->escalation_period_ptr) == ERROR)
    return false;

  /* skip this escalation if the state options don't match */
  if (hst->get_current_state() == HOST_UP
      && !he->get_escalate_on_recovery())
    return false;
  else if (hst->get_current_state() == HOST_DOWN
           && !he->get_escalate_on_down())
    return false;
  else if (hst->get_current_state() == HOST_UNREACHABLE
           && !he->get_escalate_on_unreachable())
    return false;

  return true;
}

/* checks to see whether a host notification should be escalation */
int should_host_notification_be_escalated(host* hst) {
  logger(dbg_functions, basic)
    << "should_host_notification_be_escalated()";

  if (hst == nullptr)
    return false;

  std::string id(hst->get_name());
  umultimap<std::string, std::shared_ptr<hostescalation> > const&
    escalations(state::instance().hostescalations());
  for (umultimap<std::string, std::shared_ptr<hostescalation> >::const_iterator
         it(escalations.find(id)), end(escalations.end());
       it != end && it->first == id;
       ++it) {
    hostescalation* temp_he(&*it->second);

    /* we found a matching entry, so escalate this notification! */
    if (is_valid_escalation_for_host_notification(
          hst,
          temp_he,
          NOTIFICATION_OPTION_NONE) == true)
      return true;
  }

  logger(dbg_notifications, more)
    << "Host notification will NOT be escalated.";

  return false;
}

/*
 * given a host, create a list of contacts to be notified,
 * removing duplicates
 */
int create_notification_list_from_host(
      nagios_macros* mac,
      host* hst,
      int options,
      int* escalated) {
  int escalate_notification = false;

  logger(dbg_functions, basic)
    << "create_notification_list_from_host()";

  /* see if this notification should be escalated */
  escalate_notification = should_host_notification_be_escalated(hst);

  /* set the escalation flag */
  *escalated = escalate_notification;

  /* make sure there aren't any leftover contacts */
  free_notification_list();

  /* set the escalation macro */
  string::setstr(mac->x[MACRO_NOTIFICATIONISESCALATED], escalate_notification);

  if (options & NOTIFICATION_OPTION_BROADCAST)
    logger(dbg_notifications, more)
      << "This notification will be BROADCAST to all (escalated and "
      "normal) contacts...";

  /* use escalated contacts for this notification */
  if (escalate_notification == true
      || (options & NOTIFICATION_OPTION_BROADCAST)) {

    logger(dbg_notifications, more)
      << "Adding contacts from host escalation(s) to "
      "notification list.";

    std::string id(hst->get_name());
    umultimap<std::string, std::shared_ptr<hostescalation> > const&
      escalations(state::instance().hostescalations());
    for (umultimap<std::string,
                   std::shared_ptr<hostescalation> >::const_iterator
           it(escalations.find(id)), end(escalations.end());
         it != end && it->first == id;
         ++it) {
      hostescalation* temp_he(&*it->second);

      /* see if this escalation if valid for this notification */
      if (is_valid_escalation_for_host_notification(
            hst,
            temp_he,
            options) == false)
        continue;

      logger(dbg_notifications, most)
        << "Adding individual contacts from host escalation(s) "
        "to notification list.";

      /* add all individual contacts for this escalation */
      for(contact_map::iterator
            it(temp_he->contacts.begin()),
            end(temp_he->contacts.end());
          it != end;
          ++it)
        add_notification(mac, it->second.get());

      logger(dbg_notifications, most)
        << "Adding members of contact groups from host "
        "escalation(s) to notification list.";

      /* add all contacts that belong to contactgroups for this escalation */
      for (contactgroup_map::iterator
             it(temp_he->contact_groups.begin()),
             end(temp_he->contact_groups.end());
           it != end;
           ++it) {
        logger(dbg_notifications, most)
          << "Adding members of contact group '"
          << it->first
          << "' for host escalation to notification list.";

        if (it->second == nullptr)
          continue;
        for (std::unordered_map<std::string, contact *>::const_iterator
               itm(it->second->get_members().begin()),
               endm(it->second->get_members().end());
              itm != endm;
              ++itm) {
          if (itm->second == nullptr)
            continue;
          add_notification(mac, itm->second);
        }
      }
    }
  }

  /* use normal, non-escalated contacts for this notification */
  if (escalate_notification == false
      || (options & NOTIFICATION_OPTION_BROADCAST)) {

    logger(dbg_notifications, more)
      << "Adding normal contacts for host to notification list.";

    logger(dbg_notifications, most)
      << "Adding individual contacts for host to notification list.";

    /* add all individual contacts for this host */
    for (contact_map::iterator
           it(hst->contacts.begin()),
           end(hst->contacts.end());
         it != end;
         ++it)
      add_notification(mac, it->second.get());

    logger(dbg_notifications, most)
      << "Adding members of contact groups for host to "
      "notification list.";

    /* add all contacts that belong to contactgroups for this host */
    for (contactgroup_map::iterator
           it(hst->contact_groups.begin()),
           end(hst->contact_groups.end());
         it != end;
         ++it) {
      logger(dbg_notifications, most)
        << "Adding members of contact group '"
        << it->first
        << "' for host to notification list.";

      if (it->second == nullptr)
        continue;
      for (std::unordered_map<std::string, contact *>::const_iterator
             itm(it->second->get_members().begin()),
             endm(it->second->get_members().end());
            itm != endm;
            ++itm) {
        if (itm->second == nullptr)
          continue;
        add_notification(mac, itm->second);
      }
    }
  }

  return OK;
}

/******************************************************************/
/***************** NOTIFICATION TIMING FUNCTIONS ******************/
/******************************************************************/

/* calculates next acceptable re-notification time for a service */
time_t get_next_service_notification_time(com::centreon::engine::service2* svc, time_t offset) {
  time_t next_notification = 0L;
  double interval_to_use = 0.0;
  serviceescalation* temp_se = nullptr;
  int have_escalated_interval = false;

  logger(dbg_functions, basic)
    << "get_next_service_notification_time()";
  logger(dbg_notifications, most)
    << "Calculating next valid notification time...";

  /* default notification interval */
  interval_to_use = svc->notification_interval;

  logger(dbg_notifications, most)
    << "Default interval: " << interval_to_use;

  /*
   * search all the escalation entries for valid matches for this service (at
   * its current notification number)
   */
  for (temp_se = serviceescalation_list;
       temp_se != nullptr;
       temp_se = temp_se->next) {

    /* interval < 0 means to use non-escalated interval */
    if (temp_se->notification_interval < 0.0)
      continue;

    /* skip this entry if it isn't appropriate */
    if (is_valid_escalation_for_service_notification(
          svc,
          temp_se,
          NOTIFICATION_OPTION_NONE) == false)
      continue;

    logger(dbg_notifications, most)
      << "Found a valid escalation w/ interval of "
      << temp_se->notification_interval;

    /*
     * if we haven't used a notification interval from an escalation yet,
     * use this one
     */
    if (have_escalated_interval == false) {
      have_escalated_interval = true;
      interval_to_use = temp_se->notification_interval;
    }

    /* else use the shortest of all valid escalation intervals */
    else if (temp_se->notification_interval < interval_to_use)
      interval_to_use = temp_se->notification_interval;

    logger(dbg_notifications, most)
      << "New interval: " << interval_to_use;
  }

  /*
   * if notification interval is 0, we shouldn't send any more problem
   * notifications (unless service is volatile)
   */
  if (interval_to_use == 0.0 && svc->is_volatile == false)
    svc->no_more_notifications = true;
  else
    svc->no_more_notifications = false;

  logger(dbg_notifications, most)
    << "Interval used for calculating next valid "
    "notification time: " << interval_to_use;

  /* calculate next notification time */
  next_notification = offset + static_cast<time_t>(interval_to_use *
    config->interval_length());
  return next_notification;
}

/* calculates next acceptable re-notification time for a host */
time_t get_next_host_notification_time(host* hst, time_t offset) {
  time_t next_notification = 0L;
  double interval_to_use = 0.0;
  hostescalation* temp_he = nullptr;
  int have_escalated_interval = false;

  logger(dbg_functions, basic)
    << "get_next_host_notification_time()";
  logger(dbg_notifications, most)
    << "Calculating next valid notification time...";

  /* default notification interval */
  interval_to_use = hst->get_notification_interval();

  logger(dbg_notifications, most)
    << "Default interval: " << interval_to_use;

  /*
   * check all the host escalation entries for valid matches for this host
   * (at its current notification number)
   */
    for (hostescalation_mmap::iterator
           it(hostescalation::hostescalations.begin()),
           end(hostescalation::hostescalations.begin());
         it != end;
         ++it) {

    /* interval < 0 means to use non-escalated interval */
    if (it->second->get_notification_interval() < 0.0)
      continue;

    /* skip this entry if it isn't appropriate */
    if (is_valid_escalation_for_host_notification(
          hst,
          it->second.get(),
          NOTIFICATION_OPTION_NONE) == false)
      continue;

    logger(dbg_notifications, most)
      << "Found a valid escalation w/ interval of "
      << temp_he->get_notification_interval();

    /*
     * if we haven't used a notification interval from an escalation yet,
     * use this one
     */
    if (have_escalated_interval == false) {
      have_escalated_interval = true;
      interval_to_use = it->second->get_notification_interval();
    }

    /* else use the shortest of all valid escalation intervals  */
    else if (it->second->get_notification_interval() < interval_to_use)
      interval_to_use = it->second->get_notification_interval();

    logger(dbg_notifications, most)
      << "New interval: " << interval_to_use;
  }

  /* if interval is 0, no more notifications should be sent */
  if (interval_to_use == 0.0)
    hst->set_no_more_notifications(true);
  else
    hst->set_no_more_notifications(false);

  logger(dbg_notifications, most)
    << "Interval used for calculating next valid notification time: "
    << interval_to_use;

  /* calculate next notification time */
  next_notification = static_cast<time_t>(offset +
    (interval_to_use * config->interval_length()));

  return next_notification;
}

/******************************************************************/
/***************** NOTIFICATION OBJECT FUNCTIONS ******************/
/******************************************************************/

/*
 * given a contact name, find the notification entry for them for the list in
 * memory
 */
notification* find_notification(contact* cntct) {
  logger(dbg_functions, basic)
    << "find_notification()";

  if (cntct == nullptr)
    return nullptr;

  for (notification* temp_notification = notification_list;
       temp_notification != nullptr;
       temp_notification = temp_notification->next) {
    if (temp_notification->cntct == cntct)
      return temp_notification;
  }

  /* we couldn't find the contact in the notification list */
  return nullptr;
}

/* add a new notification to the list in memory */
int add_notification(nagios_macros* mac, contact* cntct) {
  logger(dbg_functions, basic)
    << "add_notification()";

  if (cntct == nullptr)
    return ERROR;

  logger(dbg_notifications, most)
    << "Adding contact '" << cntct->get_name() << "' to notification list.";

  /* don't add anything if this contact is already on the notification list */
  if (find_notification(cntct))
    return OK;

  /* allocate memory for a new contact in the notification list */
  notification* new_notification(new notification);

  /* fill in the contact info */
  new_notification->cntct = cntct;

  /* add new notification to head of list */
  new_notification->next = notification_list;
  notification_list = new_notification;

  /* add contact to notification recipients macro */
  if (!mac->x[MACRO_NOTIFICATIONRECIPIENTS])
    string::setstr(mac->x[MACRO_NOTIFICATIONRECIPIENTS], cntct->get_name());
  else {
    std::string buffer(mac->x[MACRO_NOTIFICATIONRECIPIENTS]);
    buffer += ",";
    buffer += cntct->get_name();
    string::setstr(mac->x[MACRO_NOTIFICATIONRECIPIENTS], buffer);
  }

  return OK;
}
