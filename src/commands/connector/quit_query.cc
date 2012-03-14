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

#include <QStringList>
#include "com/centreon/engine/commands/connector/quit_query.hh"
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine::commands::connector;

/**
 *  Default constructor.
 */
quit_query::quit_query() : request(request::quit_q) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The class to copy.
 */
quit_query::quit_query(quit_query const& right)
  : request(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
quit_query::~quit_query() throw () {}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 *
 *  @return This object.
 */
quit_query& quit_query::operator=(quit_query const& right) {
  request::operator=(right);
  return (*this);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool quit_query::operator==(quit_query const& right) const throw() {
  return (request::operator==(right));
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool quit_query::operator!=(quit_query const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
request* quit_query::clone() const {
  return (new quit_query(*this));
}

/**
 *  Create the data with the request information.
 *
 *  @return The data request.
 */
QByteArray quit_query::build() {
  return (QByteArray().setNum(_id) + cmd_ending());
}

/**
 *  Restore object with the data information.
 *
 *  @param[in] data The data of the request information.
 */
void quit_query::restore(QByteArray const& data) {
  QList<QByteArray> list = data.split('\0');
  if (list.size() != 1) {
    throw (engine_error() << "bad request argument.");
  }

  bool ok;
  int id = list[0].toInt(&ok);
  if (ok == false || id < 0 || id != _id) {
    throw (engine_error() << "bad request id.");
  }
}


