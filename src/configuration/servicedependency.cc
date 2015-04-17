/*
** Copyright 2011-2015 Merethis
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
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"

extern int config_warnings;
extern int config_errors;

using namespace com::centreon;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

#define SETTER(type, method) \
  &object::setter<servicedependency, type, &servicedependency::method>::generic

servicedependency::setters const servicedependency::_setters[] = {
  { "host",                          SETTER(std::string const&, _set_hosts) },
  { "host_name",                     SETTER(std::string const&, _set_hosts) },
  { "master_host",                   SETTER(std::string const&, _set_hosts) },
  { "master_host_name",              SETTER(std::string const&, _set_hosts) },
  { "description",                   SETTER(std::string const&, _set_service_description) },
  { "service_description",           SETTER(std::string const&, _set_service_description) },
  { "master_description",            SETTER(std::string const&, _set_service_description) },
  { "master_service_description",    SETTER(std::string const&, _set_service_description) },
  { "dependent_host",                SETTER(std::string const&, _set_dependent_hosts) },
  { "dependent_host_name",           SETTER(std::string const&, _set_dependent_hosts) },
  { "dependent_description",         SETTER(std::string const&, _set_dependent_service_description) },
  { "dependent_service_description", SETTER(std::string const&, _set_dependent_service_description) },
  { "dependency_period",             SETTER(std::string const&, _set_dependency_period) },
  { "inherits_parent",               SETTER(bool, _set_inherits_parent) },
  { "execution_failure_options",     SETTER(std::string const&, _set_failure_options) },
  { "execution_failure_criteria",    SETTER(std::string const&, _set_failure_options) },
  { "failure_options",               SETTER(std::string const&, _set_failure_options) },
  { "failure_criteria",              SETTER(std::string const&, _set_failure_options) },

  // Deprecated.
  { "dependent_servicegroup",        SETTER(std::string const&, _set_dependent_servicegroups) },
  { "dependent_servicegroups",       SETTER(std::string const&, _set_dependent_servicegroups) },
  { "dependent_servicegroup_name",   SETTER(std::string const&, _set_dependent_servicegroups) },
  { "dependent_hostgroup",           SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroups",          SETTER(std::string const&, _set_dependent_hostgroups) },
  { "dependent_hostgroup_name",      SETTER(std::string const&, _set_dependent_hostgroups) },
  { "hostgroup",                     SETTER(std::string const&, _set_hostgroups) },
  { "hostgroups",                    SETTER(std::string const&, _set_hostgroups) },
  { "hostgroup_name",                SETTER(std::string const&, _set_hostgroups) },
  { "notification_failure_options",  SETTER(std::string const&, _set_notification_failure_options) },
  { "notification_failure_criteria", SETTER(std::string const&, _set_notification_failure_options) },
  { "servicegroup",                  SETTER(std::string const&, _set_servicegroups) },
  { "servicegroups",                 SETTER(std::string const&, _set_servicegroups) },
  { "servicegroup_name",             SETTER(std::string const&, _set_servicegroups) }
};

// Default values.
static unsigned short const default_failure_options(servicedependency::none);
static bool const           default_inherits_parent(false);

/**
 *  Default constructor.
 */
