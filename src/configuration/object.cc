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

#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/configuration/connector.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/hostdependency.hh"
#include "com/centreon/engine/configuration/hostescalation.hh"
#include "com/centreon/engine/configuration/hostextinfo.hh"
#include "com/centreon/engine/configuration/hostgroup.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/object.hh"
#include "com/centreon/engine/configuration/servicedependency.hh"
#include "com/centreon/engine/configuration/serviceescalation.hh"
#include "com/centreon/engine/configuration/serviceextinfo.hh"
#include "com/centreon/engine/configuration/servicegroup.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/timeperiod.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<object, type, &object::method>::generic

static struct {
  std::string const name;
  bool (*func)(object&, std::string const&);
} gl_setters[] = {
  { "use",      SETTER(std::string const&, _set_templates) },
  { "name",     SETTER(std::string const&, _set_name) },
  { "register", SETTER(bool, _set_is_template) }
};

/**
 *  Constructor.
 *
 *  @param[in] type      The object type.
 *  @param[in] type_name The object type name.
 */
object::object(object::object_type type, std::string const& type_name)
  : _id(0),
    _is_resolve(false),
    _is_template(false),
    _type(type),
    _type_name(type_name) {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
object::object(object const& right) {
  operator=(right);
}

/**
 *  Destructor.
 */
object::~object() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
object& object::operator=(object const& right) {
  if (this != &right) {
    _id = right._id;
    _is_resolve = right._is_resolve;
    _is_template = right._is_template;
    _name = right._name;
    _templates = right._templates;
    _type = right._type;
    _type_name = right._type_name;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool object::operator==(object const& right) const throw () {
  return (_id == right._id
          && _name == right._name
          && _type == right._type
          && _type_name == right._type_name
          && _is_resolve == right._is_resolve
          && _is_template == right._is_template
          && _templates == right._templates);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool object::operator!=(object const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Create object with object type.
 *
 *  @param[in] type_name The object type name.
 *
 *  @return New object.
 */
shared_ptr<object> object::create(std::string const& type_name) {
  shared_ptr<object> obj;
  if (type_name == "command")
    obj = shared_ptr<object>(new configuration::command());
  else if (type_name == "connector")
    obj = shared_ptr<object>(new configuration::connector());
  else if (type_name == "contactgroup")
    obj = shared_ptr<object>(new configuration::contactgroup());
  else if (type_name == "contact")
    obj = shared_ptr<object>(new configuration::contact());
  else if (type_name == "hostdependency")
    obj = shared_ptr<object>(new configuration::hostdependency());
  else if (type_name == "hostescalation")
    obj = shared_ptr<object>(new configuration::hostescalation());
  else if (type_name == "hostextinfo")
    obj = shared_ptr<object>(new configuration::hostextinfo());
  else if (type_name == "hostgroup")
    obj = shared_ptr<object>(new configuration::hostgroup());
  else if (type_name == "host")
    obj = shared_ptr<object>(new configuration::host());
  else if (type_name == "servicedependency")
    obj = shared_ptr<object>(new configuration::servicedependency());
  else if (type_name == "serviceescalation")
    obj = shared_ptr<object>(new configuration::serviceescalation());
  else if (type_name == "serviceextinfo")
    obj = shared_ptr<object>(new configuration::serviceextinfo());
  else if (type_name == "servicegroup")
    obj = shared_ptr<object>(new configuration::servicegroup());
  else if (type_name == "service")
    obj = shared_ptr<object>(new configuration::service());
  else if (type_name == "timeperiod")
    obj = shared_ptr<object>(new configuration::timeperiod());
  return (obj);
}

/**
 *  Get is this object is a template.
 *
 *  @return True if this object is tempalte.
 */
bool object::is_template() const throw () {
  return (_is_template);
}

/**
 *  Get the object name.
 *
 *  @return The object name.
 */
std::string const& object::name() const throw () {
  return (_name);
}

/**
 *  Parse and set the object property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool object::parse(std::string const& key, std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (false);
}

/**
 *  Parse and set the object property.
 *
 *  @param[in] line The configuration line.
 *
 *  @return True on success, otherwise false.
 */
bool object::parse(std::string const& line) {
  std::size_t pos(line.find_first_of(" \t\r", 0));
  if (pos == std::string::npos)
    return (false);
  std::string key(line.substr(0, pos));
  std::string value(line.substr(pos + 1));
  misc::trim(value);

  if (!object::parse(key, value))
    return (parse(key, value));
  return (true);
}

/**
 *  Resolve template object.
 *
 *  @param[in, out] templates The template list.
 */
void object::resolve_template(
       umap<std::string, shared_ptr<object> >& templates) {
  if (_is_resolve)
    return;

  _is_resolve = true;
  for (std::list<std::string>::const_iterator it(_templates.begin()),
         end(_templates.end());
       it != end;
       ++it) {
    umap<std::string, shared_ptr<object> >::iterator
      tmpl(templates.find(*it));
    if (tmpl == templates.end())
      throw (engine_error() << "merge failed: invalid object type");
    tmpl->second->resolve_template(templates);
    merge(*tmpl->second);
  }
}

/**
 *  Get the object type.
 *
 *  @return The object type.
 */
object::object_type object::type() const throw () {
  return (_type);
}

/**
 *  Get the object type name.
 *
 *  @return The object type name.
 */
std::string const& object::type_name() const throw () {
  return (_type_name);
}

/**
 *  Get the string hash.
 *
 *  @param[in] data The string to hash.
 *
 *  @return The string hash.
 */
std::size_t object::_hash(std::string const& data) throw () {
  std::size_t hash(0);
  _hash(hash, data);
  return (hash);
}

/**
 *  Get the list of string hash.
 *
 *  @param[in,out] hash The hash to fill.
 *  @param[in]     data The data to hash.
 */
void object::_hash(
       std::size_t& hash,
       std::list<std::string> const& data) throw () {
  for (std::list<std::string>::const_iterator
         it(data.begin()), end(data.end());
       it != end;
       ++it)
    _hash(hash, *it);
}

/**
 *  Get the string hash.
 *
 *  @param[in,out] hash The hash to fill.
 *  @param[in]     data The string to hash.
 */
void object::_hash(std::size_t& hash, std::string const& data) throw () {
  for (std::string::const_iterator it(data.begin()), end(data.end());
       it != end;
       ++it) {
    std::size_t val(static_cast<std::size_t>(*it));
    hash ^= val + 0x9e3779b9 + (hash<<6) + (hash>>2);
  }
}

/**
 *  Set is template value.
 *
 *  @param[in] value The new is template value.
 *
 *  @return True on success, otherwise false.
 */
bool object::_set_is_template(bool value) {
  _is_template = value;
  return (true);
}

/**
 *  Set name value.
 *
 *  @param[in] value The new name value.
 *
 *  @return True on success, otherwise false.
 */
bool object::_set_name(std::string const& value) {
  _name = value;
  return (true);
}

/**
 *  Set templates value.
 *
 *  @param[in] value The new templates value.
 *
 *  @return True on success, otherwise false.
 */
bool object::_set_templates(std::string const& value) {
  _templates.clear();
  misc::split(value, _templates, ',');
  return (true);
}
