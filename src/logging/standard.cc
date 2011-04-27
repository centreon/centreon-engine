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

#include <stdio.h>
#include <string.h>
#include "logging/standard.hh"

using namespace com::centreon::engine::logging;

standard::standard(bool is_stdout) {
  _file = (is_stdout == true ? stdout : stderr);
}

standard::~standard() throw() {

}

void standard::log(char const* message,
		   unsigned long long type,
		   unsigned int verbosity) throw() {
  (void)type;
  (void)verbosity;

  fwrite(message, strlen(message), 1, _file);
}
