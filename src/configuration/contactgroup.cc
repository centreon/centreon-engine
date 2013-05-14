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
#include "com/centreon/engine/misc/string.hh"

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
  : object("contactgroup") {

}

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
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void contactgroup::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "XXX: todo");
  contactgroup const& tmpl(static_cast<contactgroup const&>(obj));

  MRG_STRING(_alias);
  MRG_INHERIT(_contactgroup_members);
  MRG_STRING(_contactgroup_name);
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

void contactgroup::_set_alias(std::string const& value) {
  _alias = value;
}

void contactgroup::_set_contactgroup_members(std::string const& value) {
  _contactgroup_members.clear();
  misc::split(value, _contactgroup_members.get(), ',');
}

void contactgroup::_set_contactgroup_name(std::string const& value) {
  _contactgroup_name = value;
}

void contactgroup::_set_members(std::string const& value) {
  _members.clear();
  misc::split(value, _members.get(), ',');
}
