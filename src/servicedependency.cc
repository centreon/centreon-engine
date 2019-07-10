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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/servicedependency.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

servicedependency_mmap servicedependency::servicedependencies;

/**
 *  Create a service dependency definition.
 *
 *  @param[in] dependent_hostname            Dependent host name.
 *  @param[in] dependent_service_description Dependent service
 *                                           description.
 *  @param[in] hostname                      Host name.
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
 */
servicedependency::servicedependency(std::string const& dependent_hostname,
                                     std::string const& dependent_svc_desc,
                                     std::string const& hostname,
                                     std::string const& service_description,
                                     dependency::types dependency_type,
                                     bool inherits_parent,
                                     bool fail_on_ok,
                                     bool fail_on_warning,
                                     bool fail_on_unknown,
                                     bool fail_on_critical,
                                     bool fail_on_pending,
                                     std::string const& dependency_period)
    : dependency{dependent_hostname, hostname,        dependency_type,
                 inherits_parent,    fail_on_pending, dependency_period},
      _dependent_service_description{dependent_svc_desc},
      _service_description{service_description},
      _fail_on_ok{fail_on_ok},
      _fail_on_warning{fail_on_warning},
      _fail_on_unknown{fail_on_unknown},
      _fail_on_critical{fail_on_critical} {}

std::string const& servicedependency::get_dependent_service_description()
    const {
  return _dependent_service_description;
}

void servicedependency::set_dependent_service_description(
    std::string const& dependent_service_desciption) {
  _dependent_service_description = dependent_service_desciption;
}

std::string const& servicedependency::get_service_description() const {
  return _service_description;
}

void servicedependency::set_service_description(
    std::string const& service_description) {
  _service_description = service_description;
}

bool servicedependency::get_fail_on_ok() const {
  return _fail_on_ok;
}

void servicedependency::set_fail_on_ok(bool fail_on_ok) {
  _fail_on_ok = fail_on_ok;
}

bool servicedependency::get_fail_on_warning() const {
  return _fail_on_warning;
}

void servicedependency::set_fail_on_warning(bool fail_on_warning) {
  _fail_on_warning = fail_on_warning;
}

bool servicedependency::get_fail_on_unknown() const {
  return _fail_on_unknown;
}

void servicedependency::set_fail_on_unknown(bool fail_on_unknown) {
  _fail_on_unknown = fail_on_unknown;
}

bool servicedependency::get_fail_on_critical() const {
  return _fail_on_critical;
}

