/*
** Copyright 2011 Merethis
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
# define TEST_COMMANDS_CONNECTOR_CHECK_REQUEST_HH

# include <QByteArray>
# include "commands/connector/request.hh"

# define CMD_END       "\0\0\0\0"
# define TOSTR_(x)     #x
# define TOSTR(x)      TOSTR_(x)
# define REQUEST(data) QByteArray((data), sizeof((data)) - 1)

const int end_size = com::centreon::engine::commands::connector::request::cmd_ending().size();

bool check_request_valid(com::centreon::engine::commands::connector::request* req,
			 QByteArray request_data) {
  QByteArray data = req->build();
  if (data != request_data) {
    return (false);
  }
  req->restore(data.remove(data.size() - end_size, end_size));
  return (true);
}

bool check_request_invalid(com::centreon::engine::commands::connector::request* req) {
  try {
    QByteArray data = REQUEST(".\0\0\0\0");
    req->restore(data.remove(data.size() - end_size, end_size));
  }
  catch (...) {
    return (true);
  }
  return (false);
}

template<class T> bool check_request_clone(T* req) {
  T* clone = static_cast<T*>(req->clone());
  bool ret = (*req == *clone);
  delete clone;
  return (ret);
}

#endif // !TEST_COMMANDS_CONNECTOR_CHECK_REQUEST_HH
