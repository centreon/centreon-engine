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
#include "com/centreon/hash.hh"

using namespace com::centreon;
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
  : object(object::servicedependency),
    _execution_failure_options(default_execution_failure_options),
    _inherits_parent(default_inherits_parent),
    _notification_failure_options(default_notification_failure_options) {

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
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t servicedependency::id() const throw () {
  if (!_id) {
    hash_combine(
      _id,
      _dependent_hostgroups.get().begin(),
      _dependent_hostgroups.get().end());
    hash_combine(
      _id,
      _dependent_hosts.get().begin(),
      _dependent_hosts.get().end());
    hash_combine(
      _id,
      _dependent_servicegroups.get().begin(),
      _dependent_servicegroups.get().end());
    hash_combine(
      _id,
      _dependent_service_description.get().begin(),
      _dependent_service_description.get().end());
  }
  return (_id);
}

/**
 *  Check if the object is valid.
 *
 *  @return True if is a valid object, otherwise false.
 */
void servicedependency::check_validity() const {
  if (_service_description.empty() && _servicegroups.empty())
    throw (engine_error() << "configuration: invalid servicedependency "
           "property service_description or servicegroup is missing");
  if (_hosts.empty() && _hostgroups.empty())
    throw (engine_error() << "configuration: invalid servicedependency "
           "property host or hostgroup is missing");
  if (_dependent_hosts.empty()
      && _dependent_hostgroups.empty())
    throw (engine_error() << "configuration: invalid servicedependency "
           "property dependent_host or dependent_hostgroup is missing");
  if (_dependent_service_description.empty()
      && _dependent_servicegroups.empty())
    throw (engine_error() << "configuration: invalid servicedependency "
           "property dependent_service_description or "
           "dependent_servicegroup is missing");
  if (_dependency_period.empty())
    throw (engine_error() << "configuration: invalid servicedependency "
           "property dependency_period is missing");
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

/**
 *  Get dependency_period.
 *
 *  @return The dependency_period.
 */
std::string const& servicedependency::dependency_period() const throw () {
  return (_dependency_period);
}

/**
 *  Get dependent_hostgroup.
 *
 *  @return The dependent_hostgroup.
 */
list_string const& servicedependency::dependent_hostgroups() const throw () {
  return (_dependent_hostgroups.get());
}

/**
 *  Get dependent_hosts.
 *
 *  @return The dependent_hosts.
 */
list_string const& servicedependency::dependent_hosts() const throw () {
  return (_dependent_hosts.get());
}

/**
 *  Get dependent_servicegroup.
 *
 *  @return The dependent_servicegroup.
 */
list_string const& servicedependency::dependent_servicegroups() const throw () {
  return (_dependent_servicegroups.get());
}

/**
 *  Get dependent_service_description.
 *
 *  @return The dependent_service_description.
 */
list_string const& servicedependency::dependent_service_description() const throw () {
  return (_dependent_service_description.get());
}

/**
 *  Get execution_failure_options.
 *
 *  @return The execution_failure_options.
 */
unsigned int servicedependency::execution_failure_options() const throw () {
  return (_execution_failure_options);
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
 *  Get hostgroup.
 *
 *  @return The hostgroup.
 */
list_string const& servicedependency::hostgroups() const throw () {
  return (_hostgroups.get());
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string const& servicedependency::hosts() const throw () {
  return (_hosts.get());
}

/**
 *  Get notification_failure_options.
 *
 *  @return The notification_failure_options.
 */
unsigned int servicedependency::notification_failure_options() const throw () {
  return (_notification_failure_options);
}

/**
 *  Get servicegroup.
 *
 *  @return The servicegroup.
 */
list_string const& servicedependency::servicegroups() const throw () {
  return (_servicegroups.get());
}

/**
 *  Get service_description.
 *
 *  @return The service_description.
 */
list_string const& servicedependency::service_description() const throw () {
  return (_service_description.get());
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
 *  Set dependent_hostgroups value.
 *
 *  @param[in] value The new dependent_hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_dependent_hostgroups(std::string const& value) {
  _dependent_hostgroups.set(value);
  _id = 0;
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
  _dependent_hosts.set(value);
  _id = 0;
  return (true);
}

/**
 *  Set dependent_servicegroups value.
 *
 *  @param[in] value The new dependent_servicegroups value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_dependent_servicegroups(std::string const& value) {
  _dependent_servicegroups.set(value);
  _id = 0;
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
  _dependent_service_description.set(value);
  _id = 0;
  return (true);
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
  misc::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    misc::trim(*it);
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
  _execution_failure_options = options;
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
 *  Set hostgroups value.
 *
 *  @param[in] value The new hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_hostgroups(std::string const& value) {
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
bool servicedependency::_set_hosts(std::string const& value) {
  _hosts.set(value);
  return (true);
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
  misc::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    misc::trim(*it);
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
  _notification_failure_options = options;
  return (true);
}

/**
 *  Set servicegroups value.
 *
 *  @param[in] value The new servicegroups value.
 *
 *  @return True on success, otherwise false.
 */
bool servicedependency::_set_servicegroups(std::string const& value) {
  _servicegroups.set(value);
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
  _service_description.set(value);
  return (true);
}
