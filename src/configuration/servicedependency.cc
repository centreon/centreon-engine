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

#define SETTER(type, method) \
  &object::setter<servicedependency, type, &servicedependency::method>::generic

static struct {
  std::string const name;
  bool (*func)(servicedependency&, std::string const&);
} gl_setters[] = {
  { "servicegroup",                  SETTER(std::string const&, _set_servicegroups) },
  { "servicegroups",                 SETTER(std::string const&, _set_servicegroups) },
  { "servicegroup_name",             SETTER(std::string const&, _set_servicegroups) },
  { "hostgroup",                     SETTER(std::string const&, _set_hostgroups) },
  { "hostgroups",                    SETTER(std::string const&, _set_hostgroups) },
  { "hostgroup_name",                SETTER(std::string const&, _set_hostgroups) },
  { "host",                          SETTER(std::string const&, _set_hosts) },
  { "host_name",                     SETTER(std::string const&, _set_hosts) },
  { "master_host",                   SETTER(std::string const&, _set_hosts) },
  { "master_host_name",              SETTER(std::string const&, _set_hosts) },
  { "description",                   SETTER(std::string const&, _set_service_description) },
  { "service_description",           SETTER(std::string const&, _set_service_description) },
  { "master_description",            SETTER(std::string const&, _set_service_description) },
  { "master_service_description",    SETTER(std::string const&, _set_service_description) },
  { "dependent_servicegroup",        SETTER(std::string const&, _set_dependent_servicegroups) },
  { "dependent_servicegroups",       SETTER(std::string const&, _set_dependent_servicegroups) },
  { "dependent_servicegroup_name",   SETTER(std::string const&, _set_dependent_servicegroups) },
  { "dependent_hostgroup",           SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroups",          SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroup_name",      SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_host",                SETTER(std::string const&, _set_dependent_hosts) },
  { "dependent_host_name",           SETTER(std::string const&, _set_dependent_hosts) },
  { "dependent_description",         SETTER(std::string const&, _set_dependent_service_description) },
  { "dependent_service_description", SETTER(std::string const&, _set_dependent_service_description) },
  { "dependency_period",             SETTER(std::string const&, _set_dependency_period) },
  { "inherits_parent",               SETTER(bool, _set_inherits_parent) },
  { "execution_failure_options",     SETTER(std::string const&, _set_execution_failure_options) },
  { "execution_failure_criteria",    SETTER(std::string const&, _set_execution_failure_options) },
  { "notification_failure_options",  SETTER(std::string const&, _set_notification_failure_options) },
  { "notification_failure_criteria", SETTER(std::string const&, _set_notification_failure_options) }
};

// Default values.
static unsigned short const default_execution_failure_options(servicedependency::none);
static bool const           default_inherits_parent(false);
static unsigned short const default_notification_failure_options(servicedependency::none);

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
    _dependency_period = right._dependency_period;
    _dependent_hostgroups = right._dependent_hostgroups;
    _dependent_hosts = right._dependent_hosts;
    _dependent_servicegroups = right._dependent_servicegroups;
    _dependent_service_description = right._dependent_service_description;
    _execution_failure_options = right._execution_failure_options;
    _inherits_parent = right._inherits_parent;
    _hostgroups = right._hostgroups;
    _hosts = right._hosts;
    _notification_failure_options = right._notification_failure_options;
    _servicegroups = right._servicegroups;
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
          && _dependency_period == right._dependency_period
          && _dependent_hostgroups == right._dependent_hostgroups
          && _dependent_hosts == right._dependent_hosts
          && _dependent_servicegroups == right._dependent_servicegroups
          && _dependent_service_description == right._dependent_service_description
          && _execution_failure_options == right._execution_failure_options
          && _inherits_parent == right._inherits_parent
          && _hostgroups == right._hostgroups
          && _hosts == right._hosts
          && _notification_failure_options == right._notification_failure_options
          && _servicegroups == right._servicegroups
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
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void servicedependency::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  servicedependency const& tmpl(static_cast<servicedependency const&>(obj));

  MRG_STRING(_dependency_period);
  MRG_INHERIT(_dependent_hostgroups);
  MRG_INHERIT(_dependent_hosts);
  MRG_INHERIT(_dependent_servicegroups);
  MRG_INHERIT(_dependent_service_description);
  MRG_DEFAULT(_execution_failure_options);
  MRG_DEFAULT(_inherits_parent);
  MRG_INHERIT(_hostgroups);
  MRG_INHERIT(_hosts);
  MRG_DEFAULT(_notification_failure_options);
  MRG_INHERIT(_servicegroups);
  MRG_INHERIT(_service_description);
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
  return (false);
}

bool servicedependency::_set_dependency_period(std::string const& value) {
  _dependency_period = value;
  return (true);
}

bool servicedependency::_set_dependent_hostgroups(std::string const& value) {
  _dependent_hostgroups.set(value);
  return (true);
}

bool servicedependency::_set_dependent_hosts(std::string const& value) {
  _dependent_hosts.set(value);
  return (true);
}

bool servicedependency::_set_dependent_servicegroups(std::string const& value) {
  _dependent_servicegroups.set(value);
  return (true);
}

bool servicedependency::_set_dependent_service_description(std::string const& value) {
  _dependent_service_description.set(value);
  return (true);
}

bool servicedependency::_set_execution_failure_options(std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  misc::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    misc::trim(*it);
    if (*it == "o" || *it == "ok")
      options |= ok;
    else if (*it == "d" || *it == "unknown")
      options |= unknown;
    else if (*it == "w" || *it == "warning")
      options |= warning;
    else if (*it == "w" || *it == "critical")
      options |= critical;
    else if (*it == "p" || *it == "pending")
      options |= pending;
    else if (*it == "n" || *it == "none")
      options = none;
    else if (*it == "a" || *it == "all")
      options = ok | unknown | warning | critical | pending;
    else
      return (false);
  }
  _execution_failure_options = options;
  return (true);
}

bool servicedependency::_set_inherits_parent(bool value) {
  _inherits_parent = value;
  return (true);
}

bool servicedependency::_set_hostgroups(std::string const& value) {
  _hostgroups.set(value);
  return (true);
}

bool servicedependency::_set_hosts(std::string const& value) {
  _hosts.set(value);
  return (true);
}

bool servicedependency::_set_notification_failure_options(std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  misc::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    misc::trim(*it);
    if (*it == "o" || *it == "ok")
      options |= ok;
    else if (*it == "d" || *it == "unknown")
      options |= unknown;
    else if (*it == "w" || *it == "warning")
      options |= warning;
    else if (*it == "w" || *it == "critical")
      options |= critical;
    else if (*it == "p" || *it == "pending")
      options |= pending;
    else if (*it == "n" || *it == "none")
      options = none;
    else if (*it == "a" || *it == "all")
      options = ok | unknown | warning | critical | pending;
    else
      return (false);
  }
  _notification_failure_options = options;
  return (true);
}

bool servicedependency::_set_servicegroups(std::string const& value) {
  _servicegroups.set(value);
  return (true);
}

bool servicedependency::_set_service_description(std::string const& value) {
  _service_description.set(value);
  return (true);
}
