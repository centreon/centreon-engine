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
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<servicegroup, type, &servicegroup::method>::generic

static struct {
  std::string const name;
  bool (*func)(servicegroup&, std::string const&);
} gl_setters[] = {
  { "servicegroup_name",    setter(std::string const&, _set_servicegroup_name) },
  { "alias",                setter(std::string const&, _set_alias) },
  { "members",              setter(std::string const&, _set_members) },
  { "servicegroup_members", setter(std::string const&, _set_servicegroup_members) },
  { "notes",                setter(std::string const&, _set_notes) },
  { "notes_url",            setter(std::string const&, _set_notes_url) },
  { "action_url",           setter(std::string const&, _set_action_url) }
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
  return (object::operator==(right));
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
  return (object::parse(key, value));
}

void servicegroup::_set_action_url(std::string const& value) {
  _action_url = value;
}

void servicegroup::_set_alias(std::string const& value) {
  _alias = value;
}

void servicegroup::_set_members(std::string const& value) {
  _members.clear();
  misc::split(value, _members, ',');
}

void servicegroup::_set_notes(std::string const& value) {
  _notes = value;
}

void servicegroup::_set_notes_url(std::string const& value) {
  _notes_url = value;
}

void servicegroup::_set_servicegroup_members(std::string const& value) {
  _servicegroup_members.clear();
  misc::split(value, _servicegroup_members, ',');
}

void servicegroup::_set_servicegroup_name(std::string const& value) {
  _servicegroup_name = value;
}