void servicedependency::set_fail_on_critical(bool fail_on_critical) {
  _fail_on_critical = fail_on_critical;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool servicedependency::operator==(servicedependency const& obj) throw() {
  return static_cast<dependency>(*this) == obj &&
         _dependent_service_description ==
             obj.get_dependent_service_description() &&
         _service_description == obj.get_service_description() &&
         _fail_on_ok == obj.get_fail_on_ok() &&
         _fail_on_warning == obj.get_fail_on_warning() &&
         _fail_on_unknown == obj.get_fail_on_unknown() &&
         _fail_on_critical == obj.get_fail_on_critical();
}

/**
 *  Less-than operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if the first object is strictly less than the second.
 */
bool servicedependency::operator<(servicedependency const& obj) throw() {
  if (static_cast<dependency>(*this) != obj)
    return static_cast<dependency>(*this) < obj;
  else if (_dependent_service_description !=
           obj.get_dependent_service_description())
    return _dependent_service_description <
           obj.get_dependent_service_description();
  else if (_service_description != obj.get_service_description())
    return _service_description < obj.get_service_description();
  else if (_fail_on_ok != obj.get_fail_on_ok())
    return _fail_on_ok < obj.get_fail_on_ok();
  else if (_fail_on_warning != obj.get_fail_on_warning())
    return _fail_on_warning < obj.get_fail_on_warning();
  else if (_fail_on_critical != obj.get_fail_on_critical())
    return _fail_on_critical < obj.get_fail_on_critical();
  return _fail_on_unknown < obj.get_fail_on_unknown();
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
  std::string dependency_period_str;
  if (obj.dependency_period_ptr)
    dependency_period_str = obj.dependency_period_ptr->get_name();
  std::string dependent_svc_str("\"NULL\"");
  if (obj.dependent_service_ptr) {
    dependent_svc_str = obj.dependent_service_ptr->get_hostname();
    dependent_svc_str += ", ";
    dependent_svc_str += obj.dependent_service_ptr->get_description();
  }
  std::string master_svc_str("\"NULL\"");
  if (obj.master_service_ptr) {
    master_svc_str = obj.master_service_ptr->get_hostname();
    master_svc_str += ", ";
    master_svc_str += obj.master_service_ptr->get_description();
  }

  os << "servicedependency {\n"
        "  dependency_type:               "
     << obj.get_dependency_type()
     << "\n"
        "  dependent_hostname:            "
     << obj.get_dependent_hostname()
     << "\n"
        "  dependent_service_description: "
     << obj.get_dependent_service_description()
     << "\n"
        "  hostname:                      "
     << obj.get_hostname()
     << "\n"
        "  service_description:           "
     << obj.get_service_description()
     << "\n"
        "  dependency_period:             "
     << obj.get_dependency_period()
     << "\n"
        "  inherits_parent:               "
     << obj.get_inherits_parent()
     << "\n"
        "  fail_on_ok:                    "
     << obj.get_fail_on_ok()
     << "\n"
        "  fail_on_warning:               "
     << obj.get_fail_on_warning()
     << "\n"
        "  fail_on_unknown:               "
     << obj.get_fail_on_unknown()
     << "\n"
        "  fail_on_critical:              "
     << obj.get_fail_on_critical()
     << "\n"
        "  fail_on_pending:               "
     << obj.get_fail_on_pending()
     << "\n"
        "  circular_path_checked:         "
     << obj.get_circular_path_checked()
     << "\n"
        "  contains_circular_path:        "
     << obj.get_contains_circular_path()
     << "\n"
        "  master_service_ptr:            "
     << master_svc_str
     << "\n"
        "  dependent_service_ptr:         "
     << dependent_svc_str
     << "\n"
        "  dependency_period_ptr:         "
     << dependency_period_str
     << "\n"
        "}\n";
  return os;
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
bool servicedependency::check_for_circular_servicedependency_path(
    servicedependency* dep,
    types dependency_type) {
  // This is not the proper dependency type.
  if ((_dependency_type != dependency_type) ||
      (dep->get_dependency_type() != dependency_type))
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

  // Is this service dependent on the root service?
  // Is this host dependent on the root host?
  if (dep != this) {
    if (master_service_ptr == dep->master_service_ptr) {
      _contains_circular_path = true;
      dep->set_contains_circular_path(true);
      return true;
    }
  }

  // Notification dependencies are ok at this point as long as they
  // don't inherit.
  if ((dependency_type == dependency::notification) &&
      (!dep->get_inherits_parent()))
    return false;

  // Check all parent dependencies.
  for (servicedependency_mmap::iterator
           it(servicedependency::servicedependencies.begin()),
       end(servicedependency::servicedependencies.end());
       it != end; ++it) {
    // Only check parent dependencies.
    if (dep->master_service_ptr != it->second->dependent_service_ptr)
      continue;

    if (this->check_for_circular_servicedependency_path(it->second.get(),
                                                        dependency_type))
      return true;
  }

  return false;
}

void servicedependency::resolve(int& w, int& e) {
  (void)w;
  int errors{0};

  // Find the dependent service.
  service_map::const_iterator found{service::services.find(
    {get_dependent_hostname(), get_dependent_service_description()})};

  if (found == service::services.end() || !found->second) {
    logger(log_verification_error, basic)
      << "Error: Dependent service '"
      << get_dependent_service_description() << "' on host '"
      << get_dependent_hostname()
      << "' specified in service dependency for service '"
      << get_service_description() << "' on host '"
      << get_hostname() << "' is not defined anywhere!";
    errors++;
  }
  else
    dependent_service_ptr = found->second.get();

  // Save pointer for later.
  found = service::services.find({get_hostname(), get_service_description()});

  // Find the service we're depending on.
  if (found == service::services.end() || !found->second) {
    logger(log_verification_error, basic)
      << "Error: Service '" << get_service_description() << "' on host '"
      << get_hostname()
      << "' specified in service dependency for service '"
      << get_dependent_service_description() << "' on host '"
      << get_dependent_hostname() << "' is not defined anywhere!";
    errors++;
  }
  else
    // Save pointer for later.
    master_service_ptr = found->second.get();

  // Make sure they're not the same service.
  if (dependent_service_ptr == master_service_ptr) {
    logger(log_verification_error, basic)
      << "Error: Service dependency definition for service '"
      << get_dependent_service_description() << "' on host '"
      << get_dependent_hostname()
      << "' is circular (it depends on itself)!";
    errors++;
  }

  // Find the timeperiod.
  if (!get_dependency_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it{timeperiod::timeperiods.find(get_dependency_period())};

    if (it == timeperiod::timeperiods.end() || !it->second) {
      logger(log_verification_error, basic)
        << "Error: Dependency period '" << get_dependency_period()
        << "' specified in service dependency for service '"
        << get_dependent_service_description() << "' on host '"
        << get_dependent_hostname() << "' is not defined anywhere!";
      errors++;
    }
    else
      // Save the timeperiod pointer for later.
      dependency_period_ptr = temp_timeperiod;
  }

  // Add errors.
  if (errors) {
    e += errors;
    throw engine_error() << "Cannot resolve service dependency";
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
servicedependency_mmap::iterator servicedependency::servicedependencies_find(
    configuration::servicedependency const& k) {
  typedef servicedependency_mmap collection;
  std::pair<collection::iterator, collection::iterator> p;
  p = servicedependencies.equal_range(std::make_pair(
      k.dependent_hosts().front(), k.dependent_service_description().front()));
  while (p.first != p.second) {
    configuration::servicedependency current;
    current.configuration::object::operator=(k);
    current.dependent_hosts().push_back(
        p.first->second->get_dependent_hostname());
    current.dependent_service_description().push_back(
        p.first->second->get_dependent_service_description());
    current.hosts().push_back(p.first->second->get_hostname());
    current.service_description().push_back(
        p.first->second->get_service_description());
    current.dependency_period(p.first->second->get_dependency_period());
    current.inherits_parent(p.first->second->get_inherits_parent());
    unsigned int options((p.first->second->get_fail_on_ok()
                              ? configuration::servicedependency::ok
                              : 0) |
                         (p.first->second->get_fail_on_warning()
                              ? configuration::servicedependency::warning
                              : 0) |
                         (p.first->second->get_fail_on_unknown()
                              ? configuration::servicedependency::unknown
                              : 0) |
                         (p.first->second->get_fail_on_critical()
                              ? configuration::servicedependency::critical
                              : 0) |
                         (p.first->second->get_fail_on_pending()
                              ? configuration::servicedependency::pending
                              : 0));
    if (p.first->second->get_dependency_type() ==
        engine::dependency::notification) {
      current.dependency_type(
          configuration::servicedependency::notification_dependency);
      current.notification_failure_options(options);
    } else {
      current.dependency_type(
          configuration::servicedependency::execution_dependency);
      current.execution_failure_options(options);
    }
    if (current == k)
      break;
    ++p.first;
  }
  return (p.first == p.second) ? servicedependencies.end() : p.first;
}

