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

#include "com/centreon/engine/configuration/serviceextinfo.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<serviceextinfo, type, &serviceextinfo::method>::generic

static struct {
  std::string const name;
  bool (*func)(serviceextinfo&, std::string const&);
} gl_setters[] = {
  { "host_name",           setter(std::string const&, _set_hosts) },
  { "hostgroup",           setter(std::string const&, _set_hostgroups) },
  { "hostgroup_name",      setter(std::string const&, _set_hostgroups) },
  { "service_description", setter(std::string const&, _set_service_description) },
  { "notes",               setter(std::string const&, _set_notes) },
  { "notes_url",           setter(std::string const&, _set_notes_url) },
  { "action_url",          setter(std::string const&, _set_action_url) },
  { "icon_image",          setter(std::string const&, _set_icon_image) },
  { "icon_image_alt",      setter(std::string const&, _set_icon_image_alt) }
};

/**
 *  Default constructor.
 */
serviceextinfo::serviceextinfo()
  : object("serviceextinfo") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The serviceextinfo to copy.
 */
serviceextinfo::serviceextinfo(serviceextinfo const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
serviceextinfo::~serviceextinfo() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The serviceextinfo to copy.
 *
 *  @return This serviceextinfo.
 */
serviceextinfo& serviceextinfo::operator=(serviceextinfo const& right) {
  if (this != &right) {
    object::operator=(right);
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The serviceextinfo to compare.
 *
 *  @return True if is the same serviceextinfo, otherwise false.
 */
bool serviceextinfo::operator==(serviceextinfo const& right) const throw () {
  return (object::operator==(right));
}

/**
 *  Equal operator.
 *
 *  @param[in] right The serviceextinfo to compare.
 *
 *  @return True if is not the same serviceextinfo, otherwise false.
 */
bool serviceextinfo::operator!=(serviceextinfo const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Parse and set the serviceextinfo property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceextinfo::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (object::parse(key, value));
}

void serviceextinfo::_set_action_url(std::string const& value) {
  _action_url = value;
}

void serviceextinfo::_set_icon_image(std::string const& value) {
  _icon_image = value;
}

void serviceextinfo::_set_icon_image_alt(std::string const& value) {
  _icon_image_alt = value;
}

void serviceextinfo::_set_hosts(std::string const& value) {
  _hosts.clear();
  misc::split(value, _hosts, ',');
}

void serviceextinfo::_set_hostgroups(std::string const& value) {
  _hostgroups.clear();
  misc::split(value, _hostgroups, ',');
}

void serviceextinfo::_set_notes(std::string const& value) {
  _notes = value;
}

void serviceextinfo::_set_notes_url(std::string const& value) {
  _notes_url = value;
}

void serviceextinfo::_set_service_description(std::string const& value) {
  _service_description = value;
}
