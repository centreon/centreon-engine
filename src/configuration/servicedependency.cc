/*
** Copyright 2011-2014 Merethis
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

std::unordered_map<std::string, servicedependency::setter_func> const servicedependency::_setters{
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
  : object(object::servicedependency),
    _dependency_type(unknown_type),
    _execution_failure_options(default_execution_failure_options),
    _inherits_parent(default_inherits_parent),
    _notification_failure_options(default_notification_failure_options) {}

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
    _dependency_type = right._dependency_type;
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
  return *this;
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
          && _dependency_type == right._dependency_type
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
  return !operator==(right);
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
    return _dependent_hosts < right._dependent_hosts;
  else if (_dependent_service_description
           != right._dependent_service_description)
    return (_dependent_service_description
            < right._dependent_service_description);
  else if (_hosts != right._hosts)
    return _hosts < right._hosts;
  else if (_service_description != right._service_description)
    return _service_description < right._service_description;
  else if (_dependent_hostgroups != right._dependent_hostgroups)
    return _dependent_hostgroups < right._dependent_hostgroups;
  else if (_hostgroups != right._hostgroups)
    return _hostgroups < right._hostgroups;
  else if (_dependent_servicegroups != right._dependent_servicegroups)
    return _dependent_servicegroups < right._dependent_servicegroups;
  else if (_servicegroups != right._servicegroups)
    return _servicegroups < right._servicegroups;
  else if (_dependency_type != right._dependency_type)
    return _dependency_type < right._dependency_type;
  else if (_dependency_period != right._dependency_period)
    return _dependency_period < right._dependency_period;
  else if (_execution_failure_options
           != right._execution_failure_options)
    return (_execution_failure_options
            < right._execution_failure_options);
  else if (_inherits_parent != right._inherits_parent)
    return _inherits_parent < right._inherits_parent;
  return (_notification_failure_options
          < right._notification_failure_options);
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void servicedependency::check_validity() const {
  // Check base service(s).
  if (_servicegroups->empty()) {
    if (_service_description->empty())
      throw (engine_error() << "Service dependency is not attached to "
             << "any service or service group (properties "
             << "'service_description' or 'servicegroup_name', "
             << "respectively)");
    else if (_hosts->empty() && _hostgroups->empty())
      throw (engine_error() << "Service dependency is not attached to "
             << "any host or host group (properties 'host_name' or "
             << "'hostgroup_name', respectively)");
  }

  // Check dependent service(s).
  if (_dependent_servicegroups->empty()) {
    if (_dependent_service_description->empty())
      throw (engine_error() << "Service dependency is not attached to "
             << "any dependent service or dependent service group "
             << "(properties 'dependent_service_description' or "
             << "'dependent_servicegroup_name', respectively)");
    else if (_dependent_hosts->empty()
             && _dependent_hostgroups->empty())
      throw (engine_error() << "Service dependency is not attached to "
             << "any dependent host or dependent host group (properties "
             << "'dependent_host_name' or 'dependent_hostgroup_name', "
             << "respectively)");
  }

  // With no execution or failure options this dependency is useless.
  if (!_execution_failure_options && !_notification_failure_options) {
    ++config_warnings;
    std::ostringstream msg;
    msg << "Warning: Ignoring lame service dependency of ";
    if (!_dependent_servicegroups->empty())
      msg << "service group '" << _dependent_servicegroups->front()
          << "'";
    else {
      msg << "service '" << _dependent_service_description->front() << "' of ";
      if (!_dependent_hostgroups->empty())
        msg << "host group '" << _dependent_hostgroups->front() << "'";
      else
        msg << "host '" << _dependent_hosts->front() << "'";
    }
    msg << " on ";
    if (!_servicegroups->empty())
      msg << "service group '" << _servicegroups->front() << "'";
    else {
      msg << "service '" << _service_description->front() << "' of ";
      if (!_hostgroups->empty())
        msg << "host group '" << _hostgroups->front() << "'";
      else
        msg << "host '" << _hosts->front() << "'";
    }
    logger(log_config_warning, basic) << msg.str();
  }

  return ;
}

/**
 *  Get service dependency key.
 *
 *  @return This object.
 */
