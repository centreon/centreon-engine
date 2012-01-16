/*
** Copyright 2011 Merethis
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

#include "error.hh"
#include "globals.hh"
#include "skiplist.hh"
#include "logging/logger.hh"
#include "objects/utils.hh"
#include "objects/servicedependency.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_servicedependency(servicedependency* obj,
                            timeperiod* dependency_period) {
  try {
    objects::link(obj, dependency_period);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
    return (false);
  }
  catch (...) {
    logger(log_runtime_error, basic) << Q_FUNC_INFO << " unknow exception.";
    return (false);
  }
  return (true);
}

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_servicedependency(servicedependency const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic) << Q_FUNC_INFO << " unknow exception.";
  }
}

/**
 *  Link a servicedependency with hostgroups, dependent hostgroups and dependency
 *  period into the engine.
 *
 *  @param[in, out] obj                  Object to link with a correct name.
 *  @param[in]      dependency_period    Set servicedependency timeperiod.
 */
void objects::link(servicedependency* obj,
                   timeperiod* dependency_period) {
  // check object contents.
  if (obj == NULL)
    throw (engine_error() << "servicedependency is a NULL pointer.");
  if (obj->host_name == NULL)
    throw (engine_error() << "servicedependency invalid host name.");
  if (obj->dependent_host_name == NULL)
    throw (engine_error() << "servicedependency invalid dependent host name.");
  if (obj->service_description == NULL)
    throw (engine_error() << "servicedependency invalid service description.");
  if (obj->dependent_service_description == NULL)
    throw (engine_error() << "servicedependency invalid dependent service description.");

  if ((obj->master_service_ptr = find_service(obj->host_name, obj->service_description)) == NULL)
    throw (engine_error() << "servicedependency '" << obj->dependent_host_name
           << ", " << obj->dependent_service_description
	   << ", " << obj->host_name
           << ", " << obj->service_description
	   << "' invalid host name.");

  if ((obj->dependent_service_ptr = find_service(obj->dependent_host_name,
                                                 obj->dependent_service_description)) == NULL)
    throw (engine_error() << "servicedependency '" << obj->dependent_host_name
           << ", " << obj->dependent_service_description
	   << ", " << obj->host_name
           << ", " << obj->service_description
           << "' invalid dependent host name.");

  obj->dependency_period_ptr = dependency_period;
}

/**
 *  Cleanup memory of hostdependency.
 *
 *  @param[in] obj The hostdependency to cleanup memory.
 */
void objects::release(servicedependency const* obj) {
  if (obj == NULL)
    return;

  skiplist_delete(object_skiplists[SERVICEDEPENDENCY_SKIPLIST], obj);
  remove_object_list(obj, &servicedependency_list, &servicedependency_list_tail);

  delete[] obj->dependent_host_name;
  delete[] obj->dependent_service_description;
  delete[] obj->host_name;
  delete[] obj->service_description;
  delete[] obj->dependency_period;
  delete obj;
}
