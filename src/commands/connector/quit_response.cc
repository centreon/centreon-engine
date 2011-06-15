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

#include <QStringList>
#include "error.hh"
#include "commands/connector/quit_response.hh"

using namespace com::centreon::engine::commands::connector;

quit_response::quit_response()
  : request(request::quit_r) {

}

quit_response::quit_response(quit_response const& right)
  : request(right) {

}

quit_response::~quit_response() throw() {

}

quit_response& quit_response::operator=(quit_response const& right) {
  request::operator=(right);
  return (*this);
}

request* quit_response::clone() const {
  return (new quit_response(*this));
}

QByteArray quit_response::build() {
  return (QByteArray().setNum(_id) + cmd_ending());
}

void quit_response::restore(QByteArray const& data) {
  QList<QByteArray> list = data.split('\0');
  if (list.size() != 1) {
    throw (engine_error() << "bad request argument.");
  }

  bool ok;
  unsigned int id = list[0].toUInt(&ok);
  if (ok == false || id != _id) {
    throw (engine_error() << "bad request id.");
  }
}
