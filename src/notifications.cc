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
#include "com/centreon/engine/timezone_locker.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;

/******************************************************************/
/***************** SERVICE NOTIFICATION FUNCTIONS *****************/
/******************************************************************/

/*
 * check viability of sending out a service notification to a specific contact
 * (contact-specific filters)
 */
int check_contact_service_notification_viability(
      contact* cntct,
      com::centreon::engine::service* svc,
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
  if (svc->get_current_state() == service::state_unknown
      && !cntct->notify_on_service_unknown()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about UNKNOWN "
      "service states.";
    return ERROR;
  }

  if (svc->get_current_state() == service::state_warning
      && !cntct->notify_on_service_warning()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about WARNING "
      "service states.";
    return ERROR;
  }

  if (svc->get_current_state() == service::state_critical
      && !cntct->notify_on_service_critical()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about CRITICAL "
      "service states.";
    return ERROR;
  }

  if (svc->get_current_state() == service::state_ok) {

    if (!cntct->notify_on_service_recovery()) {
      logger(dbg_notifications, most)
        << "We shouldn't notify this contact about RECOVERY "
        "service states.";
      return ERROR;
    }

    if (!((svc->get_notified_on(notifier::unknown)
           && cntct->notify_on_service_unknown())
          || (svc->get_notified_on(notifier::warning)
              && cntct->notify_on_service_warning())
          || (svc->get_notified_on(notifier::critical)
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


/******************************************************************/
/******************* HOST NOTIFICATION FUNCTIONS ******************/
/******************************************************************/

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
  if (hst->get_current_state() == host::state_down
      && !cntct->notify_on_host_down()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about DOWN states.";
    return ERROR;
  }

  if (hst->get_current_state() == host::state_unreachable
      && !cntct->notify_on_host_unreachable()) {
    logger(dbg_notifications, most)
      << "We shouldn't notify this contact about UNREACHABLE states,";
    return ERROR;
  }

  if (hst->get_current_state() == host::state_up) {

    if (!cntct->notify_on_host_recovery()) {
      logger(dbg_notifications, most)
        << "We shouldn't notify this contact about RECOVERY states.";
      return ERROR;
    }

    if (!((hst->get_notified_on(notifier::down)
           && cntct->notify_on_host_down())
          || (hst->get_notified_on(notifier::unreachable)
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

/******************************************************************/
/***************** NOTIFICATION TIMING FUNCTIONS ******************/
/******************************************************************/

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
