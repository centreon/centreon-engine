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
#include <QVector>
#include <vector>
#include "error.hh"
#include "commands/connector/version_response.hh"

using namespace com::centreon::engine::commands::connector;

/**
 *  Default constructor.
 *
 *  @param[in] major The minimum major version was supported by the connector.
 *  @param[in] minor The minimum minor version was supported by the connector.
 */
version_response::version_response(unsigned int major, unsigned int minor)
  : request(request::version_r), _major(major), _minor(minor) {

}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
version_response::version_response(version_response const& right)
  : request(right) {
  operator=(right);
}

/**
 *  Default destructor.
 */
version_response::~version_response() throw() {

}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 *
 *  @return This object.
 */
version_response& version_response::operator=(version_response const& right) {
  if (this != &right) {
    request::operator=(right);
    _major = right._major;
    _minor = right._minor;
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
bool version_response::operator==(version_response const& right) const throw() {
  return (request::operator==(right) == true
	  && _major == right._major
	  && _minor == right._minor);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool version_response::operator!=(version_response const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
request* version_response::clone() const {
  return (new version_response(*this));
}

/**
 *  Create the data with the request information.
 *
 *  @return The data request.
 */
QByteArray version_response::build() {
  return (QByteArray().setNum(_id) + '\0' +
	  QByteArray().setNum(_major) + '\0' +
	  QByteArray().setNum(_minor) +
	  cmd_ending());
}

/**
 *  Restore object with the data information.
 *
 *  @param[in] data The data of the request information.
 */
void version_response::restore(QByteArray const& data) {
  std::vector<QByteArray> list = data.split('\0').toVector().toStdVector();
  if (list.size() != 3) {
    throw (engine_error() << "bad request argument.");
  }

  bool ok;
  int id = list[0].toInt(&ok);
  if (ok == false || id < 0 || id != _id) {
    throw (engine_error() << "bad request id.");
  }

  _major = list[1].toUInt(&ok);
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid major.");
  }

  _minor = list[2].toUInt(&ok);
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid minor.");
  }
}

/**
 *  Get the minimum major version require.
 *
 *  @return The major version.
 */
unsigned int version_response::get_major() const throw() {
  return (_major);
}

/**
 *  Get the minimum minor version require.
 *
 *  @return The minor version.
 */
unsigned int version_response::get_minor() const throw() {
  return (_minor);
}
