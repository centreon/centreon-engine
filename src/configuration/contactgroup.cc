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

#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/hash.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<contactgroup, type, &contactgroup::method>::generic

static struct {
  std::string const name;
  bool (*func)(contactgroup&, std::string const&);
} gl_setters[] = {
  { "contactgroup_name",    SETTER(std::string const&, _set_contactgroup_name) },
  { "alias",                SETTER(std::string const&, _set_alias) },
  { "members",              SETTER(std::string const&, _set_members) },
  { "contactgroup_members", SETTER(std::string const&, _set_contactgroup_members) }
};

/**
 *  Default constructor.
 */
contactgroup::contactgroup()
  : object(object::contactgroup), _resolved(false) {}

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
contactgroup::~contactgroup() throw () {

}

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
    _resolved = right._resolved;
    _resolved_members = right._resolved_members;
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
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t contactgroup::id() const throw () {
  return (_id);
}

/**
 *  Check if the object is valid.
 *
 *  @return True if is a valid object, otherwise false.
 */
void contactgroup::check_validity() const {
  if (_contactgroup_name.empty())
    throw (engine_error() << "configuration: invalid contactgroup "
           "property contactgroup_name is missing");
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void contactgroup::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
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
bool contactgroup::parse(
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
list_string const& contactgroup::contactgroup_members() const throw () {
  return (_contactgroup_members->get());
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
list_string& contactgroup::members() throw () {
  return (_members->get());
}

/**
 *  Get members.
 *
 *  @return The members.
 */
list_string const& contactgroup::members() const throw () {
  return (_members->get());
}

/**
 *  Check if contactgroup was resolved.
 *
 *  @return True if contactgroup was resolved, false otherwise.
 */
bool contactgroup::is_resolved() const throw () {
  return (_resolved);
}

/**
 *  Get resolved members.
 *
 *  @return Modified list of members.
 */
set_string& contactgroup::resolved_members() const throw () {
  return (_resolved_members);
}

/**
 *  Set whether or not contactgroup has been resolved.
 *
 *  @param[in] resolved True if contactgroup has been resolved, false
 *                      otherwise.
 */
void contactgroup::set_resolved(bool resolved) const throw () {
  _resolved = resolved;
  return ;
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
  _contactgroup_members.set(value);
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
  _id = hash(value);
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
  _members.set(value);
  return (true);
}
