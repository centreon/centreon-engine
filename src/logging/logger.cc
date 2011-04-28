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

//#include "logging/engine.hh"
#include "logging/logger.hh"

using namespace com::centreon::engine::logging;

logger::logger(unsigned long long type, unsigned int verbosity)
  : _type(type), _verbosity(verbosity) {

}

logger::logger(logger const& right) {
  operator=(right);
}

logger::~logger() throw() {
  //engine::instance().log(_buffer.str(), _type, _verbosity);
}

logger& logger::operator=(logger const& right) {
  if (this != &right) {
    _buffer << right._buffer.str();
  }
  return (*this);
}

template <typename T> logger& logger::operator<<(T obj) {
  _buffer << obj;
  return (*this);
}
