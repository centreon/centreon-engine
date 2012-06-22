/*
** Copyright 2011-2012 Merethis
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

#ifndef TEST_COMMANDS_CONNECTOR_CHECK_REQUEST_HH
#  define TEST_COMMANDS_CONNECTOR_CHECK_REQUEST_HH

#  include <string>
#  include "com/centreon/engine/commands/connector/request.hh"

#  define CMD_END       "\0\0\0\0"
#  define TOSTR_(x)     #x
#  define TOSTR(x)      TOSTR_(x)
#  define REQUEST(data) std::string((data), sizeof((data)) - 1)

const unsigned int end_size(com::centreon::engine::commands::connector::request::cmd_ending().size());

/**
 *  Check if the request is valid.
 *
 *  @param[in] req          The request to check.
 *  @param[in] request_data The valide result to compare the result request.
 *
 *  @return True if the request are ok, false otherwise.
 */
static bool check_request_valid(
              com::centreon::engine::commands::connector::request* req,
              std::string request_data) {
  std::string data(req->build());
  if (data != request_data)
    return (false);
  req->restore(data.erase(data.size() - end_size, end_size));
  return (true);
}

/**
 *  Check if the request is invalid.
 *
 *  @param[in] req The request to check.
 *
 *  @return True if the request failed, false otherwise.
 */
static bool check_request_invalid(com::centreon::engine::commands::connector::request* req) {
  try {
    std::string data(REQUEST(".\0\0\0\0"));
    req->restore(data.erase(data.size() - end_size, end_size));
  }
  catch (...) {
    return (true);
  }
  return (false);
}

/**
 *  Check copy object with clone method.
 *
 *  @param[in] req The request to check.
 *
 *  @return True if copy object succed, false otherwise.
 */
template<class T> bool check_request_clone(T* req) {
  T* clone = static_cast<T*>(req->clone());
  bool ret = (*req == *clone);
  delete clone;
  return (ret);
}

#endif // !TEST_COMMANDS_CONNECTOR_CHECK_REQUEST_HH
