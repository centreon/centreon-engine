/*
** Copyright 2011-2014 Merethis
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

#include <algorithm>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/downtime.hh"
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/contactsmember.hh"
#include "com/centreon/engine/deleter/contactgroupsmember.hh"
#include "com/centreon/engine/deleter/customvariablesmember.hh"
#include "com/centreon/engine/deleter/hostsmember.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/deleter/servicesmember.hh"
#include "com/centreon/engine/configuration/downtime.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;


/**
 *  Default constructor.
 */
applier::downtime::downtime() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::downtime::downtime(applier::downtime const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::downtime::~downtime() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::downtime& applier::downtime::operator=(applier::downtime const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new downtime.
 *
 *  @param[in] obj The new downtime to add into the monitoring engine.
 */
void applier::downtime::add_object(
               shared_ptr<configuration::downtime> obj) {
  // If no recurring period, do nothing.
  obj->resolve_recurring_period();
  if (!obj->recurring_period())
    return ;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new downtime.";

  // Add downtime to the global configuration set.
  config->downtimes().insert(obj);

  // Create downtime.
  int ret = OK;
  unsigned long id;
  time_t new_start_time, new_end_time;
  get_new_recurring_times(obj->start_time(), obj->end_time(),
                          obj->recurring_interval(),
                          obj->recurring_period(),
                          &new_start_time, &new_end_time);
  ret = add_new_downtime(obj->downtime_type(),
                         obj->host_name().c_str(),
                         obj->service_description().c_str(),
                         obj->entry_time(),
                         obj->author().c_str(),
                         obj->comment_data().c_str(),
                         new_start_time,
                         new_end_time,
                         obj->fixed(),
                         0,
                         obj->duration(),
                         obj->recurring_interval(),
                         obj->recurring_period(),
                         &id);
  if (ret == ERROR)
    throw (engine_error() << "Could not register downtime '"
         << obj->host_name() << "'");

  register_downtime(obj->downtime_type(), id);
}

/**
 *  Expand downtime.
 *
 *  @param  obj[in] the downtime to expand.
 *  @param  s[out] the configuration state.
 */
void applier::downtime::expand_object(
    shared_ptr<configuration::downtime> obj,
    configuration::state& s) {
  s.downtimes().insert(obj);
}

/**
 *  Modify downtime.
 *
 *  A downtime can never be modified: as a downtime is its own key, two
 *  non equal downtimes are always two new downtimes.
 *
 *  @param[in] obj The new downtime to modify into the monitoring engine.
 */
void applier::downtime::modify_object(
    shared_ptr<configuration::downtime> obj) {
  throw (engine_error() << "A recurring downtime should never be modified.");
  return ;
}

/**
 *  Remove a downtime from the monitoring engine.
 *
 *  @param[in] obj  The downtime to remove from the monitoring engine.
 */
void applier::downtime::remove_object(
    shared_ptr<configuration::downtime> obj) {
  // If no recurring period, do nothing.
  obj->resolve_recurring_period();
  if (!obj->recurring_period())
    return ;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing recurring downtime.";

  // Delete all the recurring downtimes which can be uniquely attributed
  // to this downtime.
  char const* host_name = NULL;
  char const* service_description = NULL;
  if (obj->downtime_type() == configuration::downtime::host
      && !obj->host_name().empty())
    host_name = obj->host_name().c_str();
  if (obj->downtime_type() == configuration::downtime::service
      && !obj->service_description().empty())
    service_description = obj->service_description().c_str();

  int num_deleted = delete_downtimes_by_unique_recurring_period_informations(
        host_name, service_description,
        obj->duration(),
        obj->recurring_interval(),
        obj->recurring_period(),
        obj->comment_data().empty() ? NULL : obj->comment_data().c_str());

  logger(logging::dbg_config, logging::more)
    << num_deleted << " recurring downtime removed.";

  // Remove downtime from the global configuration set.
  config->downtimes().erase(obj);

  return ;
}

/**
 *  Resolve downtime.
 *
 *  Basically, check that the recurring period exists and throw an error
 *  if it doesn't. This error will invalidate the whole configuration.
 *
 *  Also, handle sudden configuration rollbacks.
 *
 *  @param[in] obj The downtime to resolve.
 */
void applier::downtime::resolve_object(
    shared_ptr<configuration::downtime> obj) {

  // Check if the recurring period exists.
  ::timeperiod* old_period = obj->recurring_period();
  if (obj->resolve_recurring_period() == false) {
    throw (engine_error() << "Cannot resolve downtime.");
  }

  // A timeperiod pointer has suddenly changed! It can only happen
  // when rolling back after a configuration error (the timeperiod existed,
  // the configuration deleted it, the configuration was rolled back,
  // the timeperiod exists again with a new pointer). Fishy things are
  // going on: replace the old period pointer by the new.
  if (old_period != NULL && old_period != obj->recurring_period())
    replace_recurring_periods(old_period, obj->recurring_period());

  return ;
}
