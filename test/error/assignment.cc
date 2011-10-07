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
#include <string>
#include "error.hh"

using namespace com::centreon::engine;

/**
 *  Check that the assignment operator works properly on error.
 *
 *  @return 0 on success.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  // Base object.
  error e1;
  e1 << "foo " << "bar" << 42u;
  e1.set_fatal(false);

  // Copy object.
  error e2;
  e2 << "baz" << " qux" << 21l;
  e2 = e1;

  // Change object object.
  e1 << 3612;
  e1.set_fatal(true);

  // Check.
  return (strcmp(e1.what(), "foo bar423612")
          || (e1.is_fatal() != true)
          || strcmp(e2.what(), "foo bar42")
          || (e2.is_fatal() != false));
}
