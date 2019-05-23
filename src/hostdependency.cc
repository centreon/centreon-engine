/*
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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/hostdependency.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

hostdependency_mmap hostdependency::hostdependencies;

/**
 *  Create a host dependency definition.
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
hostdependency::hostdependency(std::string const& dependent_host_name,
                               std::string const& host_name,
                               types dependency_type,
                               bool inherits_parent,
                               bool fail_on_up,
                               bool fail_on_down,
                               bool fail_on_unreachable,
                               bool fail_on_pending,
                               std::string const& dependency_period) {
  if (dependent_host_name.empty() || host_name.empty()) {
    logger(log_config_error, basic)
      << "Error: NULL host name in host dependency definition";
    throw (engine_error() << "Could not create host execution "
                          << "dependency of '" << dependent_host_name
                          << "' on '" << host_name << "'");
  }

  _dependent_host_name = dependent_host_name;
  _host_name = host_name;
  _dependency_type = dependency_type;
  _inherits_parent = inherits_parent;
  _fail_on_up = fail_on_up;
  _fail_on_down = fail_on_down;
  _fail_on_unreachable = fail_on_unreachable;
  _fail_on_pending = fail_on_pending;
  _dependency_period = dependency_period;

  // Add new items to the configuration state.
    // Add new items to the list.
//    obj->next = hostdependency_list;
    //  hostdependency_list = obj.get();

}

hostdependency::types hostdependency::get_dependency_type() const {
 return _dependency_type;
}

void hostdependency::set_dependency_type(types dependency_type) {
  _dependency_type = dependency_type;
}

std::string const& hostdependency::get_dependent_host_name() const {
  return _dependent_host_name;
}

void hostdependency::set_dependent_host_name(
  std::string const& dependent_host_name) {
  _dependent_host_name = dependent_host_name;
}

std::string const& hostdependency::get_host_name() const {
  return _host_name;
}

void hostdependency::set_host_name(std::string const& host_name) {
  _host_name = host_name;
}

std::string const& hostdependency::get_dependency_period() const {
  return _dependency_period;
}

void hostdependency::set_dependency_period(
  std::string const& dependency_period) {
  _dependency_period = dependency_period;
}
bool hostdependency::get_inherits_parent() const {
  return _inherits_parent;
}

void hostdependency::set_inherits_parent(bool inherits_parent) {
  _inherits_parent = inherits_parent;
}

bool hostdependency::get_fail_on_up() const {
  return _fail_on_up;
}

void hostdependency::set_fail_on_up(bool fail_on_up) {
  _fail_on_up = fail_on_up;
}

bool hostdependency::get_fail_on_down() const {
  return _fail_on_down;
}

void hostdependency::set_fail_on_down(bool fail_on_down) {
  _fail_on_down = fail_on_down;
}

bool hostdependency::get_fail_on_unreachable() const {
  return _fail_on_unreachable;
}

void hostdependency::set_fail_on_unreachable(bool fail_on_unreachable) {
  _fail_on_unreachable = fail_on_unreachable;
}

bool hostdependency::get_fail_on_pending() const {
  return _fail_on_pending;
}

void hostdependency::set_fail_on_pending(bool fail_on_pending) {
  _fail_on_pending = fail_on_pending;
}

bool hostdependency::get_circular_path_checked() const {
  return _circular_path_checked;
}

void hostdependency::set_circular_path_checked(bool circular_path_checked) {
  _circular_path_checked  = circular_path_checked;
}

bool hostdependency::get_contains_circular_path() const {
  return _contains_circular_path;
}

void hostdependency::set_contains_circular_path(bool contains_circular_path) {
  _contains_circular_path = contains_circular_path;
}

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
  return (obj1.get_dependency_type() == obj2.get_dependency_type()
          && obj1.get_dependent_host_name() == obj2.get_dependent_host_name()
          && obj1.get_host_name() == obj2.get_host_name()
          && obj1.get_dependency_period() == obj2.get_dependency_period()
          && obj1.get_inherits_parent() == obj2.get_inherits_parent()
          && obj1.get_fail_on_up() == obj2.get_fail_on_up()
          && obj1.get_fail_on_down() == obj2.get_fail_on_down()
          && obj1.get_fail_on_unreachable() == obj2.get_fail_on_unreachable()
          && obj1.get_fail_on_pending() == obj2.get_fail_on_pending()
          && obj1.get_circular_path_checked() == obj2.get_circular_path_checked()
          && obj1.get_contains_circular_path() == obj2.get_contains_circular_path());
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
  if (obj1.get_dependent_host_name() != obj2.get_dependent_host_name())
    return obj1.get_dependent_host_name() < obj2.get_dependent_host_name();
  else if(obj1.get_host_name() != obj2.get_host_name())
    return obj1.get_host_name() < obj2.get_host_name();
  else if (obj1.get_dependency_type() != obj2.get_dependency_type())
    return obj1.get_dependency_type() < obj2.get_dependency_type();
  else if (obj1.get_dependency_period() != obj2.get_dependency_period())
    return obj1.get_dependency_period() < obj2.get_dependency_period();
  else if (obj1.get_inherits_parent() != obj2.get_inherits_parent())
    return obj1.get_inherits_parent() < obj2.get_inherits_parent();
  else if (obj1.get_fail_on_up() != obj2.get_fail_on_up())
    return obj1.get_fail_on_up() < obj2.get_fail_on_up();
  else if (obj1.get_fail_on_down() != obj2.get_fail_on_down())
    return obj1.get_fail_on_down() < obj2.get_fail_on_down();
  else if (obj1.get_fail_on_unreachable() != obj2.get_fail_on_unreachable())
    return obj1.get_fail_on_unreachable() < obj2.get_fail_on_unreachable();
  else if (obj1.get_fail_on_pending() != obj2.get_fail_on_pending())
    return obj1.get_fail_on_pending() < obj2.get_fail_on_pending();
  else if (obj1.get_circular_path_checked() != obj2.get_circular_path_checked())
    return obj1.get_circular_path_checked() < obj2.get_circular_path_checked();
  return obj1.get_contains_circular_path() < obj2.get_contains_circular_path();
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
  char const* dependency_period_str(nullptr);
  if (obj.dependency_period_ptr)
    dependency_period_str = chkstr(obj.dependency_period_ptr->name);

  os << "hostdependency {\n"
    "  dependency_type:        " << obj.get_dependency_type() << "\n"
    "  dependent_host_name:    " << obj.get_dependent_host_name() << "\n"
    "  host_name:              " << obj.get_host_name() << "\n"
    "  dependency_period:      " << obj.get_dependency_period() << "\n"
    "  inherits_parent:        " << obj.get_inherits_parent() << "\n"
    "  fail_on_up:             " << obj.get_fail_on_up() << "\n"
    "  fail_on_down:           " << obj.get_fail_on_down() << "\n"
    "  fail_on_unreachable:    " << obj.get_fail_on_unreachable() << "\n"
    "  fail_on_pending:        " << obj.get_fail_on_pending() << "\n"
    "  circular_path_checked:  " << obj.get_circular_path_checked() << "\n"
    "  contains_circular_path: " << obj.get_contains_circular_path() << "\n"
    "  master_host_ptr:        " << (obj.master_host_ptr ?
                                      obj.master_host_ptr->get_name() :
                                      "\"NULL\"") << "\n"
    "  dependent_host_ptr:     " << (obj.dependent_host_ptr ?
                                      obj.dependent_host_ptr->get_name() :
                                      "\"NULL\"") << "\n"
    "  dependency_period_ptr:  " << chkstr(dependency_period_str) << "\n"
    "}\n";
  return os;
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
bool hostdependency::check_for_circular_hostdependency_path(
      hostdependency* dep,
      types dependency_type) {
  if (!dep)
    return false;

  // This is not the proper dependency type.
  if ((_dependency_type != dependency_type)
      || (dep->get_dependency_type() != dependency_type))
    return false;

  // Don't go into a loop, don't bother checking anymore if we know this
  // dependency already has a loop.
  if (_contains_circular_path)
    return true;

  // Dependency has already been checked - there is a path somewhere,
  // but it may not be for this particular dep... This should speed up
  // detection for some loops.
  if (dep->get_circular_path_checked())
    return false;

  // Set the check flag so we don't get into an infinite loop.
  dep->set_circular_path_checked(true);

  // Is this host dependent on the root host?
  if (dep != this) {
    if (dependent_host_ptr == dep->master_host_ptr) {
      _contains_circular_path = true;
      dep->set_contains_circular_path(true);
      return true;
    }
  }

  // Notification dependencies are ok at this point as long as they
  // don't inherit.
  if ((dependency_type == hostdependency::notification)
      && !dep->get_inherits_parent())
    return false;

  // Check all parent dependencies.
  for (hostdependency_mmap::iterator
         it(hostdependency::hostdependencies.begin()),
         end(hostdependency::hostdependencies.end());
       it != end;
       ++it) {
    // Only check parent dependencies.
    if (dep->master_host_ptr != it->second->dependent_host_ptr)
      continue;

    if (this->check_for_circular_hostdependency_path(it->second.get(),
                                                     dependency_type))
      return true;
  }

  return false;
}

