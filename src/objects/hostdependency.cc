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
#include "com/centreon/engine/deleter/hostdependency.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/hostdependency.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::misc;

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
       hostdependency const& obj1,
       hostdependency const& obj2) throw () {
  return (obj1.dependency_type == obj2.dependency_type
          && is_equal(obj1.dependent_host_name, obj2.dependent_host_name)
          && is_equal(obj1.host_name, obj2.host_name)
          && is_equal(obj1.dependency_period, obj2.dependency_period)
          && obj1.inherits_parent == obj2.inherits_parent
          && obj1.fail_on_up == obj2.fail_on_up
          && obj1.fail_on_down == obj2.fail_on_down
          && obj1.fail_on_unreachable == obj2.fail_on_unreachable
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
       hostdependency const& obj1,
       hostdependency const& obj2) throw () {
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
       hostdependency const& obj1,
       hostdependency const& obj2) throw () {
  CMP_CSTR(obj1.dependent_host_name, obj2.dependent_host_name)
  else
    CMP_CSTR(obj1.host_name, obj2.host_name)
  else if (obj1.dependency_type != obj2.dependency_type)
    return (obj1.dependency_type < obj2.dependency_type);
  else
    CMP_CSTR(obj1.dependency_period, obj2.dependency_period)
  else if (obj1.inherits_parent != obj2.inherits_parent)
    return (obj1.inherits_parent < obj2.inherits_parent);
  else if (obj1.fail_on_up != obj2.fail_on_up)
    return (obj1.fail_on_up < obj2.fail_on_up);
  else if (obj1.fail_on_down != obj2.fail_on_down)
    return (obj1.fail_on_down < obj2.fail_on_down);
  else if (obj1.fail_on_unreachable != obj2.fail_on_unreachable)
    return (obj1.fail_on_unreachable < obj2.fail_on_unreachable);
  else if (obj1.fail_on_pending != obj2.fail_on_pending)
    return (obj1.fail_on_pending < obj2.fail_on_pending);
  else if (obj1.circular_path_checked != obj2.circular_path_checked)
    return (obj1.circular_path_checked < obj2.circular_path_checked);
  return (obj1.contains_circular_path < obj2.contains_circular_path);
}

/**
 *  Dump hostdependency content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The hostdependency to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, hostdependency const& obj) {
  char const* dependency_period_str(NULL);
  if (obj.dependency_period_ptr)
    dependency_period_str = chkstr(obj.dependency_period_ptr->name);
  char const* dependent_hst_str(NULL);
  if (obj.dependent_host_ptr)
    dependent_hst_str = obj.dependent_host_ptr->name;
  char const* master_hst_str(NULL);
  if (obj.master_host_ptr)
    master_hst_str = obj.master_host_ptr->name;

  os << "hostdependency {\n"
    "  dependency_type:        " << obj.dependency_type << "\n"
    "  dependent_host_name:    " << chkstr(obj.dependent_host_name) << "\n"
    "  host_name:              " << chkstr(obj.host_name) << "\n"
    "  dependency_period:      " << chkstr(obj.dependency_period) << "\n"
    "  inherits_parent:        " << obj.inherits_parent << "\n"
    "  fail_on_up:             " << obj.fail_on_up << "\n"
    "  fail_on_down:           " << obj.fail_on_down << "\n"
    "  fail_on_unreachable:    " << obj.fail_on_unreachable << "\n"
    "  fail_on_pending:        " << obj.fail_on_pending << "\n"
    "  circular_path_checked:  " << obj.circular_path_checked << "\n"
    "  contains_circular_path: " << obj.contains_circular_path << "\n"
    "  master_host_ptr:        " << chkstr(master_hst_str) << "\n"
    "  dependent_host_ptr:     " << chkstr(dependent_hst_str) << "\n"
    "  dependency_period_ptr:  " << chkstr(dependency_period_str) << "\n"
    "}\n";
  return (os);
}

/**
 *  Adds a host dependency definition.
 *
 *  @param[in] dependent_host_name Dependant host name.
 *  @param[in] host_name           Host name.
 *  @param[in] dependency_type     Dependency type.
 *  @param[in] inherits_parent     Do we inherits from parent ?
 *  @param[in] fail_on_up          Does dependency fail on up ?
 *  @param[in] fail_on_down        Does dependency fail on down ?
 *  @param[in] fail_on_unreachable Does dependency fail on unreachable ?
 *  @param[in] fail_on_pending     Does dependency fail on pending ?
 *  @param[in] dependency_period   Dependency period.
 *
 *  @return New host dependency.
 */
hostdependency* add_host_dependency(
                  char const* dependent_host_name,
                  char const* host_name,
                  int dependency_type,
                  int inherits_parent,
                  int fail_on_up,
                  int fail_on_down,
                  int fail_on_unreachable,
                  int fail_on_pending,
                  char const* dependency_period) {
  // Make sure we have what we need.
  if (!dependent_host_name
      || !dependent_host_name[0]
      || !host_name
      || !host_name[0]) {
    logger(log_config_error, basic)
      << "Error: NULL host name in host dependency definition";
    return (NULL);
  }

  // Allocate memory for a new host dependency entry.
  shared_ptr<hostdependency> obj(new hostdependency, deleter::hostdependency);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->dependent_host_name = my_strdup(dependent_host_name);
    obj->host_name = my_strdup(host_name);
    if (dependency_period)
      obj->dependency_period = my_strdup(dependency_period);
    obj->dependency_type = (dependency_type == EXECUTION_DEPENDENCY ? EXECUTION_DEPENDENCY : NOTIFICATION_DEPENDENCY);
    obj->fail_on_down = (fail_on_down == 1);
    obj->fail_on_pending = (fail_on_pending == 1);
    obj->fail_on_unreachable = (fail_on_unreachable == 1);
    obj->fail_on_up = (fail_on_up == 1);
    obj->inherits_parent = (inherits_parent > 0);

    // Add new items to the configuration state.
    state::instance().hostdependencies()
      .insert(std::make_pair(dependent_host_name, obj));

    // Add new items to the list.
    obj->next = hostdependency_list;
    hostdependency_list = obj.get();

    // Notify event broker.
    // XXX
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Checks to see if there exists a circular dependency for a host.
 *
 *  @param[in] root_dep        Root dependency.
 *  @param[in] dep             Dependency.
 *  @param[in] dependency_type Dependency type.
 *
 *  @return true if circular path was found, false otherwise.
 */
int check_for_circular_hostdependency_path(
      hostdependency* root_dep,
      hostdependency* dep,
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

  // Is this host dependent on the root host?
  if (dep != root_dep) {
    if (root_dep->dependent_host_ptr == dep->master_host_ptr) {
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
  for (hostdependency* temp_hd(hostdependency_list);
       temp_hd;
       temp_hd = temp_hd->next) {
    // Only check parent dependencies.
    if (dep->master_host_ptr != temp_hd->dependent_host_ptr)
      continue;

    if (check_for_circular_hostdependency_path(
          root_dep,
          temp_hd,
          dependency_type) == true)
      return (true);
  }

  return (false);
}

