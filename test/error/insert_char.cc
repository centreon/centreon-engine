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

#include <QCoreApplication>
#include <string.h>
#include "error.hh"

using namespace com::centreon::engine;

/**
 *  Check that chars can be properly inserted in error.
 *
 *  @return 0 on success.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  // Insert chars.
  error e;
  e << 'f' << 'o' << 'o';
  e << ' ';
  e << 'b' << 'a' << 'r';

  // Check insertion.
  return (strcmp("foo bar", e.what()));
}
