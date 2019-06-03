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

/*
 * checks to see if a service escalation entry is a match for the current
 * service notification
 */
int is_valid_escalation_for_service_notification(
      com::centreon::engine::service* svc,
      serviceescalation* se,
      int options) {
  int notification_number = 0;
  time_t current_time = 0L;
  com::centreon::engine::service* temp_service = nullptr;

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
int should_service_notification_be_escalated(com::centreon::engine::service* svc) {
  // Debug.
  logger(dbg_functions, basic)
    << "should_service_notification_be_escalated()";

  // Browse service escalations related to this service.
  typedef umultimap<std::pair<std::string, std::string>,
                    std::shared_ptr<serviceescalation> > collection;
  std::pair<collection::iterator, collection::iterator> p;
  p = state::instance().serviceescalations().equal_range(
        std::make_pair(svc->get_hostname(), svc->get_description()));
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
