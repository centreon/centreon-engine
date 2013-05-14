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
 *  @param[in] type The object type name.
 */
object::object(std::string const& type)
  : _is_resolve(false),
    _is_template(false),
    _type(type) {

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
    _is_resolve = right._is_resolve;
    _is_template = right._is_template;
    _name = right._name;
    _type = right._type;
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
  return (_name == right._name
          && _type == right._type
          && _is_resolve == right._is_resolve
          && _is_template == right._is_template);
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
 *  @param[in] type The object type.
 *
 *  @return New object.
 */
shared_ptr<object> object::create(std::string const& type) {
  shared_ptr<object> obj;
  if (type == "command")
    obj = shared_ptr<object>(new command());
  else if (type == "connector")
    obj = shared_ptr<object>(new connector());
  else if (type == "contactgroup")
    obj = shared_ptr<object>(new contactgroup());
  else if (type == "contact")
    obj = shared_ptr<object>(new contact());
  else if (type == "hostdependency")
    obj = shared_ptr<object>(new hostdependency());
  else if (type == "hostescalation")
    obj = shared_ptr<object>(new hostescalation());
  else if (type == "hostextinfo")
    obj = shared_ptr<object>(new hostextinfo());
  else if (type == "hostgroup")
    obj = shared_ptr<object>(new hostgroup());
  else if (type == "host")
    obj = shared_ptr<object>(new host());
  else if (type == "servicedependency")
    obj = shared_ptr<object>(new servicedependency());
  else if (type == "serviceescalation")
    obj = shared_ptr<object>(new serviceescalation());
  else if (type == "serviceextinfo")
    obj = shared_ptr<object>(new serviceextinfo());
  else if (type == "servicegroup")
    obj = shared_ptr<object>(new servicegroup());
  else if (type == "service")
    obj = shared_ptr<object>(new service());
  else if (type == "timeperiod")
    obj = shared_ptr<object>(new timeperiod());
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
      throw (engine_error() << "XXX: todo");
    tmpl->second->resolve_template(templates);
    merge(*tmpl->second);
  }
}

/**
 *  Get the object type name.
 *
 *  @return The object type name.
 */
std::string const& object::type() const throw () {
  return (_type);
}

void object::_set_is_template(bool value) {
  _is_template = value;
}

void object::_set_name(std::string const& value) {
  _name = value;
}

void object::_set_templates(std::string const& value) {
  _templates.clear();
  misc::split(value, _templates, ',');
}
