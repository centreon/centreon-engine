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

using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<serviceextinfo, type, &serviceextinfo::method>::generic

static struct {
  std::string const name;
  bool (*func)(serviceextinfo&, std::string const&);
} gl_setters[] = {
  { "host_name",           SETTER(std::string const&, _set_hosts) },
  { "hostgroup",           SETTER(std::string const&, _set_hostgroups) },
  { "hostgroup_name",      SETTER(std::string const&, _set_hostgroups) },
  { "service_description", SETTER(std::string const&, _set_service_description) },
  { "notes",               SETTER(std::string const&, _set_notes) },
  { "notes_url",           SETTER(std::string const&, _set_notes_url) },
  { "action_url",          SETTER(std::string const&, _set_action_url) },
  { "icon_image",          SETTER(std::string const&, _set_icon_image) },
  { "icon_image_alt",      SETTER(std::string const&, _set_icon_image_alt) }
};

/**
 *  Default constructor.
 */
serviceextinfo::serviceextinfo()
  : object(object::serviceextinfo, "serviceextinfo") {

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
    _action_url = right._action_url;
    _icon_image = right._icon_image;
    _icon_image_alt = right._icon_image_alt;
    _hosts = right._hosts;
    _hostgroups = right._hostgroups;
    _notes = right._notes;
    _notes_url = right._notes_url;
    _service_description = right._service_description;
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
  return (object::operator==(right)
          && _action_url == right._action_url
          && _icon_image == right._icon_image
          && _icon_image_alt == right._icon_image_alt
          && _hosts == right._hosts
          && _hostgroups == right._hostgroups
          && _notes == right._notes
          && _notes_url == right._notes_url
          && _service_description == right._service_description);
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
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t serviceextinfo::id() const throw () {
  if (!_id) {
    _hash(_id, _hosts.get());
    _hash(_id, _service_description);
  }
  return (_id);
}

/**
 *  Merge object.
 *
  *  @param[in] obj The object to merge.
 */
void serviceextinfo::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  serviceextinfo const& tmpl(static_cast<serviceextinfo const&>(obj));

  MRG_DEFAULT(_action_url);
  MRG_DEFAULT(_icon_image);
  MRG_DEFAULT(_icon_image_alt);
  MRG_INHERIT(_hosts);
  MRG_INHERIT(_hostgroups);
  MRG_DEFAULT(_notes);
  MRG_DEFAULT(_notes_url);
  MRG_DEFAULT(_service_description);
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
  return (false);
}

/**
 *  Set action_url value.
 *
 *  @param[in] value The new action_url value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceextinfo::_set_action_url(std::string const& value) {
  _action_url = value;
  return (true);
}

/**
 *  Set icon_image value.
 *
 *  @param[in] value The new icon_image value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceextinfo::_set_icon_image(std::string const& value) {
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
bool serviceextinfo::_set_icon_image_alt(std::string const& value) {
  _icon_image_alt = value;
  return (true);
}

/**
 *  Set hosts value.
 *
 *  @param[in] value The new hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceextinfo::_set_hosts(std::string const& value) {
  _hosts.set(value);
  _id = 0;
  return (true);
}

/**
 *  Set hostgroups value.
 *
 *  @param[in] value The new hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceextinfo::_set_hostgroups(std::string const& value) {
  _hostgroups.set(value);
  return (true);
}

/**
 *  Set notes value.
 *
 *  @param[in] value The new notes value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceextinfo::_set_notes(std::string const& value) {
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
bool serviceextinfo::_set_notes_url(std::string const& value) {
  _notes_url = value;
  return (true);
}

/**
 *  Set service_description value.
 *
 *  @param[in] value The new service_description value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceextinfo::_set_service_description(std::string const& value) {
  _service_description = value;
  _id = 0;
  return (true);
}
