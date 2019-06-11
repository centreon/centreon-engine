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
/******************* HOST NOTIFICATION FUNCTIONS ******************/
/******************************************************************/

/******************************************************************/
/***************** NOTIFICATION TIMING FUNCTIONS ******************/
/******************************************************************/

/******************************************************************/
/***************** NOTIFICATION OBJECT FUNCTIONS ******************/
/******************************************************************/

/* add a new notification to the list in memory */
int add_notification(nagios_macros* mac, contact* cntct) {
  logger(dbg_functions, basic)
    << "add_notification()";

  if (cntct == nullptr)
    return ERROR;

  logger(dbg_notifications, most)
    << "Adding contact '" << cntct->get_name() << "' to notification list.";

  /* don't add anything if this contact is already on the notification list */
  if (cntct->find_notification())
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
