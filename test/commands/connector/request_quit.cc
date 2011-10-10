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
#include "test/testing.hh"
#include "commands/connector/quit_query.hh"
#include "commands/connector/quit_response.hh"
#include "test/commands/connector/check_request.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands::connector;

#define QUERY         "4" CMD_END
#define RESPONSE      "5" CMD_END

/**
 *  Check the quit request.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    testing init;

    quit_query query;
    if (check_request_valid(&query, REQUEST(QUERY)) == false) {
      qDebug() << "error: query is valid failed.";
      return (1);
    }
    if (check_request_invalid(&query) == false) {
      qDebug() << "error: query is invalid failed.";
      return (1);
    }
    if (check_request_clone(&query) == false) {
      qDebug() << "error: query clone failed";
      return (1);
    }

    quit_response response;
    if (check_request_valid(&response, REQUEST(RESPONSE)) == false) {
      qDebug() << "error: response is valid failed.";
      return (1);
    }
    if (check_request_invalid(&response) == false) {
      qDebug() << "error: response is invalid failed.";
      return (1);
    }
    if (check_request_clone(&response) == false) {
      qDebug() << "error: response clone failed";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
