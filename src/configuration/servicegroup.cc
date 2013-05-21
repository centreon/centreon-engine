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

using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<servicegroup, type, &servicegroup::method>::generic

static struct {
  std::string const name;
  bool (*func)(servicegroup&, std::string const&);
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
servicegroup::servicegroup()
  : object("servicegroup") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The servicegroup to copy.
 */
servicegroup::servicegroup(servicegroup const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
servicegroup::~servicegroup() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The servicegroup to copy.
 *
 *  @return This servicegroup.
 */
servicegroup& servicegroup::operator=(servicegroup const& right) {
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
bool servicegroup::operator==(servicegroup const& right) const throw () {
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
bool servicegroup::operator!=(servicegroup const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void servicegroup::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  servicegroup const& tmpl(static_cast<servicegroup const&>(obj));

  MRG_STRING(_action_url);
  MRG_STRING(_alias);
  MRG_INHERIT(_members);
  MRG_STRING(_notes);
  MRG_STRING(_notes_url);
  MRG_INHERIT(_servicegroup_members);
  MRG_STRING(_servicegroup_name);
}

/**
 *  Parse and set the servicegroup property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool servicegroup::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (false);
}

void servicegroup::_set_action_url(std::string const& value) {
  _action_url = value;
}

void servicegroup::_set_alias(std::string const& value) {
  _alias = value;
}

void servicegroup::_set_members(std::string const& value) {
  _members.set(value);
}

void servicegroup::_set_notes(std::string const& value) {
  _notes = value;
}

void servicegroup::_set_notes_url(std::string const& value) {
  _notes_url = value;
}

void servicegroup::_set_servicegroup_members(std::string const& value) {
  _servicegroup_members.set(value);
}

void servicegroup::_set_servicegroup_name(std::string const& value) {
  _servicegroup_name = value;
}
