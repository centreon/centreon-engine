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

#include "com/centreon/engine/retention/object.hh"
#include <array>
#include "com/centreon/engine/retention/comment.hh"
#include "com/centreon/engine/retention/contact.hh"
#include "com/centreon/engine/retention/downtime.hh"
#include "com/centreon/engine/retention/host.hh"
#include "com/centreon/engine/retention/info.hh"
#include "com/centreon/engine/retention/program.hh"
#include "com/centreon/engine/retention/service.hh"

using namespace com::centreon::engine;

/**
 *  Constructor.
 *
 *  @param[in] type The object type.
 */
retention::object::object(type_id type) : _type(type) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
retention::object::object(object const& right) {
  operator=(right);
}

/**
 *  Destructor.
 */
retention::object::~object() throw() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
retention::object& retention::object::operator=(object const& right) {
  if (this != &right) {
    _type = right._type;
  }
  return *this;
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool retention::object::operator==(object const& right) const throw() {
  return _type == right._type;
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool retention::object::operator!=(object const& right) const throw() {
  return !operator==(right);
}

/**
 *  Factory of retention object.
 *
 *  @return A new object.
 */
retention::object_ptr retention::object::create(std::string const& type_name) {
  object_ptr obj;
  if (type_name == "service")
    obj = object_ptr(new retention::service);
  else if (type_name == "host")
    obj = object_ptr(new retention::host);
  else if (type_name == "contact")
    obj = object_ptr(new retention::contact);
  else if (type_name == "hostcomment")
    obj = object_ptr(new retention::comment(comment::host));
  else if (type_name == "servicecomment")
    obj = object_ptr(new retention::comment(comment::service));
  else if (type_name == "hostdowntime")
    obj = object_ptr(new retention::downtime(downtime::host));
  else if (type_name == "servicedowntime")
    obj = object_ptr(new retention::downtime(downtime::service));
  else if (type_name == "info")
    obj = object_ptr(new retention::info);
  else if (type_name == "program")
    obj = object_ptr(new retention::program);
  return obj;
}

/**
 *  Get the object type.
 *
 *  @return The object type.
 */
retention::object::type_id retention::object::type() const throw() {
  return _type;
}

/**
 *  Get the object type name.
 *
 *  @return The object type name.
 */
std::string const& retention::object::type_name() const throw() {
  static std::string const tab[] = {"comment", "contact", "downtime", "host",
                                    "info",    "program", "service"};
  return tab[_type];
}
