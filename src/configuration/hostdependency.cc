/*
** Copyright 2011-2013,2015 Merethis
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
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"

extern int config_warnings;
extern int config_errors;

using namespace com::centreon;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

#define SETTER(type, method) \
  &object::setter<hostdependency, type, &hostdependency::method>::generic

hostdependency::setters const hostdependency::_setters[] = {
  { "host",                          SETTER(std::string const&, _set_hosts) },
  { "host_name",                     SETTER(std::string const&, _set_hosts) },
  { "master_host",                   SETTER(std::string const&, _set_hosts) },
  { "master_host_name",              SETTER(std::string const&, _set_hosts) },
  { "dependent_host",                SETTER(std::string const&, _set_dependent_hosts) },
  { "dependent_host_name",           SETTER(std::string const&, _set_dependent_hosts) },
  { "inherits_parent",               SETTER(bool, _set_inherits_parent) },
  { "execution_failure_options",     SETTER(std::string const&, _set_failure_options) },
  { "execution_failure_criteria",    SETTER(std::string const&, _set_failure_options) },
  { "failure_options",               SETTER(std::string const&, _set_failure_options) },
  { "failure_criteria",              SETTER(std::string const&, _set_failure_options) },

  // Deprecated.
  { "dependent_hostgroup",           SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroups",          SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroup_name",      SETTER(std::string const&, _set_dependent_hostgroups) },
  { "hostgroup",                     SETTER(std::string const&, _set_hostgroups) },
  { "hostgroups",                    SETTER(std::string const&, _set_hostgroups) },
  { "hostgroup_name",                SETTER(std::string const&, _set_hostgroups) },
  { "notification_failure_options",  SETTER(std::string const&, _set_notification_failure_options) },
  { "notification_failure_criteria", SETTER(std::string const&, _set_notification_failure_options) }
};

// Default values.
static unsigned short const default_failure_options(hostdependency::none);
static bool const           default_inherits_parent(false);

/**
 *  Default constructor.
 */
hostdependency::hostdependency()
  : object(object::hostdependency),
    _failure_options(default_failure_options),
    _inherits_parent(default_inherits_parent) {}

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
hostdependency::~hostdependency() throw () {}

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
    _dependent_hosts = right._dependent_hosts;
    _failure_options = right._failure_options;
    _hosts = right._hosts;
    _inherits_parent = right._inherits_parent;
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
          && _dependent_hosts == right._dependent_hosts
          && _failure_options == right._failure_options
          && _hosts == right._hosts
          && _inherits_parent == right._inherits_parent);
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
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool hostdependency::operator<(hostdependency const& right) const {
  if (_dependent_hosts != right._dependent_hosts)
    return (_dependent_hosts < right._dependent_hosts);
  else if (_hosts != right._hosts)
    return (_hosts < right._hosts);
  else if (_dependency_period != right._dependency_period)
    return (_dependency_period < right._dependency_period);
  else if (_failure_options != right._failure_options)
    return (_failure_options < right._failure_options);
  return (_inherits_parent < right._inherits_parent);
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void hostdependency::check_validity() const {
  if (_hosts->empty())
    throw (engine_error() << "Host dependency is not attached to any "
           << "host (property 'host_name')");
  if (_dependent_hosts->empty())
    throw (engine_error() << "Host dependency is not attached to any "
           << "dependent host (property 'dependent_host_name')");

  if (!_failure_options) {
    ++config_warnings;
    std::string host_name(_hosts->front());
    std::string dependend_host_name(_dependent_hosts->front());
    logger(log_config_warning, basic)
      << "Warning: Ignoring lame host dependency of host '"
      << dependend_host_name << "' on host '"
      << host_name << "'.";
  }

  return ;
}

/**
 *  Get host dependency key.
 *
 *  @return This object.
 */
hostdependency::key_type const& hostdependency::key() const throw () {
  return (*this);
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void hostdependency::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "Cannot merge host dependency with '"
           << obj.type() << "'");
  hostdependency const& tmpl(static_cast<hostdependency const&>(obj));

  MRG_DEFAULT(_dependency_period);
  MRG_INHERIT(_dependent_hosts);
  MRG_OPTION(_failure_options);
  MRG_INHERIT(_hosts);
  MRG_OPTION(_inherits_parent);
}

