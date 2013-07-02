/*
** Copyright 2011-2013 Merethis
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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/servicedependency.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/servicedependency.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

#define CMP_CSTR(str1, str2) \
  if ((!!(str1) ^ !!(str2)) \
      || ((str1) && (str2) && strcmp((str1), (str2)))) \
    return (strcmp((str1), (str2)) < 0);

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       servicedependency const& obj1,
       servicedependency const& obj2) throw () {
  return (obj1.dependency_type == obj2.dependency_type
          && is_equal(obj1.dependent_host_name, obj2.dependent_host_name)
          && is_equal(obj1.dependent_service_description, obj2.dependent_service_description)
          && is_equal(obj1.host_name, obj2.host_name)
          && is_equal(obj1.service_description, obj2.service_description)
          && is_equal(obj1.dependency_period, obj2.dependency_period)
          && obj1.inherits_parent == obj2.inherits_parent
          && obj1.fail_on_ok == obj2.fail_on_ok
          && obj1.fail_on_warning == obj2.fail_on_warning
          && obj1.fail_on_unknown == obj2.fail_on_unknown
          && obj1.fail_on_critical == obj2.fail_on_critical
          && obj1.fail_on_pending == obj2.fail_on_pending
          && obj1.circular_path_checked == obj2.circular_path_checked
          && obj1.contains_circular_path == obj2.contains_circular_path);
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       servicedependency const& obj1,
       servicedependency const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Less-than operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if the first object is strictly less than the second.
 */
bool operator<(
       servicedependency const& obj1,
       servicedependency const& obj2) {
  CMP_CSTR(obj1.dependent_host_name, obj2.dependent_host_name)
  else
    CMP_CSTR(
      obj1.dependent_service_description,
      obj2.dependent_service_description)
  else
    CMP_CSTR(obj1.host_name, obj2.host_name)
  else
    CMP_CSTR(obj1.service_description, obj2.service_description)
  else
    CMP_CSTR(obj1.dependency_period, obj2.dependency_period)
  else if (obj1.inherits_parent != obj2.inherits_parent)
    return (obj1.inherits_parent < obj2.inherits_parent);
  else if (obj1.fail_on_ok != obj2.fail_on_ok)
    return (obj1.fail_on_ok < obj2.fail_on_ok);
  else if (obj1.fail_on_warning != obj2.fail_on_warning)
    return (obj1.fail_on_warning < obj2.fail_on_warning);
  else if (obj1.fail_on_critical != obj2.fail_on_critical)
    return (obj1.fail_on_critical < obj2.fail_on_critical);
  else if (obj1.fail_on_unknown != obj2.fail_on_unknown)
    return (obj1.fail_on_unknown < obj2.fail_on_unknown);
  else if (obj1.fail_on_pending != obj2.fail_on_pending)
    return (obj1.fail_on_pending < obj2.fail_on_pending);
  else if (obj1.circular_path_checked != obj2.circular_path_checked)
    return (obj1.circular_path_checked < obj2.circular_path_checked);
  return (obj1.contains_circular_path < obj2.contains_circular_path);
}

/**
 *  Dump servicedependency content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The servicedependency to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, servicedependency const& obj) {
  char const* dependency_period_str(NULL);
  if (obj.dependency_period_ptr)
    dependency_period_str = chkstr(obj.dependency_period_ptr->name);
  std::string dependent_svc_str("\"NULL\"");
  if (obj.dependent_service_ptr) {
    dependent_svc_str = obj.dependent_service_ptr->host_name;
    dependent_svc_str += ", ";
    dependent_svc_str += obj.dependent_service_ptr->description;
  }
  std::string master_svc_str("\"NULL\"");
  if (obj.master_service_ptr) {
    master_svc_str = obj.master_service_ptr->host_name;
    master_svc_str += ", ";
    master_svc_str += obj.master_service_ptr->description;
  }

  os << "servicedependency {\n"
    "  dependency_type:               " << obj.dependency_type << "\n"
    "  dependent_host_name:           " << chkstr(obj.dependent_host_name) << "\n"
    "  dependent_service_description: " << chkstr(obj.dependent_service_description) << "\n"
    "  host_name:                     " << chkstr(obj.host_name) << "\n"
    "  service_description:           " << chkstr(obj.service_description) << "\n"
    "  dependency_period:             " << chkstr(obj.dependency_period) << "\n"
    "  inherits_parent:               " << obj.inherits_parent << "\n"
    "  fail_on_ok:                    " << obj.fail_on_ok << "\n"
    "  fail_on_warning:               " << obj.fail_on_warning << "\n"
    "  fail_on_unknown:               " << obj.fail_on_unknown << "\n"
    "  fail_on_critical:              " << obj.fail_on_critical << "\n"
    "  fail_on_pending:               " << obj.fail_on_pending << "\n"
    "  circular_path_checked:         " << obj.circular_path_checked << "\n"
    "  contains_circular_path:        " << obj.contains_circular_path << "\n"
    "  master_service_ptr:            " << master_svc_str << "\n"
    "  dependent_service_ptr:         " << dependent_svc_str << "\n"
    "  dependency_period_ptr:         " << chkstr(dependency_period_str) << "\n"
    "}\n";
  return (os);
}


/**
 *  Adds a service dependency definition.
 *
 *  @param[in] dependent_host_name           Dependent host name.
 *  @param[in] dependent_service_description Dependent service
 *                                           description.
 *  @param[in] host_name                     Host name.
 *  @param[in] service_description           Service description.
 *  @param[in] dependency_type               Type of dependency.
 *  @param[in] inherits_parent               Inherits parent ?
 *  @param[in] fail_on_ok                    Does dependency fail on
 *                                           ok state ?
 *  @param[in] fail_on_warning               Does dependency fail on
 *                                           warning state ?
 *  @param[in] fail_on_unknown               Does dependency fail on
 *                                           unknown state ?
 *  @param[in] fail_on_critical              Does dependency fail on
 *                                           critical state ?
 *  @param[in] fail_on_pending               Does dependency fail on
 *                                           pending state ?
 *  @param[in] dependency_period             Dependency timeperiod name.
 *
 *  @return Service dependency.
 */
