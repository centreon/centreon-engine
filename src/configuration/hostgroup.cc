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

#include "com/centreon/engine/configuration/hostgroup.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"

extern int config_warnings;
extern int config_errors;

using namespace com::centreon;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

#define SETTER(type, method) \
  &object::setter<hostgroup, type, &hostgroup::method>::generic

hostgroup::setters const hostgroup::_setters[] = {
  { "hostgroup_name",    SETTER(std::string const&, _set_hostgroup_name) },
  { "alias",             SETTER(std::string const&, _set_alias) },
  { "members",           SETTER(std::string const&, _set_members) },
  { "hostgroup_members", SETTER(std::string const&, _set_hostgroup_members) },

  // Deprecated.
  { "notes",             SETTER(std::string const&, _set_notes) },
  { "notes_url",         SETTER(std::string const&, _set_notes_url) },
  { "action_url",        SETTER(std::string const&, _set_action_url) }
};

/**
 *  Constructor.
 *
 *  @param[in] key The object key.
 */
hostgroup::hostgroup(key_type const& key)
  : object(object::hostgroup),
    _hostgroup_name(key),
    _resolved(false) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostgroup to copy.
 */
hostgroup::hostgroup(hostgroup const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
hostgroup::~hostgroup() throw () {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostgroup to copy.
 *
 *  @return This hostgroup.
 */
hostgroup& hostgroup::operator=(hostgroup const& right) {
  if (this != &right) {
    object::operator=(right);
    _alias = right._alias;
    _hostgroup_members = right._hostgroup_members;
    _hostgroup_name = right._hostgroup_name;
    _members = right._members;
    _resolved = right._resolved;
    _resolved_members = right._resolved_members;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostgroup to compare.
 *
 *  @return True if is the same hostgroup, otherwise false.
 */
bool hostgroup::operator==(hostgroup const& right) const throw () {
  return (object::operator==(right)
          && _alias == right._alias
          && _hostgroup_members == right._hostgroup_members
          && _hostgroup_name == right._hostgroup_name
          && _members == right._members
          && _resolved == right._resolved
          && _resolved_members == right._resolved_members);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostgroup to compare.
 *
 *  @return True if is not the same hostgroup, otherwise false.
 */
bool hostgroup::operator!=(hostgroup const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool hostgroup::operator<(hostgroup const& right) const throw () {
  if (_hostgroup_name != right._hostgroup_name)
    return (_hostgroup_name < right._hostgroup_name);
  else if (_alias != right._alias)
    return (_alias < right._alias);
  else if (_hostgroup_members != right._hostgroup_members)
    return (_hostgroup_members < right._hostgroup_members);
  else if (_members != right._members)
    return (_members < right._members);
  else if (_resolved != right._resolved)
    return (_resolved < right._resolved);
  return (_resolved_members < right._resolved_members);
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void hostgroup::check_validity() const {
  if (_hostgroup_name.empty())
    throw (engine_error() << "Host group has no name "
           "(property 'hostgroup_name')");
  return ;
}

/**
 *  Get the host group key.
 *
 *  @return The host group name.
 */
hostgroup::key_type const& hostgroup::key() const throw () {
  return (_hostgroup_name);
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void hostgroup::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "Cannot merge host group with '"
           << obj.type() << "'");
  hostgroup const& tmpl(static_cast<hostgroup const&>(obj));
  MRG_DEFAULT(_alias);
  MRG_INHERIT(_hostgroup_members);
  MRG_DEFAULT(_hostgroup_name);
  MRG_INHERIT(_members);
}

/**
 *  Parse and set the hostgroup property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool hostgroup::parse(char const* key, char const* value) {
  for (unsigned int i(0);
       i < sizeof(_setters) / sizeof(_setters[0]);
       ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  return (false);
}

/**
 *  Get alias.
 *
 *  @return The alias.
 */
std::string const& hostgroup::alias() const throw () {
  return (_alias);
}

/**
 *  Get hostgroup_members.
 *
 *  @return The hostgroup_members.
 */
list_string const& hostgroup::hostgroup_members() const throw () {
  return (*_hostgroup_members);
}

/**
 *  Get hostgroup_name.
 *
 *  @return The hostgroup_name.
 */
std::string const& hostgroup::hostgroup_name() const throw () {
  return (_hostgroup_name);
}

/**
 *  Get members.
 *
 *  @return The members.
 */
list_string& hostgroup::members() throw () {
  return (*_members);
}

/**
 *  Get members.
 *
 *  @return The members.
 */
list_string const& hostgroup::members() const throw () {
  return (*_members);
}

/**
 *  Check if hostgroup was resolved.
 *
 *  @return True if hostgroup was resolved, false otherwise.
 */
bool hostgroup::is_resolved() const throw () {
  return (_resolved);
}

/**
 *  Get resolved members.
 *
 *  @return Modifiable list of members.
 */
set_string& hostgroup::resolved_members() const throw () {
  return (_resolved_members);
}

/**
 *  Set whether or not hostgroup has been resolved.
 *
 *  @param[in] resolved True if hostgroup has been resolved, false
 *                      otherwise.
 */
void hostgroup::set_resolved(bool resolved) const throw () {
  _resolved = resolved;
  return ;
}

/**
 *  Deprecated variable.
 *
 *  @param[in] value  Unused.
 *
 *  @return           True.
 */
bool hostgroup::_set_action_url(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
    << "Warning: hostgroup action_url was ignored";
  ++config_warnings;
  return (true);
}

/**
 *  Set alias value.
 *
 *  @param[in] value The new alias value.
 *
 *  @return True on success, otherwise false.
 */
bool hostgroup::_set_alias(std::string const& value) {
  _alias = value;
  return (true);
}

/**
 *  Set hostgroup_members value.
 *
 *  @param[in] value The new hostgroup_members value.
 *
 *  @return True on success, otherwise false.
 */
bool hostgroup::_set_hostgroup_members(std::string const& value) {
  _hostgroup_members = value;
  return (true);
}

/**
 *  Set hostgroup_name value.
 *
 *  @param[in] value The new hostgroup_name value.
 *
 *  @return True on success, otherwise false.
 */
bool hostgroup::_set_hostgroup_name(std::string const& value) {
  _hostgroup_name = value;
  return (true);
}

/**
 *  Set members value.
 *
 *  @param[in] value The new members value.
 *
 *  @return True on success, otherwise false.
 */
bool hostgroup::_set_members(std::string const& value) {
  _members = value;
  return (true);
}

/**
 *  Deprecated variable.
 *
 *  @param[in] value  Unused.
 *
 *  @return           True.
 */
bool hostgroup::_set_notes(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
    << "Warning: hostgroup notes was ignored";
  ++config_warnings;
  return (true);
}

/**
 *  Deprecated variable.
 *
 *  @param[in] value  Unused.
 *
 *  @return           True.
 */
bool hostgroup::_set_notes_url(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
    << "Warning: hostgroup notes_url was ignored";
  ++config_warnings;
  return (true);
}
