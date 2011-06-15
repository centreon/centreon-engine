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
#include "commands/connector/quit_query.hh"

using namespace com::centreon::engine::commands::connector;

quit_query::quit_query()
  : request(request::quit_q) {

}

quit_query::quit_query(quit_query const& right)
  : request(right) {
  operator=(right);
}

quit_query::~quit_query() throw() {

}

quit_query& quit_query::operator=(quit_query const& right) {
  request::operator=(right);
  return (*this);
}

request* quit_query::clone() const {
  return (new quit_query(*this));
}

QByteArray quit_query::build() {
  return (QByteArray().setNum(_id) + cmd_ending());
}

void quit_query::restore(QByteArray const& data) {
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


