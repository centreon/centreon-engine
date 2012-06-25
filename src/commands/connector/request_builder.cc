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

#include <sstream>
#include "com/centreon/engine/commands/connector/error_response.hh"
#include "com/centreon/engine/commands/connector/execute_query.hh"
#include "com/centreon/engine/commands/connector/execute_response.hh"
#include "com/centreon/engine/commands/connector/quit_query.hh"
#include "com/centreon/engine/commands/connector/quit_response.hh"
#include "com/centreon/engine/commands/connector/request_builder.hh"
#include "com/centreon/engine/commands/connector/version_query.hh"
#include "com/centreon/engine/commands/connector/version_response.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine::commands::connector;

/**
 *  Get instance of the request builder singleton.
 *
 *  @return This singleton.
 */
request_builder& request_builder::instance() {
  static request_builder instance;
  return (instance);
}

/**
 *  Get a request object.
 *
 *  @return The request object build with data.
 */
shared_ptr<request> request_builder::build(std::string const& data) const {
  unsigned int req_id(0);
  std::istringstream iss(data.substr(0, data.find('\0')));
  if (!(iss >> req_id) || !iss.eof())
    throw (engine_error() << "bad request id.");

  std::map<unsigned int, shared_ptr<request> >::const_iterator
    it(_list.find(req_id));
  if (it == _list.end()) {
    throw (engine_error() << "bad request id.");
  }

  shared_ptr<request> ret(it->second->clone());
  ret->restore(data);
  return (ret);
}

/**
 *  Default constructor.
 */
request_builder::request_builder() {
  shared_ptr<request> req(new version_query());
  _list.insert(std::pair<unsigned int, shared_ptr<request> >(
                      req->get_id(),
                      req));
  req = shared_ptr<request>(new version_response());
  _list.insert(std::pair<unsigned int, shared_ptr<request> >(
                      req->get_id(),
                      req));
  req = shared_ptr<request>(new execute_query());
  _list.insert(std::pair<unsigned int, shared_ptr<request> >(
                      req->get_id(),
                      req));
  req = shared_ptr<request>(new execute_response());
  _list.insert(std::pair<unsigned int, shared_ptr<request> >(
                      req->get_id(),
                      req));
  req = shared_ptr<request>(new quit_query());
  _list.insert(std::pair<unsigned int, shared_ptr<request> >(
                      req->get_id(),
                      req));
  req = shared_ptr<request>(new quit_response());
  _list.insert(std::pair<unsigned int, shared_ptr<request> >(
                      req->get_id(),
                      req));
  req = shared_ptr<request>(new error_response());
  _list.insert(std::pair<unsigned int, shared_ptr<request> >(
                      req->get_id(),
                      req));
}

/**
 *  Destructor.
 */
request_builder::~request_builder() throw () {}
