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
#include "commands/connector/error_response.hh"
#include "test/commands/connector/check_request.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands::connector;

#define MESSAGE  "warning"
#define CODE     1
#define RESPONSE "6\0" TOSTR(CODE) "\0" MESSAGE CMD_END

/**
 *  Check the error request.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    testing init;

    error_response response(MESSAGE, static_cast<error_response::e_code>(CODE));
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
