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

#include <Qvector>
#include <vector>
#include "error.hh"
#include "commands/connector/version_query.hh"

using namespace com::centreon::engine::commands::connector;

/**
 *  Default constructor.
 */
version_query::version_query()
  : request(request::version_q) {

}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
version_query::version_query(version_query const& right)
  : request(right) {
  operator=(right);
}

/**
 *  Default destructor.
 */
version_query::~version_query() throw() {

}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 *
 *  @return This object.
 */
version_query& version_query::operator=(version_query const& right) {
  if (this != &right) {
    request::operator=(right);
  }
  return (*this);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool version_query::operator==(version_query const& right) const throw() {
  return (request::operator==(right));
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool version_query::operator!=(version_query const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
request* version_query::clone() const {
  return (new version_query(*this));
}

/**
 *  Create the data with the request information.
 *
 *  @return The data request.
 */
QByteArray version_query::build() {
  return (QByteArray().setNum(_id) + cmd_ending());
}

/**
 *  Restore object with the data information.
 *
 *  @param[in] data The data of the request information.
 */
void version_query::restore(QByteArray const& data) {
  std::vector<QByteArray> list = data.split('\0').toVector().toStdVector();
  if (list.size() != 1) {
    throw (engine_error() << "bad request argument.");
  }

  bool ok;
  int id = list[0].toInt(&ok);
  if (ok == false || id < 0 || id != _id) {
    throw (engine_error() << "bad request id.");
  }
}


