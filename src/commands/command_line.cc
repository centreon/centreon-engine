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

#include <string.h>
#include "com/centreon/engine/commands/command_line.hh"
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine::commands;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
command_line::command_line() : _argc(0), _argv(NULL), _size(0) {}

/**
 *  Parse command line.
 *
 *  @param[in] cmdline  The command line to parse.
 */
command_line::command_line(QString const& cmdline)
  : _argc(0), _argv(NULL), _size(0) {
  parse(cmdline.toStdString());
}

/**
 *  Build command line.
 *
 *  @param[in] appname  The application name.
 *  @param[in] args     The array arguments.
 */
command_line::command_line(
                QString const& appname,
                QStringList const& args) {
  std::string cmdline(appname.toStdString());
  for (QStringList::const_iterator it(args.begin()), end(args.end());
       it != end;
       ++it)
    cmdline += " " + (*it).toStdString();
  parse(cmdline);
}

/**
 *  Copy constructor.
 *
 *  @param[in] right  The object to copy.
 */
command_line::command_line(command_line const& right) : _argv(NULL) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
command_line::~command_line() throw () {
  _release();
}

/**
 *  Copy operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
command_line& command_line::operator=(command_line const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right  The object to compare.
 *
 *  @return True if objects are equal, otherwise false.
 */
bool command_line::operator==(command_line const& right) const throw () {
  return (_argc == right._argc
          && _size == right._size
          && !memcmp(_argv[0], right._argv[0], _size));
}

/**
 *  Not equal operator.
 *
 *  @param[in] right  The object to compare.
 *
 *  @return True if objects are not equal, otherwise false.
 */
bool command_line::operator!=(command_line const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Get the size array of arguments.
 *
 *  @return Size.
 */
int command_line::get_argc() const throw () {
  return (_argc);
}

/**
 *  Get the array of arguments.
 *
 *  @return Array arguments.
 */
char** command_line::get_argv() const throw () {
  return (_argv);
}

/**
 *  Parse command line and store arguments.
 *
 *  @param[in] cmdline  The command line to parse.
 */
void command_line::parse(std::string const& cmdline) {
  // Allocate buffer.
  _release();
  char* str(new char[cmdline.size() + 1]);
  _size = 0;
  str[_size] = 0;

  // Status variables.
  bool escap(false);
  char sep(0);
  char last(0);
  for (size_t i(0), end(cmdline.size()); i < end; ++i) {
    // Current processed char.
    char c(cmdline[i]);

    // Is this an escaped character ?
    escap = (last == '\\' ? !escap : false);
    if (escap) {
      switch (c) {
      case 'n':
        c = '\n';
        break ;
      case 'r':
        c = '\r';
        break ;
      case 't':
        c = '\t';
        break ;
      }
    }

    // End of token.
    if (!sep && isblank(c)) {
      if (last && !isblank(last) && str[_size - 1]) {
        str[_size++] = 0;
        ++_argc;
      }
    }
    // Quotes.
    else if (!escap && (c == '\'' || c == '"')) {
      if (!sep)
        sep = c;
      else if (sep == c)
        sep = 0;
      else if ((c != '\\') || escap)
        str[_size++] = c;
    }
    else if ((c != '\\') || escap)
      str[_size++] = c;
    last = c;
  }

  // Not-terminated quote.
  if (sep) {
    delete [] str;
    throw (engine_error() << "missing separator '" << sep << "'");
  }

  // Terminate string if not already done so.
  if (last && _size && str[_size - 1]) {
    str[_size] = 0;
    ++_argc;
  }

  // Put tokens in table.
  _size = 0;
  _argv = new char*[_argc + 1];
  _argv[_argc] = NULL;
  for (int i(0); i < _argc; ++i) {
    _argv[i] = str + _size;
    while (str[_size++]);
  }

  // If no token were found, avoid memory leak.
  if (!_argv[0])
    delete [] str;

  return ;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 */
void command_line::_internal_copy(command_line const& right) {
  if (this != &right) {
    _argc = right._argc;
    _size = right._size;
    _release();
    if (right._argv) {
      _argv = new char*[_argc + 1];
      _argv[0] = new char[_size];
      _argv[_argc] = NULL;
      memcpy(_argv[0], right._argv[0], _size);
      unsigned int pos(0);
      for (int i(0); i < _argc; ++i) {
        _argv[i] = _argv[0] + pos;
        while (_argv[0][pos++]);
      }
    }
  }
  return ;
}

/**
 *  Release memory used.
 */
void command_line::_release() {
  if (_argv)
    delete[] _argv[0];
  delete[] _argv;
  _argv = NULL;
}
