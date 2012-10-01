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

#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/hostdependency.hh"
#include "com/centreon/engine/objects/utils.hh"
#include "com/centreon/engine/skiplist.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_hostdependency(
       hostdependency* obj,
       timeperiod* dependency_period) {
  try {
    objects::link(obj, dependency_period);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
    return (false);
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << __func__ << " unknow exception";
    return (false);
  }
  return (true);
}

 /**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_hostdependency(hostdependency const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << __func__ << " unknow exception";
  }
  return;
}

/**
 *  Link an hostdependency with hostgroups, dependent_hostgroups and dependency
 *  period into the engine.
 *
 *  @param[in, out] obj                  Object to link with a correct name.
 *  @param[in]      dependency_period    Set hostdependency timeperiod.
 */
void objects::link(hostdependency* obj, timeperiod* dependency_period) {
  // check object contents.
  if (obj == NULL)
    throw (engine_error() << "hostdependency is a NULL pointer.");
  if (obj->host_name == NULL)
    throw (engine_error() << "hostdependency invalid host name.");
  if (obj->dependent_host_name == NULL)
    throw (engine_error()
           << "hostdependency invalid dependent host name.");

  if ((obj->master_host_ptr = find_host(obj->host_name)) == NULL)
    throw (engine_error()
           << "hostdependency '" << obj->dependent_host_name
           << ", " << obj->host_name << "' invalid host name.");

  if ((obj->dependent_host_ptr = find_host(obj->dependent_host_name)) == NULL)
    throw (engine_error()
           << "hostdependency '" << obj->dependent_host_name << ", "
           << obj->host_name << "' invalid dependent host name.");

  obj->dependency_period_ptr = dependency_period;
  return;
}

/**
 *  Cleanup memory of hostdependency.
 *
 *  @param[in] obj The hostdependency to cleanup memory.
 */
void objects::release(hostdependency const* obj) {
  if (obj == NULL)
    return;

  skiplist_delete(object_skiplists[HOSTDEPENDENCY_SKIPLIST], obj);
  remove_object_list(
    obj,
    &hostdependency_list,
    &hostdependency_list_tail);

  delete[] obj->dependent_host_name;
  delete[] obj->host_name;
  delete[] obj->dependency_period;
  delete obj;
  return;
}
