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

#include <QDateTime>
#include "logging/engine.hh"
#include "logging/logger.hh"

using namespace com::centreon::engine::logging;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 *
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 */
logger::logger(unsigned long long type, unsigned int verbosity)
  : _stream(&_buffer), _type(type), _verbosity(verbosity) {
  _stream << "[" << QDateTime::currentMSecsSinceEpoch() << "] ";
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
logger::logger(logger const& right)
  : _stream(&_buffer) {
  operator=(right);
}

/**
 *  Default destructor.
 */
logger::~logger() {
  _stream << flush;
  engine::instance().log(qPrintable(_buffer.trimmed() + "\n"), _type, _verbosity);
}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 *
 *  @return This object.
 */
logger& logger::operator=(logger const& right) {
  if (this != &right) {
    right._stream.flush();
    _stream.flush();
    _buffer = right._buffer;
    _type = right._type;
    _verbosity = right._verbosity;
  }
  return (*this);
}

/**
 *  Append a std:string to the message buffer.
 *
 *  @param[in] str String to append.
 *
 *  @return This object.
 */
logger& logger::operator<<(std::string const& str) {
  _stream << str.c_str();
  return (*this);
}
