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
#include "com/centreon/engine/xodtemplate.hh"
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
             (tmprd->alias ? tmprd->alias->c_str() : NULL),
             tmprd->range,
             tmprd->exclude);

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

  // Find existing timeperiod.
  timeperiod* tperiod(find_timeperiod(tmprd->id->name.c_str()));
  if (!tperiod)
    throw (engine_error() << "cannot modify non-existent timeperiod '"
           << tmprd->id->name << "'");

  // Operate on a template timeperiod.
  xodtemplate_timeperiod* tmpl(NULL);
  timeperiod* tp(NULL);
  try {
    tmpl = new xodtemplate_timeperiod;
    memset(tmpl, 0, sizeof(*tmpl));
    for (std::vector<std::string>::const_iterator
           it(tmprd->range.begin()),
           end(tmprd->range.end());
         it != end;
         ++it) {
      // Split directive.
      size_t first_space(it->find(' '));
      if (std::string::npos == first_space)
        throw (engine_error() << "invalid timeperiod directive '"
               << *it << "'");
      std::string key(it->substr(0, first_space));
      std::string
        value(it->substr(it->find_first_not_of(' ', first_space + 1)));
      if (xodtemplate_parse_timeperiod_directive(
            tmpl,
            key.c_str(),
            value.c_str()) == ERROR)
        throw (engine_error()
               << "error while parsing timeperiod directive '"
               << *it << "'");
    }

    // Temporary timeperiod object.
    tp = new timeperiod;
    memset(tp, 0, sizeof(*tp));
    if (xodtemplate_fill_timeperiod(tmpl, tp) == ERROR)
      throw (engine_error() << "cannot fill timeperiod");

    // Remove old timeperiod members (except name).
    std::auto_ptr<timeperiod> fake_tp(new timeperiod);
    memcpy(fake_tp.get(), tperiod, sizeof(tperiod));
    fake_tp->name = NULL;
    objects::release(fake_tp.release());
  }
  catch (...) {
    if (tmpl)
      xodtemplate_free_timeperiod(tmpl);
    if (tp)
      objects::release(tp);
    throw ;
  }

  // Set new members to timeperiod.
  tperiod->alias = (tmprd->alias
                    ? my_strdup(tmprd->alias->c_str())
                    : NULL);
  tperiod->exclusions = tperiod->exclusions;
  memcpy(tperiod->days, tp->days, sizeof(tperiod->days));
  memcpy(
    tperiod->exceptions,
    tp->exceptions,
    sizeof(tperiod->exceptions));

  // Free resources.
  xodtemplate_free_timeperiod(tmpl);
  delete tp;

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
