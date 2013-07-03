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

#include "com/centreon/engine/configuration/object_info.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
object_info::object_info() {

}

/**
 *  Constructor.
 *
 *  @param[in]
 *  @param[in]
 *  @param[in]
 */
object_info::object_info(
  object_ptr obj,
  std::string const& path,
  unsigned int line)
  : _line(line),
    _obj(obj),
    _path(path) {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
object_info::object_info(object_info const& right) {
  operator=(right);
}

/**
 *  Destructor.
 */
object_info::~object_info() throw () {

}

/**
 *  Copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
object_info& object_info::operator=(object_info const& right) {
  if (this != &right) {
    _line = right._line;
    _obj = right._obj;
    _path = right._path;
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
bool object_info::operator==(object_info const& right) const throw () {
  return (_line == right._line
          && _path == right._path
          /*XXX: && _obj == right._obj*/);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool object_info::operator!=(object_info const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Get line.
 *
 *  @return The line number.
 */
unsigned int object_info::line() const throw () {
  return (_line);
}

/**
 *  Set line.
 *
 *  @param[in] line The new line.
 */
void object_info::line(unsigned int line) throw () {
  _line = line;
}

/**
 *  Get object.
 *
 *  @return The object.
 */
object_ptr object_info::object() const throw () {
  return (_obj);
}

/**
 *  Set object.
 *
 *  @param[in] obj The new object.
 */
void object_info::object(object_ptr obj) {
  _obj = obj;
}

/**
 *  Get path.
 *
 *  @return The path.
 */
std::string const& object_info::path() const throw () {
  return (_path);
}

/**
 *  Set path.
 *
 *  @param[in] path The new path.
 */
void object_info::path(std::string const& path) {
  _path = path;
}
