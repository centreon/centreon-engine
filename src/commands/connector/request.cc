/*
** Copyright 2011      Merethis
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

#include "commands/connector/request.hh"

using namespace com::centreon::engine::commands::connector;

/**
 *  Default constructor.
 */
request::request(e_type id)
  : _id(id) {

}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
request::request(request const& right) {
  operator=(right);
}

/**
 *  Default destructor.
 */
request::~request() throw() {

}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 *
 *  @return This object.
 */
request& request::operator=(request const& right) {
  if (this != &right) {
    _id = right._id;
  }
  return (*this);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool request::operator==(request const& right) const throw() {
  return (_id == right._id);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool request::operator!=(request const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get the data value of the command's ending.
 *
 *  @return The data value of the command's ending.
 */
std::string const& request::cmd_ending() throw() {
  static std::string ending(4, '\0');
  return (ending);
}

/**
 *  Get the type id of the request.
 *
 *  @return The type id of the request.
 */
request::e_type request::get_id() const throw() {
  return (_id);
}
