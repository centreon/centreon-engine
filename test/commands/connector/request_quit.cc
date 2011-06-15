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
#include "commands/connector/quit_query.hh"
#include "commands/connector/quit_response.hh"

using namespace com::centreon::engine::commands::connector;

#define QUERY         "4\0\0\0\0"
#define RESPONSE      "5\0\0\0\0"
#define BAD           ".\0\0\0\0"
#define REQUEST(data) QByteArray((data), sizeof((data)) - 1)

const int end_size = request::cmd_ending().size();

bool check_valid(request* req, QByteArray request_data) {
  QByteArray data = req->build();
  if (data != request_data) {
    return (false);
  }
  req->restore(data.remove(data.size() - end_size, end_size));
  return (true);
}

bool check_invalid(request* req, QByteArray request_data) {
  try {
    req->restore(request_data.remove(request_data.size() - end_size, end_size));
  }
  catch (...) {
    return (true);
  }
  return (false);
}

int main(int argc, char** argv) {
  try {
    QCoreApplication app(argc, argv);

    quit_query query;
    if (check_valid(&query, REQUEST(QUERY)) == false) {
      qDebug() << "error: quit_query::check_valid failed.";
      return (1);
    }
    if (check_invalid(&query, REQUEST(BAD)) == false) {
      qDebug() << "error: quit_query::check_invalid failed.";
      return (1);
    }

    quit_response response;
    if (check_valid(&response, REQUEST(RESPONSE)) == false) {
      qDebug() << "error: quit_response::check_valid failed.";
      return (1);
    }
    if (check_invalid(&response, REQUEST(BAD)) == false) {
      qDebug() << "error: quit_response::check_invalid failed.";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
