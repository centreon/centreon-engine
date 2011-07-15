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

#include <QCoreApplication>
#include <QDebug>
#include <exception>
#include "error.hh"
#include "commands/connector/quit_query.hh"
#include "commands/connector/quit_response.hh"
#include "check_request.hh"

using namespace com::centreon::engine::commands::connector;

#define QUERY         "4" CMD_END
#define RESPONSE      "5" CMD_END

/**
 *  Check the quit request.
 */
int main(int argc, char** argv) {
  try {
    QCoreApplication app(argc, argv);

    quit_query query;
    if (check_request_valid(&query, REQUEST(QUERY)) == false)
      throw (engine_error() << "query is valid failed.");
    if (check_request_invalid(&query) == false)
      throw (engine_error() << "query is invalid failed.");
    if (check_request_clone(&query) == false)
      throw (engine_error() << "query clone failed");

    quit_response response;
    if (check_request_valid(&response, REQUEST(RESPONSE)) == false)
      throw (engine_error() << "response is valid failed.");
    if (check_request_invalid(&response) == false)
      throw (engine_error() << "response is invalid failed.");
    if (check_request_clone(&response) == false)
      throw (engine_error() << "response clone failed");
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
