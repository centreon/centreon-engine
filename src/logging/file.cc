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

#include "logging/file.hh"
#include "error.hh"

using namespace com::centreon::engine::logging;

file::file(char const* file) : _file(file) {
  _file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  if (_file.QFile::error() != QFile::NoError) {
    throw (engine_error() << _file.errorString());
  }
}

file::~file() throw() {
  _file.close();
}

void file::rotate() {

}

void file::log(char const* message,
	       unsigned long long type,
	       unsigned int verbosity) throw() {
  (void)type;
  (void)verbosity;

  _file.write(message);
}
