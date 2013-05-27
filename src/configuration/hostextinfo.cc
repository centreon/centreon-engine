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

#define SETTER(type, method) \
  &object::setter<hostextinfo, type, &hostextinfo::method>::generic

static struct {
  std::string const name;
  bool (*func)(hostextinfo&, std::string const&);
} gl_setters[] = {
  { "host_name",       SETTER(std::string const&, _set_hosts) },
  { "hostgroup",       SETTER(std::string const&, _set_hostgroups) },
  { "hostgroup_name",  SETTER(std::string const&, _set_hostgroups) },
  { "notes",           SETTER(std::string const&, _set_notes) },
  { "notes_url",       SETTER(std::string const&, _set_notes_url) },
  { "action_url",      SETTER(std::string const&, _set_action_url) },
  { "icon_image",      SETTER(std::string const&, _set_icon_image) },
  { "icon_image_alt",  SETTER(std::string const&, _set_icon_image_alt) },
  { "vrml_image",      SETTER(std::string const&, _set_vrml_image) },
  { "gd2_image",       SETTER(std::string const&, _set_gd2_image) },
  { "statusmap_image", SETTER(std::string const&, _set_statusmap_image) },
  { "2d_coords",       SETTER(std::string const&, _set_2d_coords) },
  { "3d_coords",       SETTER(std::string const&, _set_3d_coords) }
};

// Default values.
static point_2d const default_2d_coords(-1, -1);
static point_3d const default_3d_coords(0.0, 0.0, 0.0);


/**
 *  Default constructor.
 */
hostextinfo::hostextinfo()
  : object(object::hostextinfo, "hostextinfo") {

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
    _2d_coords = right._2d_coords;
    _3d_coords = right._3d_coords;
    _action_url = right._action_url;
    _gd2_image = right._gd2_image;
    _hostgroups = right._hostgroups;
    _hosts = right._hosts;
    _icon_image = right._icon_image;
    _icon_image_alt = right._icon_image_alt;
    _notes = right._notes;
    _notes_url = right._notes_url;
    _statusmap_image = right._statusmap_image;
    _vrml_image = right._vrml_image;
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
  return (object::operator==(right)
          && _2d_coords == right._2d_coords
          && _3d_coords == right._3d_coords
          && _action_url == right._action_url
          && _gd2_image == right._gd2_image
          && _hostgroups == right._hostgroups
          && _hosts == right._hosts
          && _icon_image == right._icon_image
          && _icon_image_alt == right._icon_image_alt
          && _notes == right._notes
          && _notes_url == right._notes_url
          && _statusmap_image == right._statusmap_image
          && _vrml_image == right._vrml_image);
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
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t hostextinfo::id() const throw () {
  return (_id);
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void hostextinfo::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  hostextinfo const& tmpl(static_cast<hostextinfo const&>(obj));

  MRG_OPTION(_2d_coords);
  MRG_OPTION(_3d_coords);
  MRG_DEFAULT(_action_url);
  MRG_DEFAULT(_gd2_image);
  MRG_INHERIT(_hostgroups);
  MRG_INHERIT(_hosts);
  MRG_DEFAULT(_icon_image);
  MRG_DEFAULT(_icon_image_alt);
  MRG_DEFAULT(_notes);
  MRG_DEFAULT(_notes_url);
  MRG_DEFAULT(_statusmap_image);
  MRG_DEFAULT(_vrml_image);
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
  return (false);
}

/**
 *  Set 2d_coords value.
 *
 *  @param[in] value The new 2d_coords value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_2d_coords(std::string const& value) {
  std::list<std::string> coords;
  misc::split(value, coords, ',');
  if (coords.size() != 2)
    return (false);

  int x;
  if (!misc::to(misc::trim(coords.front()), x))
    return (false);
  coords.pop_front();

  int y;
  if (!misc::to(misc::trim(coords.front()), y))
    return (false);

  _2d_coords = point_2d(x, y);
  return (true);
}

/**
 *  Set 3d_coords value.
 *
 *  @param[in] value The new 3d_coords value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_3d_coords(std::string const& value) {
  std::list<std::string> coords;
  misc::split(value, coords, ',');
  if (coords.size() != 2)
    return (false);

  double x;
  if (!misc::to(misc::trim(coords.front()), x))
    return (false);
  coords.pop_front();

  double y;
  if (!misc::to(misc::trim(coords.front()), y))
    return (false);
  coords.pop_front();

  double z;
  if (!misc::to(misc::trim(coords.front()), z))
    return (false);

  _3d_coords = point_3d(x, y, z);
  return (true);
}

/**
 *  Set action_url value.
 *
 *  @param[in] value The new action_url value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_action_url(std::string const& value) {
  _action_url = value;
  return (true);
}

/**
 *  Set gd2_image value.
 *
 *  @param[in] value The new gd2_image value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_gd2_image(std::string const& value) {
  _gd2_image = value;
  return (true);
}

/**
 *  Set hostgroups value.
 *
 *  @param[in] value The new hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_hostgroups(std::string const& value) {
  _hostgroups.set(value);
  return (true);
}

/**
 *  Set hosts value.
 *
 *  @param[in] value The new hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_hosts(std::string const& value) {
  _hosts.set(value);
  _id = 0;
  _hash(_id, _hosts.get());
  return (true);
}

/**
 *  Set icon_image value.
 *
 *  @param[in] value The new icon_image value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_icon_image(std::string const& value) {
  _icon_image = value;
  return (true);
}

/**
 *  Set icon_image_alt value.
 *
 *  @param[in] value The new icon_image_alt value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_icon_image_alt(std::string const& value) {
  _icon_image_alt = value;
  return (true);
}

/**
 *  Set notes value.
 *
 *  @param[in] value The new notes value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_notes(std::string const& value) {
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
bool hostextinfo::_set_notes_url(std::string const& value) {
  _notes_url = value;
  return (true);
}

/**
 *  Set statusmap_image value.
 *
 *  @param[in] value The new statusmap_image value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_statusmap_image(std::string const& value) {
  _statusmap_image = value;
  return (true);
}

/**
 *  Set vrml_image value.
 *
 *  @param[in] value The new vrml_image value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_vrml_image(std::string const& value) {
  _vrml_image = value;
  return (true);
}
