/*
** Copyright 2011-2013,2017 Centreon
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

#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/error.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<contactgroup, type, &contactgroup::method>::generic

std::unordered_map<std::string, contactgroup::setter_func> const contactgroup::_setters{
  { "contactgroup_name",    SETTER(std::string const&, _set_contactgroup_name) },
  { "alias",                SETTER(std::string const&, _set_alias) },
  { "members",              SETTER(std::string const&, _set_members) },
  { "contactgroup_members", SETTER(std::string const&, _set_contactgroup_members) }
};

/**
 *  Constructor.
 *
 *  @param[in] key The object key.
 */
contactgroup::contactgroup(key_type const& key)
  : object(object::contactgroup),
    _contactgroup_name(key) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The contactgroup to copy.
 */
contactgroup::contactgroup(contactgroup const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
contactgroup::~contactgroup() throw () {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The contactgroup to copy.
 *
 *  @return This contactgroup.
 */
contactgroup& contactgroup::operator=(contactgroup const& right) {
  if (this != &right) {
    object::operator=(right);
    _alias = right._alias;
    _contactgroup_members = right._contactgroup_members;
    _contactgroup_name = right._contactgroup_name;
    _members = right._members;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The contactgroup to compare.
 *
 *  @return True if is the same contactgroup, otherwise false.
 */
bool contactgroup::operator==(contactgroup const& right) const throw () {
  return (object::operator==(right)
          && _alias == right._alias
          && _contactgroup_members == right._contactgroup_members
          && _contactgroup_name == right._contactgroup_name
          && _members == right._members);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The contactgroup to compare.
 *
 *  @return True if is not the same contactgroup, otherwise false.
 */
bool contactgroup::operator!=(contactgroup const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool contactgroup::operator<(contactgroup const& right) const throw () {
  if (_contactgroup_name != right._contactgroup_name)
    return (_contactgroup_name < right._contactgroup_name);
  else if (_alias != right._alias)
    return (_alias < right._alias);
  else if (_contactgroup_members != right._contactgroup_members)
    return (_contactgroup_members < right._contactgroup_members);
  return (_members < right._members);
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void contactgroup::check_validity() const {
  if (_contactgroup_name.empty())
    throw (engine_error() << "Contact group has no name "
           "(property 'contactgroup_name')");
  return ;
}

/**
 *  Get the contact group key.
 *
 *  @return The contact group name.
 */
contactgroup::key_type const& contactgroup::key() const throw () {
  return (_contactgroup_name);
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void contactgroup::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "Cannot merge contact group with '"
           << obj.type() << "'");
  contactgroup const& tmpl(static_cast<contactgroup const&>(obj));

  MRG_DEFAULT(_alias);
  MRG_INHERIT(_contactgroup_members);
  MRG_DEFAULT(_contactgroup_name);
  MRG_INHERIT(_members);
}

/**
 *  Parse and set the contactgroup property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool contactgroup::parse(char const* key, char const* value) {
  std::unordered_map<std::string, contactgroup::setter_func>::const_iterator
    it{_setters.find(key)};
  if (it != _setters.end())
    return (it->second)(*this, value);
  return false;
}

/**
 *  Get alias.
 *
 *  @return The alias.
 */
std::string const& contactgroup::alias() const throw () {
  return (_alias);
}

/**
 *  Get contactgroup_members.
 *
 *  @return The contactgroup_members.
 */
set_string& contactgroup::contactgroup_members() throw () {
  return (*_contactgroup_members);
}

/**
 *  Get contactgroup_members.
 *
 *  @return The contactgroup_members.
 */
set_string const& contactgroup::contactgroup_members() const throw () {
  return (*_contactgroup_members);
}

/**
 *  Get contactgroup_name.
 *
 *  @return The contactgroup_name.
 */
std::string const& contactgroup::contactgroup_name() const throw () {
  return (_contactgroup_name);
}

/**
 *  Get members.
 *
 *  @return The members.
 */
set_string& contactgroup::members() throw () {
  return (*_members);
}

/**
 *  Get members.
 *
 *  @return The members.
 */
set_string const& contactgroup::members() const throw () {
  return (*_members);
}

/**
 *  Set alias value.
 *
 *  @param[in] value The new alias value.
 *
 *  @return True on success, otherwise false.
 */
bool contactgroup::_set_alias(std::string const& value) {
  _alias = value;
  return (true);
}

/**
 *  Set contactgroup_members value.
 *
 *  @param[in] value The new contactgroup_members value.
 *
 *  @return True on success, otherwise false.
 */
bool contactgroup::_set_contactgroup_members(std::string const& value) {
  _contactgroup_members = value;
  return (true);
}

/**
 *  Set contactgroup_name value.
 *
 *  @param[in] value The new contactgroup_name value.
 *
 *  @return True on success, otherwise false.
 */
bool contactgroup::_set_contactgroup_name(std::string const& value) {
  _contactgroup_name = value;
  return (true);
}

/**
 *  Set members value.
 *
 *  @param[in] value The new members value.
 *
 *  @return True on success, otherwise false.
 */
bool contactgroup::_set_members(std::string const& value) {
  _members = value;
  return (true);
}
