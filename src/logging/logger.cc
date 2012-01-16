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
  &logger::_nothing<s_setprecision const&>,
  &logger::_nothing<void const*>,
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


/**
 *  Default copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
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

/**
 *  Add a QString into the logger buffer.
 *
 *  @param[in] obj The QString.
 *
 *  @return This object.
 */
logger& logger::operator<<(QString const& obj) {
  return ((this->*(_redirector->redirect_qstring))(obj));
}

/**
 *  Add a std::string into the logger buffer.
 *
 *  @param[in] obj The std::string.
 *
 *  @return This object.
 */
logger& logger::operator<<(std::string const& obj) {
  return ((this->*(_redirector->redirect_std_string))(obj));
}

/**
 *  Add a string into the logger buffer.
 *
 *  @param[in] obj The string.
 *
 *  @return This object.
 */
logger& logger::operator<<(char const* obj) {
  return ((this->*(_redirector->redirect_string))(obj));
}

/**
 *  Add a char into the logger buffer.
 *
 *  @param[in] obj The char.
 *
 *  @return This object.
 */
logger& logger::operator<<(char obj) {
  return ((this->*(_redirector->redirect_char))(obj));
}

/**
 *  Add an integer into the logger buffer.
 *
 *  @param[in] obj The integer.
 *
 *  @return This object.
 */
logger& logger::operator<<(int obj) {
  return ((this->*(_redirector->redirect_int))(obj));
}

/**
 *  Add a unsigned integer into the logger buffer.
 *
 *  @param[in] obj The unsigned integer.
 *
 *  @return This object.
 */
logger& logger::operator<<(unsigned int obj) {
  return ((this->*(_redirector->redirect_uint))(obj));
}

/**
 *  Add a long integer into the logger buffer.
 *
 *  @param[in] obj The long integer.
 *
 *  @return This object.
 */
logger& logger::operator<<(long obj) {
  return ((this->*(_redirector->redirect_long))(obj));
}

/**
 *  Add a unsigned long integer into the logger buffer.
 *
 *  @param[in] obj The unsigned long integer.
 *
 *  @return This object.
 */
logger& logger::operator<<(unsigned long obj) {
  return ((this->*(_redirector->redirect_ulong))(obj));
}

/**
 *  Add a long long interger into the logger buffer.
 *
 *  @param[in] obj The long long integer.
 *
 *  @return This object.
 */
logger& logger::operator<<(long long obj) {
  return ((this->*(_redirector->redirect_long_long))(obj));
}

/**
 *  Add a unsigned long long integer into the logger buffer.
 *
 *  @param[in] obj The unsigned long long integer.
 *
 *  @return This object.
 */
logger& logger::operator<<(unsigned long long obj) {
  return ((this->*(_redirector->redirect_ulong_long))(obj));
}

/**
 *  Add a double into the logger buffer.
 *
 *  @param[in] obj The double.
 *
 *  @return This object.
 */
logger& logger::operator<<(double obj) {
  return ((this->*(_redirector->redirect_double))(obj));
}

/**
 *  Set the logger flags.
 *
 *  @param[in] obj The flags.
 *
 *  @return This object.
 */
logger& logger::operator<<(e_flags obj) {
  return ((this->*(_redirector->redirect_flags))(obj));
}

/**
 *  Set the logger precision.
 *
 *  @param[in] obj The precision.
 *
 *  @return This object.
 */
logger& logger::operator<<(s_setprecision const& obj) {
  return ((this->*(_redirector->redirect_setprecision))(obj));
}

/**
 *  Add a pointer into the logger buffer.
 *
 *  @param[in] obj The pointer.
 *
 *  @return This object.
 */
logger& logger::operator<<(void const* obj) {
  return ((this->*(_redirector->redirect_void_ptr))(obj));
}

/**
 *  Inisialization of the logger buffer.
 */
void logger::_init() {
  _buffer << "[" << time(NULL) << "] ";
}

/**
 *  Send the content of the logger buffer at the engine.
 */
void logger::_flush() {
  _buffer << "\n";
  engine::instance().log(_buffer.str().c_str(), _type, _verbosity);
}

/**
 *  Do nothing.
 */
void logger::_nothing() {

}

