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

#include <time.h>
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
  : _type(type), _verbosity(verbosity) {
  _buffer << "[" << time(NULL) << "] ";
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
logger::logger(logger const& right) {
  operator=(right);
}

/**
 *  Default destructor.
 */
logger::~logger() {
  std::string buf = _trim(_buffer.str()) + "\n";
  engine::instance().log(buf.c_str(), _type, _verbosity);
}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 */
logger& logger::operator=(logger const& right) {
  if (this != &right) {
    _buffer << right._buffer.str();
  }
  return (*this);
}

/**
 *  Trim a string.
 *
 *  @param[in] str The string.
 *
 *  @return The trimming stream.
 */
std::string logger::_trim(std::string str) throw() {
  const char* whitespaces = " \t\r\n";
  size_t pos = str.find_last_not_of(whitespaces);

  if (pos == std::string::npos)
    str.clear();
  else
    {
      str.erase(pos + 1);
      if ((pos = str.find_first_not_of(whitespaces)) != std::string::npos)
        str.erase(0, pos);
    }
  return (str);
}
