/*
** Copyright 2011-2012 Merethis
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

#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/webservice/commands.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "soapH.h"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Create a new timeperiod.
 *
 *  @param[in]  s     SOAP object.
 *  @param[in]  tmprd Timeperiod properties.
 *  @param[out] res   Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__timeperiodAdd(
      soap* s,
      ns1__timeperiodType* tmprd,
      centreonengine__timeperiodAddResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(tmprd->id->name)

  // Create timeperiod.
  objects::add_timeperiod(
             tmprd->id->name.c_str(),
             tmprd->alias.c_str(),
             std2qt(tmprd->range),
             std2qt(tmprd->exclude));

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Modify an existing timeperiod.
 *
 *  @param[in]  s     SOAP object.
 *  @param[in]  tmprd Timeperiod properties.
 *  @param[out] res   Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__timeperiodModify(
      soap* s,
      ns1__timeperiodType* tmprd,
      centreonengine__timeperiodModifyResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(tmprd->id->name)

  // XXX

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Remove a timeperiod.
 *
 *  @param[in]  s             SOAP object.
 *  @param[in]  timeperiod_id Target timeperiod.
 *  @param[out] res           Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__timeperiodRemove(
      soap* s,
      ns1__timeperiodIDType* timeperiod_id,
      centreonengine__timeperiodRemoveResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(timeperiod_id->name)

  // Find target timeperiod.
  timeperiod* tmprd(find_timeperiod(timeperiod_id->name.c_str()));
  if (tmprd) {
    // Check link with contacts.
    for (contact* cntct(contact_list); cntct; cntct = cntct->next)
      if ((cntct->host_notification_period_ptr == tmprd)
          || (cntct->service_notification_period_ptr == tmprd))
        throw (engine_error() << "cannot remove timeperiod '"
               << timeperiod_id->name
               << "': used by at least one contact");

    // Check link with hosts.
    for (host* hst(host_list); hst; hst = hst->next)
      if ((hst->check_period_ptr == tmprd)
          || (hst->notification_period_ptr == tmprd))
        throw (engine_error() << "cannot remove timeperiod '"
               << timeperiod_id->name
               << "': used by at least one host");

    // Check link with services.
    for (service* svc(service_list); svc; svc = svc->next)
      if ((svc->check_period_ptr == tmprd)
          || (svc->notification_period_ptr == tmprd))
        throw (engine_error() << "cannot remove timeperiod '"
               << timeperiod_id->name
               << "': used by at least one service");

    // Remove timeperiod.
    remove_timeperiod_by_id(timeperiod_id->name.c_str());
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}
