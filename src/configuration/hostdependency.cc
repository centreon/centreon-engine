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

#include "com/centreon/engine/configuration/hostdependency.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<hostdependency, type, &hostdependency::method>::generic

static struct {
  std::string const name;
  bool (*func)(hostdependency&, std::string const&);
} gl_setters[] = {
  { "hostgroup",                     setter(std::string const&, _set_hostgroups) },
  { "hostgroups",                    setter(std::string const&, _set_hostgroups) },
  { "hostgroup_name",                setter(std::string const&, _set_hostgroups) },
  { "host",                          setter(std::string const&, _set_hosts) },
  { "host_name",                     setter(std::string const&, _set_hosts) },
  { "master_host",                   setter(std::string const&, _set_hosts) },
  { "master_host_name",              setter(std::string const&, _set_hosts) },
  { "dependent_hostgroup",           setter(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroups",          setter(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroup_name",      setter(std::string const&, _set_dependent_hostgroups) },
  { "dependent_host",                setter(std::string const&, _set_dependent_hosts) },
  { "dependent_host_name",           setter(std::string const&, _set_dependent_hosts) },
  { "inherits_parent",               setter(bool, _set_inherits_parent) },
  { "notification_failure_options",  setter(std::string const&, _set_notification_failure_options) },
  { "notification_failure_criteria", setter(std::string const&, _set_notification_failure_options) },
  { "execution_failure_options",     setter(std::string const&, _set_execution_failure_options) },
  { "execution_failure_criteria",    setter(std::string const&, _set_execution_failure_options) }
};

/**
 *  Default constructor.
 */
hostdependency::hostdependency()
  : object("hostdependency") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostdependency to copy.
 */
hostdependency::hostdependency(hostdependency const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
hostdependency::~hostdependency() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostdependency to copy.
 *
 *  @return This hostdependency.
 */
hostdependency& hostdependency::operator=(hostdependency const& right) {
  if (this != &right) {
    object::operator=(right);
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostdependency to compare.
 *
 *  @return True if is the same hostdependency, otherwise false.
 */
bool hostdependency::operator==(hostdependency const& right) const throw () {
  return (object::operator==(right));
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostdependency to compare.
 *
 *  @return True if is not the same hostdependency, otherwise false.
 */
bool hostdependency::operator!=(hostdependency const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Parse and set the hostdependency property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool hostdependency::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (object::parse(key, value));
}

void hostdependency::_set_dependency_period(std::string const& value) {
  _dependency_period = value;
}

void hostdependency::_set_dependent_hostgroups(std::string const& value) {
  _dependent_hostgroups.clear();
  misc::split(value, _dependent_hostgroups, ',');
}

void hostdependency::_set_dependent_hosts(std::string const& value) {
  _dependent_hosts.clear();
  misc::split(value, _dependent_hosts, ',');
}

void hostdependency::_set_execution_failure_options(std::string const& value) {
  _execution_failure_options = 0; // XXX:
}

void hostdependency::_set_hostgroups(std::string const& value) {
  _hostgroups.clear();
  misc::split(value, _hostgroups, ',');
}

void hostdependency::_set_hosts(std::string const& value) {
  _hosts.clear();
  misc::split(value, _hosts, ',');
}

void hostdependency::_set_inherits_parent(bool value) {
  _inherits_parent = value;
}

void hostdependency::_set_notification_failure_options(std::string const& value) {
  _notification_failure_options = 0; // XXX:
}