servicedependency::key_type const& servicedependency::key() const throw () {
  return *this;
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
  MRG_INHERIT(_dependent_hostgroups);
  MRG_INHERIT(_dependent_hosts);
  MRG_INHERIT(_dependent_servicegroups);
  MRG_INHERIT(_dependent_service_description);
  MRG_OPTION(_execution_failure_options);
  MRG_OPTION(_inherits_parent);
  MRG_INHERIT(_hostgroups);
  MRG_INHERIT(_hosts);
  MRG_OPTION(_notification_failure_options);
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
bool servicedependency::parse(char const* key, char const* value) {
  std::unordered_map<std::string, servicedependency::setter_func>::const_iterator
    it{_setters.find(key)};
  if (it != _setters.end())
    return (it->second)(*this, value);
  return false;
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
  return _dependency_period;
}

/**
 *  Set the dependency type.
 *
 *  @param[in] type Dependency type.
 */
void servicedependency::dependency_type(
                          servicedependency::dependency_kind type) throw () {
  _dependency_type = type;
  return ;
}

/**
 *  Get the dependency type.
 *
 *  @return Dependency type.
 */
servicedependency::dependency_kind servicedependency::dependency_type() const throw () {
  return _dependency_type;
}

/**
 *  Get dependent host groups.
 *
 *  @return Dependent host groups.
 */
list_string& servicedependency::dependent_hostgroups() throw () {
  return *_dependent_hostgroups;
}

/**
 *  Get dependent_hostgroup.
 *
 *  @return The dependent_hostgroup.
 */
list_string const& servicedependency::dependent_hostgroups() const throw () {
  return *_dependent_hostgroups;
}

/**
 *  Get dependent hosts.
 *
 *  @return Dependent hosts.
 */
list_string& servicedependency::dependent_hosts() throw () {
  return *_dependent_hosts;
}

/**
 *  Get dependent_hosts.
 *
 *  @return The dependent_hosts.
 */
list_string const& servicedependency::dependent_hosts() const throw () {
  return *_dependent_hosts;
}

/**
 *  Get dependent service groups.
 *
 *  @return Dependent service groups.
 */
list_string& servicedependency::dependent_servicegroups() throw () {
  return *_dependent_servicegroups;
}

/**
 *  Get dependent_servicegroup.
 *
 *  @return The dependent_servicegroup.
 */
list_string const& servicedependency::dependent_servicegroups() const throw () {
  return *_dependent_servicegroups;
}

/**
 *  Get dependent service description.
 *
 *  @return Dependent service description.
 */
list_string& servicedependency::dependent_service_description() throw () {
  return *_dependent_service_description;
}

/**
 *  Get dependent_service_description.
 *
 *  @return The dependent_service_description.
 */
list_string const& servicedependency::dependent_service_description() const throw () {
  return *_dependent_service_description;
}

/**
 *  Set the execution failure options.
 *
 *  @param[in] options New execution failure options.
 */
void servicedependency::execution_failure_options(
                          unsigned int options) throw () {
  _execution_failure_options = options;
  return ;
}

/**
 *  Get execution_failure_options.
 *
 *  @return The execution_failure_options.
 */
unsigned int servicedependency::execution_failure_options() const throw () {
  return _execution_failure_options;
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
  return _inherits_parent;
}

/**
 *  Get host groups.
 *
 *  @return Host groups.
 */
list_string& servicedependency::hostgroups() throw () {
  return *_hostgroups;
}

/**
 *  Get hostgroup.
 *
 *  @return The hostgroup.
 */
list_string const& servicedependency::hostgroups() const throw () {
  return *_hostgroups;
}

/**
 *  Get hosts.
 *
 *  @return Hosts.
 */
list_string& servicedependency::hosts() throw () {
  return *_hosts;
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string const& servicedependency::hosts() const throw () {
  return *_hosts;
}

/**
 *  Set the notification failure options.
 *
 *  @param[in] options New notification failure options.
 */
void servicedependency::notification_failure_options(
                          unsigned int options) throw () {
  _notification_failure_options = options;
  return ;
}

/**
 *  Get notification_failure_options.
 *
 *  @return The notification_failure_options.
 */
unsigned int servicedependency::notification_failure_options() const throw () {
  return _notification_failure_options;
}

/**
 *  Get service groups.
 *
 *  @return Service groups.
 */
list_string& servicedependency::servicegroups() throw () {
  return *_servicegroups;
}

/**
 *  Get servicegroup.
 *
 *  @return The servicegroup.
 */
list_string const& servicedependency::servicegroups() const throw () {
  return *_servicegroups;
}

/**
 *  Get service description.
 *
 *  @return Service description.
 */
list_string& servicedependency::service_description() throw () {
  return *_service_description;
}

/**
 *  Get service_description.
 *
 *  @return The service_description.
 */
list_string const& servicedependency::service_description() const throw () {
  return *_service_description;
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
  return true;
}

/**
 *  Set dependent_hostgroups value.
 *
 *  @param[in] value The new dependent_hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_dependent_hostgroups(std::string const& value) {
  _dependent_hostgroups = value;
  return true;
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
  return true;
}

/**
 *  Set dependent_servicegroups value.
 *
 *  @param[in] value The new dependent_servicegroups value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_dependent_servicegroups(std::string const& value) {
  _dependent_servicegroups = value;
  return true;
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
  return true;
}

/**
 *  Set execution_failure_options value.
 *
 *  @param[in] value The new execution_failure_options value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_execution_failure_options(std::string const& value) {
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
      return false;
  }
  _execution_failure_options = options;
  return true;
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
  return true;
}

/**
 *  Set hostgroups value.
 *
 *  @param[in] value The new hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_hostgroups(std::string const& value) {
  _hostgroups = value;
  return true;
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
  return true;
}

/**
 *  Set notification_failure_options value.
 *
 *  @param[in] value The new notification_failure_options value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_notification_failure_options(std::string const& value) {
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
      return false;
  }
  _notification_failure_options = options;
  return true;
}

/**
 *  Set servicegroups value.
 *
 *  @param[in] value The new servicegroups value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_servicegroups(std::string const& value) {
  _servicegroups = value;
  return true;
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
  return true;
}
