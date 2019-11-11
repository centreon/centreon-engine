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

#include <array>
#include "com/centreon/engine/hostdependency.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
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
 *  @param[in] dependent_hostname  Dependant host name.
 *  @param[in] hostname            Host name.
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
hostdependency::hostdependency(std::string const& dependent_hostname,
                               std::string const& hostname,
                               dependency::types dependency_type,
                               bool inherits_parent,
                               bool fail_on_up,
                               bool fail_on_down,
                               bool fail_on_unreachable,
                               bool fail_on_pending,
                               std::string const& dependency_period)
    : dependency(dependent_hostname,
                 hostname,
                 dependency_type,
                 inherits_parent,
                 fail_on_pending,
                 dependency_period),
      master_host_ptr{nullptr},
      dependent_host_ptr{nullptr},
      _fail_on_up{fail_on_up},
      _fail_on_down{fail_on_down},
      _fail_on_unreachable{fail_on_unreachable} {}

bool hostdependency::get_fail_on(int state) const {
  std::array<bool, 3> retval{_fail_on_up, _fail_on_down, _fail_on_unreachable};
  return retval[state];
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

/**
 *  Dump hostdependency content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The hostdependency to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, hostdependency const& obj) {
  std::string dependency_period_str;
  if (obj.dependency_period_ptr)
    dependency_period_str = obj.dependency_period_ptr->get_name();

  os << "hostdependency {\n"
        "  dependency_type:        "
     << obj.get_dependency_type()
     << "\n"
        "  dependent_hostname:    "
     << obj.get_dependent_hostname()
     << "\n"
        "  hostname:               "
     << obj.get_hostname()
     << "\n"
        "  dependency_period:      "
     << obj.get_dependency_period()
     << "\n"
        "  inherits_parent:        "
     << obj.get_inherits_parent()
     << "\n"
        "  fail_on_up:             "
     << obj.get_fail_on_up()
     << "\n"
        "  fail_on_down:           "
     << obj.get_fail_on_down()
     << "\n"
        "  fail_on_unreachable:    "
     << obj.get_fail_on_unreachable()
     << "\n"
        "  fail_on_pending:        "
     << obj.get_fail_on_pending()
     << "\n"
        "  circular_path_checked:  "
     << obj.get_circular_path_checked()
     << "\n"
        "  contains_circular_path: "
     << obj.get_contains_circular_path()
     << "\n"
        "  master_host_ptr:        "
     << (obj.master_host_ptr ? obj.master_host_ptr->get_name() : "\"NULL\"")
     << "\n"
        "  dependent_host_ptr:     "
     << (obj.dependent_host_ptr ? obj.dependent_host_ptr->get_name()
                                : "\"NULL\"")
     << "\n"
        "  dependency_period_ptr:  "
     << dependency_period_str
     << "\n"
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
  if (_dependency_type != dependency_type ||
      dep->get_dependency_type() != dependency_type)
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
  if (dependency_type == dependency::notification &&
      !dep->get_inherits_parent())
    return false;

  // Check all parent dependencies.
  for (hostdependency_mmap::iterator
           it(hostdependency::hostdependencies.begin()),
       end(hostdependency::hostdependencies.end());
       it != end; ++it) {
    // Only check parent dependencies.
    if (dep->master_host_ptr != it->second->dependent_host_ptr)
      continue;

    if (check_for_circular_hostdependency_path(it->second.get(),
                                               dependency_type))
      return true;
  }

  return false;
}

void hostdependency::resolve(int& w, int& e) {
  (void)w;
  int errors{0};

  // Find the dependent host.
  host_map::const_iterator it{host::hosts.find(_dependent_hostname)};
  if (it == host::hosts.end() || !it->second) {
    logger(log_verification_error, basic)
      << "Error: Dependent host specified in host dependency for "
         "host '" << _dependent_hostname
      << "' is not defined anywhere!";
    errors++;
    dependent_host_ptr = nullptr;
  }
  else
    dependent_host_ptr = it->second.get();

  // Find the host we're depending on.
  it = host::hosts.find(_hostname);
  if (it == host::hosts.end() || !it->second) {
    logger(log_verification_error, basic)
      << "Error: Host specified in host dependency for host '"
      << _dependent_hostname << "' is not defined anywhere!";
    errors++;
    master_host_ptr = nullptr;
  }
  else
    master_host_ptr = it->second.get();

  // Make sure they're not the same host.
  if (dependent_host_ptr == master_host_ptr && dependent_host_ptr != nullptr) {
    logger(log_verification_error, basic)
      << "Error: Host dependency definition for host '"
      << _dependent_hostname
      << "' is circular (it depends on itself)!";
    errors++;
  }

  // Find the timeperiod.
  if (!_dependency_period.empty()) {
    timeperiod_map::const_iterator
      it{timeperiod::timeperiods.find(_dependency_period)};

    if (it == timeperiod::timeperiods.end() || !it->second) {
      logger(log_verification_error, basic)
        << "Error: Dependency period '" << this->get_dependency_period()
        << "' specified in host dependency for host '"
        << _dependent_hostname
        << "' is not defined anywhere!";
      errors++;
      dependency_period_ptr = nullptr;
    }
    else
      dependency_period_ptr = it->second.get();
  }

  // Add errors.
  if (errors) {
    e += errors;
    throw engine_error() << "Cannot resolve host dependency";
  }
}


/**
 *  Find a service dependency from its key.
 *
 *  @param[in] k The service dependency configuration.
 *
 *  @return Iterator to the element if found,
 *          servicedependencies().end() otherwise.
 */
hostdependency_mmap::iterator hostdependency::hostdependencies_find(
  const com::centreon::engine::configuration::hostdependency &k) {
  typedef hostdependency_mmap collection;
  std::pair<collection::iterator, collection::iterator> p;

  p = hostdependencies.equal_range(*k.dependent_hosts().begin());
  while (p.first != p.second) {
    configuration::hostdependency current;
    current.configuration::object::operator=(k);
    current.dependent_hosts().insert(
      p.first->second->get_dependent_hostname());
    current.hosts().insert(p.first->second->get_hostname());
    current.dependency_period((!p.first->second->get_dependency_period().empty()
      ? p.first->second->get_dependency_period().c_str() : ""));
    current.inherits_parent(p.first->second->get_inherits_parent());
    unsigned int options(
      (p.first->second->get_fail_on_up()
        ? configuration::hostdependency::up : 0)
          | (p.first->second->get_fail_on_down()
            ? configuration::hostdependency::down : 0)
               | (p.first->second->get_fail_on_unreachable()
                 ? configuration::hostdependency::unreachable: 0)
                   | (p.first->second->get_fail_on_pending()
                     ? configuration::hostdependency::pending : 0));
    if (p.first->second->get_dependency_type() == engine::hostdependency::notification) {
      current.dependency_type(
        configuration::hostdependency::notification_dependency);
      current.notification_failure_options(options);
    } else {
      current.dependency_type(
        configuration::hostdependency::execution_dependency);
      current.execution_failure_options(options);
    }
    if (current == k)
      break ;
    ++p.first;
  }
  return (p.first == p.second) ? hostdependencies.end() : p.first;
}
