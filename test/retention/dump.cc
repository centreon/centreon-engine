/*
** Copyright 2011-2013 Merethis
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

#include <sstream>
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::retention;

/**
 *  Check objects dump.
 *
 *  @param[in] argc Size of argv array.
 *  @param[in] argv Argumments array.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  if (argc != 2)
    throw (engine_error() << "usage: " << argv[0] << " object_name");

  std::ostringstream oss;
  std::string const type(argv[1]);

  if (type == "comment")
    dump::comments(oss);
  else if (type == "contact")
    dump::contacts(oss);
  else if (type == "downtime")
    dump::downtimes(oss);
  else if (type == "header")
    dump::header(oss);
  else if (type == "host")
    dump::hosts(oss);
  else if (type == "info")
    dump::info(oss);
  else if (type == "program")
    dump::program(oss);
  else if (type == "service")
    dump::services(oss);

  std::cout << oss.str();
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
