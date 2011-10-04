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

#include <time.h>
#include "logging/logger.hh"

using namespace com::centreon::engine::logging;

/**************************************
 *                                     *
 *           Static Objects            *
 *                                     *
 **************************************/

logger::redirector const logger::_redir_nothing = {
  &logger::_nothing<QString const&>,
  &logger::_nothing<std::string const&>,
  &logger::_nothing<char const*>,
  &logger::_nothing<char>,
  &logger::_nothing<int>,
  &logger::_nothing<unsigned int>,
  &logger::_nothing<long>,
  &logger::_nothing<unsigned long>,
  &logger::_nothing<long long>,
  &logger::_nothing<unsigned long long>,
  &logger::_nothing<double>,
  &logger::_nothing<e_flags>,
  &logger::_builder<s_setprecision const&>,
  &logger::_builder<void const*>,
  &logger::_nothing,
  &logger::_nothing
};

logger::redirector const logger::_redir_builder = {
  &logger::_builder<QString const&>,
  &logger::_builder<std::string const&>,
  &logger::_builder<char const*>,
  &logger::_builder<char>,
  &logger::_builder<int>,
  &logger::_builder<unsigned int>,
  &logger::_builder<long>,
  &logger::_builder<unsigned long>,
  &logger::_builder<long long>,
  &logger::_builder<unsigned long long>,
  &logger::_builder<double>,
  &logger::_builder<e_flags>,
  &logger::_builder<s_setprecision const&>,
  &logger::_builder<void const*>,
  &logger::_init,
  &logger::_flush
};

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
  : _redirector(NULL),
    _engine(engine::instance()),
    _type(type),
    _verbosity(verbosity) {
  if (_engine.is_logged(type, verbosity))
    _redirector = &_redir_builder;
  else
    _redirector = &_redir_nothing;
  (this->*(_redirector->init))();
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
logger::logger(logger const& right)
  : _engine(engine::instance()) {
  operator=(right);
}

/**
 *  Default destructor.
 */
logger::~logger() {
  (this->*(_redirector->flush))();
}

logger& logger::operator=(logger const& right) {
  if (this != &right) {
    _type = right._type;
    _verbosity = right._verbosity;
    _redirector = right._redirector;
    right._buffer.flush();
    _buffer.str(right._buffer.str());
  }
  return (*this);
}

logger& logger::operator<<(QString const& obj) {
  return ((this->*(_redirector->redirect_qstring))(obj));
}

logger& logger::operator<<(std::string const& obj) {
  return ((this->*(_redirector->redirect_std_string))(obj));
}

logger& logger::operator<<(char const* obj) {
  return ((this->*(_redirector->redirect_string))(obj));
}

logger& logger::operator<<(char obj) {
  return ((this->*(_redirector->redirect_char))(obj));
}

logger& logger::operator<<(int obj) {
  return ((this->*(_redirector->redirect_int))(obj));
}

logger& logger::operator<<(unsigned int obj) {
  return ((this->*(_redirector->redirect_uint))(obj));
}

logger& logger::operator<<(long obj) {
  return ((this->*(_redirector->redirect_long))(obj));
}

logger& logger::operator<<(unsigned long obj) {
  return ((this->*(_redirector->redirect_ulong))(obj));
}

logger& logger::operator<<(long long obj) {
  return ((this->*(_redirector->redirect_long_long))(obj));
}

logger& logger::operator<<(unsigned long long obj) {
  return ((this->*(_redirector->redirect_ulong_long))(obj));
}

logger& logger::operator<<(double obj) {
  return ((this->*(_redirector->redirect_double))(obj));
}

logger& logger::operator<<(e_flags obj) {
  return ((this->*(_redirector->redirect_flags))(obj));
}

logger& logger::operator<<(s_setprecision const& obj) {
  return ((this->*(_redirector->redirect_setprecision))(obj));
}

logger& logger::operator<<(void const* obj) {
  return ((this->*(_redirector->redirect_void_ptr))(obj));
}

void logger::_init() {
  _buffer << "[" << time(NULL) << "] ";
}

void logger::_flush() {
  _buffer << "\n";
  engine::instance().log(_buffer.str().c_str(), _type, _verbosity);
}

void logger::_nothing() {

}
