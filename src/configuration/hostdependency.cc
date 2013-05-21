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

using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<hostdependency, type, &hostdependency::method>::generic

static struct {
  std::string const name;
  bool (*func)(hostdependency&, std::string const&);
} gl_setters[] = {
  { "hostgroup",                     SETTER(std::string const&, _set_hostgroups) },
  { "hostgroups",                    SETTER(std::string const&, _set_hostgroups) },
  { "hostgroup_name",                SETTER(std::string const&, _set_hostgroups) },
  { "host",                          SETTER(std::string const&, _set_hosts) },
  { "host_name",                     SETTER(std::string const&, _set_hosts) },
  { "master_host",                   SETTER(std::string const&, _set_hosts) },
  { "master_host_name",              SETTER(std::string const&, _set_hosts) },
  { "dependent_hostgroup",           SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroups",          SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroup_name",      SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_host",                SETTER(std::string const&, _set_dependent_hosts) },
  { "dependent_host_name",           SETTER(std::string const&, _set_dependent_hosts) },
  { "inherits_parent",               SETTER(bool, _set_inherits_parent) },
  { "notification_failure_options",  SETTER(std::string const&, _set_notification_failure_options) },
  { "notification_failure_criteria", SETTER(std::string const&, _set_notification_failure_options) },
  { "execution_failure_options",     SETTER(std::string const&, _set_execution_failure_options) },
  { "execution_failure_criteria",    SETTER(std::string const&, _set_execution_failure_options) }
};

// Default values.
static unsigned int const default_execution_failure_options(hostdependency::none);
static bool const         default_inherits_parent(false);
static unsigned int const default_notification_failure_options(hostdependency::none);

/**
 *  Default constructor.
 */
hostdependency::hostdependency()
  : object("hostdependency"),
    _execution_failure_options(default_execution_failure_options),
    _inherits_parent(default_inherits_parent),
    _notification_failure_options(default_notification_failure_options) {

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
    _dependency_period = right._dependency_period;
    _dependent_hostgroups = right._dependent_hostgroups;
    _dependent_hosts = right._dependent_hosts;
    _execution_failure_options = right._execution_failure_options;
    _hostgroups = right._hostgroups;
    _hosts = right._hosts;
    _inherits_parent = right._inherits_parent;
    _notification_failure_options = right._notification_failure_options;
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
  return (object::operator==(right)
          && _dependency_period == right._dependency_period
          && _dependent_hostgroups == right._dependent_hostgroups
          && _dependent_hosts == right._dependent_hosts
          && _execution_failure_options == right._execution_failure_options
          && _hostgroups == right._hostgroups
          && _hosts == right._hosts
          && _inherits_parent == right._inherits_parent
          && _notification_failure_options == right._notification_failure_options);
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
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void hostdependency::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  hostdependency const& tmpl(static_cast<hostdependency const&>(obj));

  MRG_STRING(_dependency_period);
  MRG_INHERIT(_dependent_hostgroups);
  MRG_INHERIT(_dependent_hosts);
  MRG_DEFAULT(_execution_failure_options);
  MRG_INHERIT(_hostgroups);
  MRG_INHERIT(_hosts);
  MRG_DEFAULT(_inherits_parent);
  MRG_DEFAULT(_notification_failure_options);
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
  return (false);
}

void hostdependency::_set_dependency_period(std::string const& value) {
  _dependency_period = value;
}

void hostdependency::_set_dependent_hostgroups(std::string const& value) {
  _dependent_hostgroups.set(value);
}

void hostdependency::_set_dependent_hosts(std::string const& value) {
  _dependent_hosts.set(value);
}

void hostdependency::_set_execution_failure_options(std::string const& value) {
  _execution_failure_options = 0; // XXX:
}

void hostdependency::_set_hostgroups(std::string const& value) {
  _hostgroups.set(value);
}

void hostdependency::_set_hosts(std::string const& value) {
  _hosts.set(value);
}

void hostdependency::_set_inherits_parent(bool value) {
  _inherits_parent = value;
}

void hostdependency::_set_notification_failure_options(std::string const& value) {
  _notification_failure_options = 0; // XXX:
}
