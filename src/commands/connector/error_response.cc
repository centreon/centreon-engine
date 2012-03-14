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
#include "com/centreon/engine/commands/connector/error_response.hh"
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine::commands::connector;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] message The error message.
 *  @param[in] code    The exit code value.
 */
error_response::error_response(QString const& message, e_code code)
  : request(request::error_r), _code(code), _message(message) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
error_response::error_response(error_response const& right)
  : request(right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
error_response::~error_response() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
error_response& error_response::operator=(error_response const& right) {
  if (this != &right) {
    request::operator=(right);
    _internal_copy(right);
  }
  return (*this);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if objects have the same value.
 */
bool error_response::operator==(
                       error_response const& right) const throw () {
  return (request::operator==(right)
          && (_code == right._code)
          && (_message == right._message));
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if objects have different values.
 */
bool error_response::operator!=(
                       error_response const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Create the data with the request information.
 *
 *  @return The data request.
 */
QByteArray error_response::build() {
  return (QByteArray().setNum(_id) + '\0' +
          QByteArray().setNum(_code) + '\0' +
          _message.toAscii() + cmd_ending());
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
 *  Get the code error of connector.
 *
 *  @return The code value.
 */
error_response::e_code error_response::get_code() const throw () {
  return (_code);
}

/**
 *  Get the error message.
 *
 *  @return The error message.
 */
QString const& error_response::get_message() const throw () {
  return (_message);
}

/**
 *  Restore object with the data information.
 *
 *  @param[in] data The data of the request information.
 */
void error_response::restore(QByteArray const& data) {
  QList<QByteArray> list = data.split('\0');
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

  _message = list[2];
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid message.");
  }
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void error_response::_internal_copy(error_response const& right) {
  _code = right._code;
  _message = right._message;
  return ;
}
