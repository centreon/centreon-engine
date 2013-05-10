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

#include "com/centreon/engine/configuration/hostextinfo.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<hostextinfo, type, &hostextinfo::method>::generic

static struct {
  std::string const name;
  bool (*func)(hostextinfo&, std::string const&);
} gl_setters[] = {
  { "host_name",       setter(std::string const&, _set_hosts) },
  { "hostgroup",       setter(std::string const&, _set_hostgroups) },
  { "hostgroup_name",  setter(std::string const&, _set_hostgroups) },
  { "notes",           setter(std::string const&, _set_notes) },
  { "notes_url",       setter(std::string const&, _set_notes_url) },
  { "action_url",      setter(std::string const&, _set_action_url) },
  { "icon_image",      setter(std::string const&, _set_icon_image) },
  { "icon_image_alt",  setter(std::string const&, _set_icon_image_alt) },
  { "vrml_image",      setter(std::string const&, _set_vrml_image) },
  { "gd2_image",       setter(std::string const&, _set_gd2_image) },
  { "statusmap_image", setter(std::string const&, _set_statusmap_image) },
  { "2d_coords",       setter(int, _set_2d_coords) },
  { "3d_coords",       setter(int, _set_3d_coords) }
};

/**
 *  Default constructor.
 */
hostextinfo::hostextinfo()
  : object("hostextinfo") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostextinfo to copy.
 */
hostextinfo::hostextinfo(hostextinfo const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
hostextinfo::~hostextinfo() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostextinfo to copy.
 *
 *  @return This hostextinfo.
 */
hostextinfo& hostextinfo::operator=(hostextinfo const& right) {
  if (this != &right) {
    object::operator=(right);
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostextinfo to compare.
 *
 *  @return True if is the same hostextinfo, otherwise false.
 */
bool hostextinfo::operator==(hostextinfo const& right) const throw () {
  return (object::operator==(right));
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostextinfo to compare.
 *
 *  @return True if is not the same hostextinfo, otherwise false.
 */
bool hostextinfo::operator!=(hostextinfo const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Parse and set the hostextinfo property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (object::parse(key, value));
}

void hostextinfo::_set_2d_coords(int value) {
  _2d_coords = value;
}

void hostextinfo::_set_3d_coords(int value) {
  _3d_coords = value;
}

void hostextinfo::_set_action_url(std::string const& value) {
  _action_url = value;
}

void hostextinfo::_set_gd2_image(std::string const& value) {
  _gd2_image = value;
}

void hostextinfo::_set_hostgroups(std::string const& value) {
  _hostgroups.clear();
  misc::split(value, _hostgroups, ',');
}

void hostextinfo::_set_hosts(std::string const& value) {
  _hosts.clear();
  misc::split(value, _hosts, ',');
}

void hostextinfo::_set_icon_image(std::string const& value) {
  _icon_image = value;
}

void hostextinfo::_set_icon_image_alt(std::string const& value) {
  _icon_image_alt = value;
}

void hostextinfo::_set_notes(std::string const& value) {
  _notes = value;
}

void hostextinfo::_set_notes_url(std::string const& value) {
  _notes_url = value;
}

void hostextinfo::_set_statusmap_image(std::string const& value) {
  _statusmap_image = value;
}

void hostextinfo::_set_vrml_image(std::string const& value) {
  _vrml_image = value;
}
