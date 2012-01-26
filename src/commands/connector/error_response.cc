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

#include <QVector>
#include <vector>
#include "error.hh"
#include "commands/connector/error_response.hh"

using namespace com::centreon::engine::commands::connector;

/**
 *  Default constructor.
 *
 *  @param[in] message   The error message.
 *  @param[in] code The exit code value.
 */
error_response::error_response(std::string const& message, e_code code)
  : request(request::error_r), _message(message), _code(code) {

}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
error_response::error_response(error_response const& right)
  : request(right) {
  operator=(right);
}

/**
 *  Default destructor.
 */
error_response::~error_response() throw() {

}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 *
 *  @return This object.
 */
error_response& error_response::operator=(error_response const& right) {
  if (this != &right) {
    request::operator=(right);
    _message = right._message;
    _code = right._code;
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
bool error_response::operator==(error_response const& right) const throw() {
  return (request::operator==(right) == true
	  && _message == right._message
	  && _code == right._code);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool error_response::operator!=(error_response const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
request* error_response::clone() const {
  return (new error_response(*this));
}

/**
 *  Create the data with the request information.
 *
 *  @return The data request.
 */
QByteArray error_response::build() {
  return (QByteArray().setNum(_id) + '\0' +
	  QByteArray().setNum(_code) + '\0' +
	  _message.c_str() + cmd_ending());
}

/**
 *  Restore object with the data information.
 *
 *  @param[in] data The data of the request information.
 */
void error_response::restore(QByteArray const& data) {
  std::vector<QByteArray> list = data.split('\0').toVector().toStdVector();
  if (list.size() != 3) {
    throw (engine_error() << "bad request argument.");
  }

  bool ok;
  int id = list[0].toInt(&ok);
  if (ok == false || id < 0 || id != _id) {
    throw (engine_error() << "bad request id.");
  }

  unsigned int code = list[1].toUInt(&ok);
  if (ok == false || code > error) {
    throw (engine_error() << "bad request argument, invalid code.");
  }

  _code = static_cast<e_code>(code);

  _message = list[2].constData();
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid message.");
  }
}

/**
 *  Get the error message.
 *
 *  @return The error message.
 */
std::string const& error_response::get_message() const throw() {
  return (_message);
}

/**
 *  Get the code error of connector.
 *
 *  @return The code value.
 */
error_response::e_code error_response::get_code() const throw() {
  return (_code);
}
