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
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<hostextinfo, type, &hostextinfo::method>::generic

std::unordered_map<std::string, hostextinfo::setter_func> const
    hostextinfo::_setters{
        {"host_name", SETTER(std::string const&, _set_hosts)},
        {"hostgroup", SETTER(std::string const&, _set_hostgroups)},
        {"hostgroup_name", SETTER(std::string const&, _set_hostgroups)},
        {"notes", SETTER(std::string const&, _set_notes)},
        {"notes_url", SETTER(std::string const&, _set_notes_url)},
        {"action_url", SETTER(std::string const&, _set_action_url)},
        {"icon_image", SETTER(std::string const&, _set_icon_image)},
        {"icon_image_alt", SETTER(std::string const&, _set_icon_image_alt)},
        {"vrml_image", SETTER(std::string const&, _set_vrml_image)},
        {"gd2_image", SETTER(std::string const&, _set_statusmap_image)},
        {"statusmap_image", SETTER(std::string const&, _set_statusmap_image)},
        {"2d_coords", SETTER(std::string const&, _set_coords_2d)},
        {"3d_coords", SETTER(std::string const&, _set_coords_3d)}};

// Default values.
static point_2d const default_coords_2d(-1, -1);
static point_3d const default_coords_3d(0.0, 0.0, 0.0);

/**
 *  Default constructor.
 */
hostextinfo::hostextinfo()
    : object(object::hostextinfo),
      _coords_2d(default_coords_2d),
      _coords_3d(default_coords_3d) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostextinfo to copy.
 */
hostextinfo::hostextinfo(hostextinfo const& right) : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
hostextinfo::~hostextinfo() throw() {}

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
    _action_url = right._action_url;
    _coords_2d = right._coords_2d;
    _coords_3d = right._coords_3d;
    _hostgroups = right._hostgroups;
    _hosts = right._hosts;
    _icon_image = right._icon_image;
    _icon_image_alt = right._icon_image_alt;
    _notes = right._notes;
    _notes_url = right._notes_url;
    _statusmap_image = right._statusmap_image;
    _vrml_image = right._vrml_image;
  }
  return *this;
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostextinfo to compare.
 *
 *  @return True if is the same hostextinfo, otherwise false.
 */
bool hostextinfo::operator==(hostextinfo const& right) const throw() {
  return (object::operator==(right) && _action_url == right._action_url &&
          _coords_2d == right._coords_2d && _coords_3d == right._coords_3d &&
          _hostgroups == right._hostgroups && _hosts == right._hosts &&
          _icon_image == right._icon_image &&
          _icon_image_alt == right._icon_image_alt && _notes == right._notes &&
          _notes_url == right._notes_url &&
          _statusmap_image == right._statusmap_image &&
          _vrml_image == right._vrml_image);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostextinfo to compare.
 *
 *  @return True if is not the same hostextinfo, otherwise false.
 */
bool hostextinfo::operator!=(hostextinfo const& right) const throw() {
  return !operator==(right);
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void hostextinfo::check_validity() const {
  if (_hostgroups->empty() && _hosts->empty())
    throw(engine_error()
          << "Host extended information is not attached"
          << " to any host or host group (properties 'host_name' or "
          << "'hostgroup_name', respectively)");
  return;
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void hostextinfo::merge(object const& obj) {
  if (obj.type() != _type)
    throw(engine_error() << "Cannot merge host extended information with '"
                         << obj.type() << "'");
  hostextinfo const& tmpl(static_cast<hostextinfo const&>(obj));

  MRG_DEFAULT(_action_url);
  MRG_OPTION(_coords_2d);
  MRG_OPTION(_coords_3d);
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
bool hostextinfo::parse(char const* key, char const* value) {
  std::unordered_map<std::string, hostextinfo::setter_func>::const_iterator it{
      _setters.find(key)};
  if (it != _setters.end())
    return (it->second)(*this, value);
  return false;
}

/**
 *  Get action_url.
 *
 *  @return The action_url.
 */
std::string const& hostextinfo::action_url() const throw() {
  return _action_url;
}

/**
 *  Get coords_2d.
 *
 *  @return The coords_2d.
 */
point_2d const& hostextinfo::coords_2d() const throw() {
  return _coords_2d.get();
}

/**
 *  Get 3d_coords.
 *
 *  @return The 3d_coords.
 */
point_3d const& hostextinfo::coords_3d() const throw() {
  return _coords_3d.get();
}

/**
 *  Get hostgroups.
 *
 *  @return The hostgroups.
 */
set_string const& hostextinfo::hostgroups() const throw() {
  return *_hostgroups;
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
set_string const& hostextinfo::hosts() const throw() {
  return *_hosts;
}

/**
 *  Get icon_image.
 *
 *  @return The icon_image.
 */
std::string const& hostextinfo::icon_image() const throw() {
  return _icon_image;
}

/**
 *  Get icon_image_alt.
 *
 *  @return The icon_image_alt.
 */
std::string const& hostextinfo::icon_image_alt() const throw() {
  return _icon_image_alt;
}

/**
 *  Get notes.
 *
 *  @return The notes.
 */
std::string const& hostextinfo::notes() const throw() {
  return _notes;
}

/**
 *  Get notes_url.
 *
 *  @return The notes_url.
 */
std::string const& hostextinfo::notes_url() const throw() {
  return _notes_url;
}

/**
 *  Get statusmap_image.
 *
 *  @return The statusmap_image.
 */
std::string const& hostextinfo::statusmap_image() const throw() {
  return _statusmap_image;
}

/**
 *  Get vrml_image.
 *
 *  @return The vrml_image.
 */
std::string const& hostextinfo::vrml_image() const throw() {
  return _vrml_image;
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
  return true;
}

/**
 *  Set coords_2s value.
 *
 *  @param[in] value The new coords_2d value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_coords_2d(std::string const& value) {
  std::list<std::string> coords;
  string::split(value, coords, ',');
  if (coords.size() != 2)
    return false;

  int x;
  if (!string::to(string::trim(coords.front()).c_str(), x))
    return false;
  coords.pop_front();

  int y;
  if (!string::to(string::trim(coords.front()).c_str(), y))
    return false;

  _coords_2d = point_2d(x, y);
  return true;
}

/**
 *  Set coords_3d value.
 *
 *  @param[in] value The new coords_3d value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_coords_3d(std::string const& value) {
  std::list<std::string> coords;
  string::split(value, coords, ',');
  if (coords.size() != 3)
    return false;

  double x;
  if (!string::to(string::trim(coords.front()).c_str(), x))
    return false;
  coords.pop_front();

  double y;
  if (!string::to(string::trim(coords.front()).c_str(), y))
    return false;
  coords.pop_front();

  double z;
  if (!string::to(string::trim(coords.front()).c_str(), z))
    return false;

  _coords_3d = point_3d(x, y, z);
  return true;
}

/**
 *  Set hostgroups value.
 *
 *  @param[in] value The new hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_hostgroups(std::string const& value) {
  _hostgroups = value;
  return true;
}

/**
 *  Set hosts value.
 *
 *  @param[in] value The new hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool hostextinfo::_set_hosts(std::string const& value) {
  _hosts = value;
  return true;
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
  return true;
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
  return true;
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
  return true;
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
  return true;
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
  return true;
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
  return true;
}
