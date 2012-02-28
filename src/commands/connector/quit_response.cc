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

#include <sstream>
#include <vector>
#include "error.hh"
#include "commands/connector/quit_response.hh"

using namespace com::centreon::engine::commands::connector;

/**
 *  Default constructor.
 */
quit_response::quit_response()
  : request(request::quit_r) {

}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
quit_response::quit_response(quit_response const& right)
  : request(right) {

}

/**
 *  Default destructor.
 */
quit_response::~quit_response() throw() {

}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 *
 *  @return This object.
 */
quit_response& quit_response::operator=(quit_response const& right) {
  request::operator=(right);
  return (*this);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool quit_response::operator==(quit_response const& right) const throw() {
  return (request::operator==(right));
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool quit_response::operator!=(quit_response const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
request* quit_response::clone() const {
  return (new quit_response(*this));
}

/**
 *  Create the data with the request information.
 *
 *  @return The data request.
 */
std::string quit_response::build() {
  std::ostringstream oss;
  oss << _id;
  oss.write(cmd_ending().c_str(), cmd_ending().size());
  return (oss.str());
}

/**
 *  Restore object with the data information.
 *
 *  @param[in] data The data of the request information.
 */
void quit_response::restore(std::string const& data) {
  std::vector<std::string> list;
  size_t last(0);
  size_t pos(data.find('\0', last));
  while (pos != std::string::npos) {
    list.push_back(data.substr(last, pos - last));
    last = pos + 1;
    pos = data.find('\0', last);
  }
  if (last != data.size())
    list.push_back(data.substr(last));

  if (list.size() != 1)
    throw (engine_error() << "bad request argument.");

  int id(0);
  std::istringstream iss(list[0]);
  if ((!(iss >> id) || !iss.eof()) || id < 0 || id != _id)
    throw (engine_error() << "bad request id.");
}