servicedependency* add_service_dependency(
                     char const* dependent_host_name,
                     char const* dependent_service_description,
                     char const* host_name,
                     char const* service_description,
                     int dependency_type,
                     int inherits_parent,
                     int fail_on_ok,
                     int fail_on_warning,
                     int fail_on_unknown,
                     int fail_on_critical,
                     int fail_on_pending,
                     char const* dependency_period) {
  // Make sure we have what we need.
  if (!host_name
      || !host_name[0]
      || !service_description
      || !service_description[0]) {
    logger(log_config_error, basic)
      << "Error: NULL master service description/host "
         "name in service dependency definition";
    return (NULL);
  }
  if (!dependent_host_name
      || !dependent_host_name[0]
      || !dependent_service_description
      || !dependent_service_description[0]) {
    logger(log_config_error, basic)
      << "Error: NULL dependent service description/host "
         "name in service dependency definition";
    return (NULL);
  }

  // Allocate memory for a new service dependency entry.
  shared_ptr<servicedependency> obj(new servicedependency, deleter::servicedependency);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->dependent_host_name = string::dup(dependent_host_name);
    obj->dependent_service_description = string::dup(dependent_service_description);
    obj->host_name = string::dup(host_name);
    obj->service_description = string::dup(service_description);
    if (dependency_period)
      obj->dependency_period = string::dup(dependency_period);

    obj->dependency_type = (dependency_type == EXECUTION_DEPENDENCY) ? EXECUTION_DEPENDENCY : NOTIFICATION_DEPENDENCY;
    obj->fail_on_critical = (fail_on_critical == 1);
    obj->fail_on_ok = (fail_on_ok == 1);
    obj->fail_on_pending = (fail_on_pending == 1);
    obj->fail_on_unknown = (fail_on_unknown == 1);
    obj->fail_on_warning = (fail_on_warning == 1);
    obj->inherits_parent = (inherits_parent > 0);

    // Add new items to the configuration state.
    std::pair<std::string, std::string>
      id(std::make_pair(dependent_host_name, dependent_service_description));
    state::instance().servicedependencies()
      .insert(std::make_pair(id, obj));

    // Add new items to the list.
    obj->next = servicedependency_list;
    servicedependency_list = obj.get();

    // Notify event broker.
    // XXX
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Checks to see if there exists a circular dependency for a service.
 *
 *  @param[in] root_dep        Root dependency.
 *  @param[in] dep             Dependency.
 *  @param[in] dependency_type Dependency type.
 *
 *  @return true if circular path was found, false otherwise.
 */
int check_for_circular_servicedependency_path(
      servicedependency* root_dep,
      servicedependency* dep,
      int dependency_type) {
  if (!root_dep || !dep)
    return (false);

  // This is not the proper dependency type.
  if ((root_dep->dependency_type != dependency_type)
      || (dep->dependency_type != dependency_type))
    return (false);

  // Don't go into a loop, don't bother checking anymore if we know this
  // dependency already has a loop.
  if (root_dep->contains_circular_path == true)
    return (true);

  // Dependency has already been checked - there is a path somewhere,
  // but it may not be for this particular dep... This should speed up
  // detection for some loops.
  if (dep->circular_path_checked == true)
    return (false);

  // Set the check flag so we don't get into an infinite loop.
  dep->circular_path_checked = true;

  // Is this service dependent on the root service?
  if (dep != root_dep) {
    if (root_dep->dependent_service_ptr == dep->master_service_ptr) {
      root_dep->contains_circular_path = true;
      dep->contains_circular_path = true;
      return (true);
    }
  }

  // Notification dependencies are ok at this point as long as they
  // don't inherit.
  if ((dependency_type == NOTIFICATION_DEPENDENCY)
      && (dep->inherits_parent == false))
    return (false);

  // Check all parent dependencies.
  for (servicedependency* temp_sd(servicedependency_list);
       temp_sd;
       temp_sd = temp_sd->next) {
    // Only check parent dependencies.
    if (dep->master_service_ptr != temp_sd->dependent_service_ptr)
      continue;

    if (check_for_circular_servicedependency_path(
          root_dep,
          temp_sd,
          dependency_type) == true)
      return (true);
  }

  return (false);
}