servicedependency::servicedependency()
  : object(object::servicedependency),
    _failure_options(default_failure_options),
    _inherits_parent(default_inherits_parent) {}

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
servicedependency::~servicedependency() throw () {}

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
    _dependent_hosts = right._dependent_hosts;
    _dependent_service_description = right._dependent_service_description;
    _failure_options = right._failure_options;
    _inherits_parent = right._inherits_parent;
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
          && _dependency_period == right._dependency_period
          && _dependent_hosts == right._dependent_hosts
          && _dependent_service_description == right._dependent_service_description
          && _failure_options == right._failure_options
          && _inherits_parent == right._inherits_parent
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
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool servicedependency::operator<(servicedependency const& right) const {
  if (_dependent_hosts != right._dependent_hosts)
    return (_dependent_hosts < right._dependent_hosts);
  else if (_dependent_service_description
           != right._dependent_service_description)
    return (_dependent_service_description
            < right._dependent_service_description);
  else if (_hosts != right._hosts)
    return (_hosts < right._hosts);
  else if (_service_description != right._service_description)
    return (_service_description < right._service_description);
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
void servicedependency::check_validity() const {
  // Check base service(s).
  if (_service_description->empty())
    throw (engine_error() << "Service dependency is not attached to "
           << "any service (property 'service_description')");
  else if (_hosts->empty())
    throw (engine_error() << "Service dependency is not attached to "
           << "any host (property 'host_name')");
  // Check dependent service(s).
  if (_dependent_service_description->empty())
    throw (engine_error() << "Service dependency is not attached to "
           << "any dependent service (property "
           << "'dependent_service_description')");
  else if (_dependent_hosts->empty())
    throw (engine_error() << "Service dependency is not attached to "
           << "any dependent host (property 'dependent_host_name')");

  // With no execution or failure options this dependency is useless.
  if (!_failure_options) {
    ++config_warnings;
    logger(log_config_warning, basic)
      << "Warning: Ignoring lame service dependency of service '"
      << _dependent_service_description->front() << "' of "
      << "host '" << _dependent_hosts->front() << "' on service '"
      << _service_description->front() << "' of host '"
      << _hosts->front() << "'";
  }

  return ;
}

/**
 *  Get service dependency key.
 *
 *  @return This object.
 */
servicedependency::key_type const& servicedependency::key() const throw () {
  return (*this);
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void servicedependency::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "Cannot merge service dependency with '"
           << obj.type() << "'");
  servicedependency const& tmpl(static_cast<servicedependency const&>(obj));

  MRG_DEFAULT(_dependency_period);
  MRG_INHERIT(_dependent_hosts);
  MRG_INHERIT(_dependent_service_description);
  MRG_OPTION(_failure_options);
  MRG_OPTION(_inherits_parent);
  MRG_INHERIT(_hosts);
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
bool servicedependency::parse(char const* key, char const* value) {
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
 *  @param[in] period Dependency period.
 */
void servicedependency::dependency_period(std::string const& period) {
  _dependency_period = period;
  return ;
}

/**
 *  Get dependency_period.
 *
 *  @return The dependency_period.
 */
std::string const& servicedependency::dependency_period() const throw () {
  return (_dependency_period);
}

/**
 *  Get dependent hosts.
 *
 *  @return Dependent hosts.
 */
list_string& servicedependency::dependent_hosts() throw () {
  return (*_dependent_hosts);
}

/**
 *  Get dependent_hosts.
 *
 *  @return The dependent_hosts.
 */
list_string const& servicedependency::dependent_hosts() const throw () {
  return (*_dependent_hosts);
}

/**
 *  Get dependent service description.
 *
 *  @return Dependent service description.
 */
list_string& servicedependency::dependent_service_description() throw () {
  return (*_dependent_service_description);
}

/**
 *  Get dependent_service_description.
 *
 *  @return The dependent_service_description.
 */
list_string const& servicedependency::dependent_service_description() const throw () {
  return (*_dependent_service_description);
}

/**
 *  Set the execution failure options.
 *
 *  @param[in] options New execution failure options.
 */
void servicedependency::failure_options(unsigned int options) throw () {
  _failure_options = options;
  return ;
}

/**
 *  Get execution failure options.
 *
 *  @return The execution failure options.
 */
unsigned int servicedependency::failure_options() const throw () {
  return (_failure_options);
}

/**
 *  Set parent inheritance.
 *
 *  @param[in] inherit Parent inheritance.
 */
void servicedependency::inherits_parent(bool inherit) throw () {
  _inherits_parent = inherit;
  return ;
}

/**
 *  Get inherits_parent.
 *
 *  @return The inherits_parent.
 */
bool servicedependency::inherits_parent() const throw () {
  return (_inherits_parent);
}

/**
 *  Get hosts.
 *
 *  @return Hosts.
 */
list_string& servicedependency::hosts() throw () {
  return (*_hosts);
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string const& servicedependency::hosts() const throw () {
  return (*_hosts);
}

/**
 *  Get service description.
 *
 *  @return Service description.
 */
list_string& servicedependency::service_description() throw () {
  return (*_service_description);
}

/**
 *  Get service_description.
 *
 *  @return The service_description.
 */
list_string const& servicedependency::service_description() const throw () {
  return (*_service_description);
}


/**
 *  Set dependency_period value.
 *
 *  @param[in] value The new dependency_period value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_dependency_period(std::string const& value) {
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
bool servicedependency::_set_dependent_hostgroups(
                          std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: service dependency "
    << "dependent_hostgroups variable was ignored";
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
bool servicedependency::_set_dependent_hosts(std::string const& value) {
  _dependent_hosts = value;
  return (true);
}

/**
 *  Deprecated variable.
 *
 *  @param[in] value  Unused.
 *
 *  @return True.
 */
bool servicedependency::_set_dependent_servicegroups(
                          std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: service dependency "
    << "dependent_servicegroups variable was ignored";
  ++config_warnings;
  return (true);
}

/**
 *  Set dependent_service_description value.
 *
 *  @param[in] value The new dependent_service_description value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_dependent_service_description(std::string const& value) {
  _dependent_service_description = value;
  return (true);
}

/**
 *  Set failure_options value.
 *
 *  @param[in] value The new failure_options value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_failure_options(std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  string::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    string::trim(*it);
    if (*it == "o" || *it == "ok")
      options |= ok;
    else if (*it == "u" || *it == "unknown")
      options |= unknown;
    else if (*it == "w" || *it == "warning")
      options |= warning;
    else if (*it == "c" || *it == "critical")
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
  _failure_options = options;
  return (true);
}

/**
 *  Set inherits_parent value.
 *
 *  @param[in] value The new inherits_parent value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_inherits_parent(bool value) {
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
bool servicedependency::_set_hostgroups(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
    << "Warning: service dependency hostgroups variable was ignored";
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
bool servicedependency::_set_hosts(std::string const& value) {
  _hosts = value;
  return (true);
}

/**
 *  Deprecated variable.
 *
 *  @param[in] value  Unused.
 *
 *  @return True.
 */
bool servicedependency::_set_notification_failure_options(
                          std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
    << "Warning: service dependency notification_failure_options"
    << " variable was ignored";
  ++config_warnings;
  return (true);
}

/**
 *  Deprecated variable.
 *
 *  @param[in] value  Unused.
 *
 *  @return True.
 */
bool servicedependency::_set_servicegroups(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
    << "Warning: service dependency servicegroups variable was ignored";
  ++config_warnings;
  return (true);
}

/**
 *  Set service_description value.
 *
 *  @param[in] value The new service_description value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_service_description(std::string const& value) {
  _service_description = value;
  return (true);
}
