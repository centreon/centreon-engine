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

#include "com/centreon/engine/configuration/servicegroup.hh"
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine;

#define SETTER(type, method) \
  &configuration::object::setter< \
     configuration::servicegroup, \
     type, \
     &configuration::servicegroup::method>::generic

static struct {
  std::string const name;
  bool (*func)(configuration::servicegroup&, std::string const&);
} gl_setters[] = {
  { "servicegroup_name",    SETTER(std::string const&, _set_servicegroup_name) },
  { "alias",                SETTER(std::string const&, _set_alias) },
  { "members",              SETTER(std::string const&, _set_members) },
  { "servicegroup_members", SETTER(std::string const&, _set_servicegroup_members) },
  { "notes",                SETTER(std::string const&, _set_notes) },
  { "notes_url",            SETTER(std::string const&, _set_notes_url) },
  { "action_url",           SETTER(std::string const&, _set_action_url) }
};


/**
 *  Default constructor.
 */
configuration::servicegroup::servicegroup()
  : object(object::servicegroup, "servicegroup") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The servicegroup to copy.
 */
configuration::servicegroup::servicegroup(servicegroup const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
configuration::servicegroup::~servicegroup() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The servicegroup to copy.
 *
 *  @return This servicegroup.
 */
configuration::servicegroup& configuration::servicegroup::operator=(
                               servicegroup const& right) {
  if (this != &right) {
    object::operator=(right);
    _action_url = right._action_url;
    _alias = right._alias;
    _members = right._members;
    _notes = right._notes;
    _notes_url = right._notes_url;
    _servicegroup_members = right._servicegroup_members;
    _servicegroup_name = right._servicegroup_name;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The servicegroup to compare.
 *
 *  @return True if is the same servicegroup, otherwise false.
 */
bool configuration::servicegroup::operator==(
       servicegroup const& right) const throw () {
  return (object::operator==(right)
          && _action_url == right._action_url
          && _alias == right._alias
          && _members == right._members
          && _notes == right._notes
          && _notes_url == right._notes_url
          && _servicegroup_members == right._servicegroup_members
          && _servicegroup_name == right._servicegroup_name);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The servicegroup to compare.
 *
 *  @return True if is not the same servicegroup, otherwise false.
 */
bool configuration::servicegroup::operator!=(
       servicegroup const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t configuration::servicegroup::id() const throw () {
  return (_id);
}

/**
 *  Check if the object is valid.
 *
 *  @return True if is a valid object, otherwise false.
 */
bool configuration::servicegroup::is_valid() const throw () {
  return (!_servicegroup_name.empty());
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void configuration::servicegroup::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  servicegroup const& tmpl(static_cast<servicegroup const&>(obj));

  MRG_DEFAULT(_action_url);
  MRG_DEFAULT(_alias);
  MRG_INHERIT(_members);
  MRG_DEFAULT(_notes);
  MRG_DEFAULT(_notes_url);
  MRG_INHERIT(_servicegroup_members);
  MRG_DEFAULT(_servicegroup_name);
}

/**
 *  Parse and set the servicegroup property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::servicegroup::parse(
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
 *  Set action_url value.
 *
 *  @param[in] value The new action_url value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::servicegroup::_set_action_url(
       std::string const& value) {
  _action_url = value;
  return (true);
}

/**
 *  Set alias value.
 *
 *  @param[in] value The new alias value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::servicegroup::_set_alias(
       std::string const& value) {
  _alias = value;
  return (true);
}

/**
 *  Set members value.
 *
 *  @param[in] value The new members value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::servicegroup::_set_members(
       std::string const& value) {
  _members.set(value);
  return (true);
}

/**
 *  Set notes value.
 *
 *  @param[in] value The new notes value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::servicegroup::_set_notes(
       std::string const& value) {
  _notes = value;
  return (true);
}

/**
 *  Set notes_url value.
 *
 *  @param[in] value The new notes_url value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::servicegroup::_set_notes_url(
       std::string const& value) {
  _notes_url = value;
  return (true);
}

/**
 *  Set servicegroup_members value.
 *
 *  @param[in] value The new servicegroup_members value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::servicegroup::_set_servicegroup_members(
       std::string const& value) {
  _servicegroup_members.set(value);
  return (true);
}

/**
 *  Set servicegroup_name value.
 *
 *  @param[in] value The new servicegroup_name value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::servicegroup::_set_servicegroup_name(
       std::string const& value) {
  _servicegroup_name = value;
  _id = _hash(value);
  return (true);
}
