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
#include "commands/connector/execute_query.hh"
#include "commands/connector/execute_response.hh"
#include "check_request.hh"
#include "engine.hh"

using namespace com::centreon::engine::commands::connector;

#define ID        1
#define TIMEOUT   42
#define TIMESTAMP 1308131877
#define BINARY    "./bin"

#define IS_EXECUTED 1
#define STDERR      ""
#define STDOUT      "is valid request"

#define QUERY    "2\0" TOSTR(ID) "\0" TOSTR(TIMEOUT) "\0" TOSTR(TIMESTAMP) "\0" BINARY CMD_END
#define RESPONSE "3\0" TOSTR(ID) "\0" TOSTR(IS_EXECUTED) "\0" TOSTR(STATE_OK) "\0" TOSTR(TIMESTAMP) "\0" STDERR "\0" STDOUT CMD_END

int main(int argc, char** argv) {
  try {
    QCoreApplication app(argc, argv);

    QDateTime time;
    time.setMSecsSinceEpoch(TIMESTAMP);
    execute_query query(ID, BINARY, time, TIMEOUT);
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

    execute_response response(ID, IS_EXECUTED, STATE_OK, time, STDERR, STDOUT);
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
