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
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<hostgroup, type, &hostgroup::method>::generic

static struct {
  std::string const name;
  bool (*func)(hostgroup&, std::string const&);
} gl_setters[] = {
  { "hostgroup_name",    setter(std::string const&, _set_hostgroup_members) },
  { "alias",             setter(std::string const&, _set_alias) },
  { "members",           setter(std::string const&, _set_members) },
  { "hostgroup_members", setter(std::string const&, _set_hostgroup_members) },
  { "notes",             setter(std::string const&, _set_notes) },
  { "notes_url",         setter(std::string const&, _set_notes_url) },
  { "action_url",        setter(std::string const&, _set_action_url) }
};

/**
 *  Default constructor.
 */
hostgroup::hostgroup()
  : object("hostgroup") {

}

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
hostgroup::~hostgroup() throw () {

}

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
  return (object::operator==(right));
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
 *  Parse and set the hostgroup property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool hostgroup::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (object::parse(key, value));
}

void hostgroup::_set_action_url(std::string const& value) {
  _action_url = value;
}

void hostgroup::_set_alias(std::string const& value) {
  _alias = value;
}

void hostgroup::_set_hostgroup_members(std::string const& value) {
  _hostgroup_members.clear();
  misc::split(value, _hostgroup_members, ',');
}

void hostgroup::_set_hostgroup_name(std::string const& value) {
  _hostgroup_name = value;
}

void hostgroup::_set_members(std::string const& value) {
  _members.clear();
  misc::split(value, _members, ',');
}

void hostgroup::_set_notes(std::string const& value) {
  _notes = value;
}

void hostgroup::_set_notes_url(std::string const& value) {
  _notes_url = value;
}
