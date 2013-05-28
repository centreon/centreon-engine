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

using namespace com::centreon::engine;

#define SETTER(type, method) \
  &configuration::object::setter< \
     configuration::hostdependency, \
     type, \
     &configuration::hostdependency::method>::generic

static struct {
  std::string const name;
  bool (*func)(configuration::hostdependency&, std::string const&);
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
static unsigned short const default_execution_failure_options(configuration::hostdependency::none);
static bool const           default_inherits_parent(false);
static unsigned short const default_notification_failure_options(configuration::hostdependency::none);

/**
 *  Default constructor.
 */
configuration::hostdependency::hostdependency()
  : object(object::hostdependency, "hostdependency") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostdependency to copy.
 */
configuration::hostdependency::hostdependency(
  hostdependency const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
configuration::hostdependency::~hostdependency() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostdependency to copy.
 *
 *  @return This hostdependency.
 */
configuration::hostdependency& configuration::hostdependency::operator=(
                                 hostdependency const& right) {
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
bool configuration::hostdependency::operator==(
       hostdependency const& right) const throw () {
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
bool configuration::hostdependency::operator!=(
       hostdependency const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t configuration::hostdependency::id() const throw () {
  return (_id);
}

/**
 *  Check if the object is valid.
 *
 *  @return True if is a valid object, otherwise false.
 */
bool configuration::hostdependency::is_valid() const throw () {
  return ((!_hosts.empty() || !_hostgroups.empty())
          && (!_dependent_hosts.empty() || !_dependent_hostgroups.empty())
          && !_dependency_period.empty());
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void configuration::hostdependency::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  hostdependency const& tmpl(static_cast<hostdependency const&>(obj));

  MRG_DEFAULT(_dependency_period);
  MRG_INHERIT(_dependent_hostgroups);
  MRG_INHERIT(_dependent_hosts);
  MRG_OPTION(_execution_failure_options);
  MRG_INHERIT(_hostgroups);
  MRG_INHERIT(_hosts);
  MRG_OPTION(_inherits_parent);
  MRG_OPTION(_notification_failure_options);
}

/**
 *  Parse and set the hostdependency property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::hostdependency::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (false);
}

/**
 *  Set dependency_period value.
 *
 *  @param[in] value The new dependency_period value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::hostdependency::_set_dependency_period(
       std::string const& value) {
  _dependency_period = value;
  return (true);
}

/**
 *  Set dependent_hostgroups value.
 *
 *  @param[in] value The new dependent_hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::hostdependency::_set_dependent_hostgroups(
       std::string const& value) {
  _dependent_hostgroups.set(value);
  return (true);
}

/**
 *  Set dependent_hosts value.
 *
 *  @param[in] value The new dependent_hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::hostdependency::_set_dependent_hosts(
       std::string const& value) {
  _dependent_hosts.set(value);
  _id = 0;
  _hash(_id, _dependent_hosts.get());
  return (true);
}

/**
 *  Set execution_failure_options value.
 *
 *  @param[in] value The new execution_failure_options value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::hostdependency::_set_execution_failure_options(
       std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  misc::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    misc::trim(*it);
    if (*it == "o" || *it == "up")
      options |= up;
    else if (*it == "d" || *it == "down")
      options |= down;
    else if (*it == "u" || *it == "unreachable")
      options |= unreachable;
    else if (*it == "p" || *it == "pending")
      options |= pending;
    else if (*it == "n" || *it == "none")
      options = none;
    else if (*it == "a" || *it == "all")
      options = up | down | unreachable | pending;
    else
      return (false);
  }
  _execution_failure_options = options;
  return (true);
}

/**
 *  Set hostgroups value.
 *
 *  @param[in] value The new hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::hostdependency::_set_hostgroups(
       std::string const& value) {
  _hostgroups.set(value);
  return (true);
}

/**
 *  Set hosts value.
 *
 *  @param[in] value The new hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::hostdependency::_set_hosts(
       std::string const& value) {
  _hosts.set(value);
  return (true);
}

/**
 *  Set inherits_parent value.
 *
 *  @param[in] value The new inherits_parent value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::hostdependency::_set_inherits_parent(bool value) {
  _inherits_parent = value;
  return (true);
}

/**
 *  Set notification_failure_options value.
 *
 *  @param[in] value The new notification_failure_options value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::hostdependency::_set_notification_failure_options(
       std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  misc::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    misc::trim(*it);
    if (*it == "o" || *it == "up")
      options |= up;
    else if (*it == "d" || *it == "down")
      options |= down;
    else if (*it == "u" || *it == "unreachable")
      options |= unreachable;
    else if (*it == "p" || *it == "pending")
      options |= pending;
    else if (*it == "n" || *it == "none")
      options = none;
    else if (*it == "a" || *it == "all")
      options = up | down | unreachable | pending;
    else
      return (false);
  }
  _notification_failure_options = options;
  return (true);
}