/**
 *  Parse and set the hostdependency property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool hostdependency::parse(char const* key, char const* value) {
  for (unsigned int i(0);
       i < sizeof(_setters) / sizeof(_setters[0]);
       ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  return (false);
}

/**
 *  Set the dependency period.
 *
 *  @param[in] period New dependency period.
 */
void hostdependency::dependency_period(std::string const& period) {
  _dependency_period = period;
  return ;
}

/**
 *  Get dependency_period.
 *
 *  @return The dependency_period.
 */
std::string const& hostdependency::dependency_period() const throw () {
  return (_dependency_period);
}

/**
 *  Get dependent hosts.
 *
 *  @return The dependent hosts.
 */
list_string& hostdependency::dependent_hosts() throw () {
  return (*_dependent_hosts);
}

/**
 *  Get dependent_hosts.
 *
 *  @return The dependent_hosts.
 */
list_string const& hostdependency::dependent_hosts() const throw () {
  return (*_dependent_hosts);
}

/**
 *  Set the execution failure options.
 *
 *  @param[in] options New options.
 */
void hostdependency::failure_options(unsigned int options) throw () {
  _failure_options.set(options);
  return ;
}

/**
 *  Get execution_failure_options.
 *
 *  @return The execution_failure_options.
 */
unsigned int hostdependency::failure_options() const throw () {
  return (_failure_options);
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string& hostdependency::hosts() throw () {
  return (*_hosts);
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string const& hostdependency::hosts() const throw () {
  return (*_hosts);
}

/**
 *  Set parent inheritance.
 *
 *  @param[in] inherit True if dependency inherits parent.
 */
void hostdependency::inherits_parent(bool inherit) throw () {
  _inherits_parent = inherit;
  return ;
}

/**
 *  Get inherits_parent.
 *
 *  @return The inherits_parent.
 */
bool hostdependency::inherits_parent() const throw () {
  return (_inherits_parent);
}

/**
 *  Set dependency_period value.
 *
 *  @param[in] value The new dependency_period value.
 *
 *  @return True on success, otherwise false.
 */
bool hostdependency::_set_dependency_period(std::string const& value) {
  _dependency_period = value;
  return (true);
}

/**
 *  Deprecated variable.
 *
 *  @param[in] value  Unused.
 *
 *  @return True.
 */
bool hostdependency::_set_dependent_hostgroups(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
    << "Warning: host dependency hostgroups was ignored";
  ++config_warnings;
  return (true);
}

/**
 *  Set dependent_hosts value.
 *
 *  @param[in] value The new dependent_hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool hostdependency::_set_dependent_hosts(std::string const& value) {
  _dependent_hosts = value;
  return (true);
}

/**
 *  Set execution failure options.
 *
 *  @param[in] value The new failure options.
 *
 *  @return True on success, otherwise false.
 */
bool hostdependency::_set_failure_options(std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  string::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    string::trim(*it);
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
  _failure_options = options;
  return (true);
}

/**
 *  Deprecated variable.
 *
 *  @param[in] value  Unused.
 *
 *  @return True.
 */
bool hostdependency::_set_hostgroups(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
    << "Warning: host dependency hostgroups was ignored";
  ++config_warnings;
  return (true);
}

/**
 *  Set hosts value.
 *
 *  @param[in] value The new hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool hostdependency::_set_hosts(std::string const& value) {
  _hosts = value;
  return (true);
}

/**
 *  Set inherits_parent value.
 *
 *  @param[in] value The new inherits_parent value.
 *
 *  @return True on success, otherwise false.
 */
bool hostdependency::_set_inherits_parent(bool value) {
  _inherits_parent = value;
  return (true);
}

/**
 *  Deprecated variable.
 *
 *  @param[in] value  Unused.
 *
 *  @return True.
 */
bool hostdependency::_set_notification_failure_options(
                       std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
    << "Warning: host dependency notification_failure_options"
    << " variable was ignored";
  ++config_warnings;
  return (true);
}
