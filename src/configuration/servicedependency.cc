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

#include "com/centreon/engine/configuration/servicedependency.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<servicedependency, type, &servicedependency::method>::generic

static struct {
  std::string const name;
  bool (*func)(servicedependency&, std::string const&);
} gl_setters[] = {
  { "servicegroup",                  setter(std::string const&, _set_servicegroups) },
  { "servicegroups",                 setter(std::string const&, _set_servicegroups) },
  { "servicegroup_name",             setter(std::string const&, _set_servicegroups) },
  { "hostgroup",                     setter(std::string const&, _set_hostgroups) },
  { "hostgroups",                    setter(std::string const&, _set_hostgroups) },
  { "hostgroup_name",                setter(std::string const&, _set_hostgroups) },
  { "host",                          setter(std::string const&, _set_hosts) },
  { "host_name",                     setter(std::string const&, _set_hosts) },
  { "master_host",                   setter(std::string const&, _set_hosts) },
  { "master_host_name",              setter(std::string const&, _set_hosts) },
  { "description",                   setter(std::string const&, _set_service_description) },
  { "service_description",           setter(std::string const&, _set_service_description) },
  { "master_description",            setter(std::string const&, _set_service_description) },
  { "master_service_description",    setter(std::string const&, _set_service_description) },
  { "dependent_servicegroup",        setter(std::string const&, _set_dependent_servicegroups) },
  { "dependent_servicegroups",       setter(std::string const&, _set_dependent_servicegroups) },
  { "dependent_servicegroup_name",   setter(std::string const&, _set_dependent_servicegroups) },
  { "dependent_hostgroup",           setter(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroups",          setter(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroup_name",      setter(std::string const&, _set_dependent_hostgroups) },
  { "dependent_host",                setter(std::string const&, _set_dependent_hosts) },
  { "dependent_host_name",           setter(std::string const&, _set_dependent_hosts) },
  { "dependent_description",         setter(std::string const&, _set_dependent_service_description) },
  { "dependent_service_description", setter(std::string const&, _set_dependent_service_description) },
  { "dependency_period",             setter(std::string const&, _set_dependency_period) },
  { "inherits_parent",               setter(bool, _set_inherits_parent) },
  { "execution_failure_options",     setter(std::string const&, _set_execution_failure_options) },
  { "execution_failure_criteria",    setter(std::string const&, _set_execution_failure_options) },
  { "notification_failure_options",  setter(std::string const&, _set_notification_failure_options) },
  { "notification_failure_criteria", setter(std::string const&, _set_notification_failure_options) }
};

/**
 *  Default constructor.
 */
servicedependency::servicedependency()
  : object("servicedependency") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The servicedependency to copy.
 */
servicedependency::servicedependency(servicedependency const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
servicedependency::~servicedependency() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The servicedependency to copy.
 *
 *  @return This servicedependency.
 */
servicedependency& servicedependency::operator=(servicedependency const& right) {
  if (this != &right) {
    object::operator=(right);
    _hosts = right._hosts;
    _service_description = right._service_description;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The servicedependency to compare.
 *
 *  @return True if is the same servicedependency, otherwise false.
 */
bool servicedependency::operator==(servicedependency const& right) const throw () {
  return (object::operator==(right)
          && _hosts == right._hosts
          && _service_description == right._service_description);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The servicedependency to compare.
 *
 *  @return True if is not the same servicedependency, otherwise false.
 */
bool servicedependency::operator!=(servicedependency const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Parse and set the servicedependency property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (object::parse(key, value));
}

void servicedependency::_set_dependency_period(std::string const& value) {
  _dependency_period = value;
}

void servicedependency::_set_dependent_hostgroups(std::string const& value) {
  _dependent_hostgroups.clear();
  misc::split(value, _dependent_hostgroups, ',');
}

void servicedependency::_set_dependent_hosts(std::string const& value) {
  _dependent_hosts.clear();
  misc::split(value, _dependent_hosts, ',');
}

void servicedependency::_set_dependent_servicegroups(std::string const& value) {
  _dependent_servicegroups.clear();
  misc::split(value, _dependent_servicegroups, ',');
}

void servicedependency::_set_dependent_service_description(std::string const& value) {
  _dependent_service_description = value;
}

void servicedependency::_set_execution_failure_options(std::string const& value) {
  _execution_failure_options = 0; // XXX:
}

void servicedependency::_set_inherits_parent(bool value) {
  _inherits_parent = value;
}

void servicedependency::_set_hostgroups(std::string const& value) {
  _hostgroups.clear();
  misc::split(value, _hostgroups, ',');
}

void servicedependency::_set_hosts(std::string const& value) {
  _hosts.clear();
  misc::split(value, _hosts, ',');
}

void servicedependency::_set_notification_failure_options(std::string const& value) {
  _notification_failure_options = 0; // XXX:
}

void servicedependency::_set_servicegroups(std::string const& value) {
  _servicegroups.clear();
  misc::split(value, _servicegroups, ',');
}

void servicedependency::_set_service_description(std::string const& value) {
  _service_description = value;
}
