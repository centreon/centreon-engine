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

#include "com/centreon/engine/retention/info.hh"
#include <ctime>
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check info.
 *
 *  @param[in] argc Size of argv array.
 *  @param[in] argv Argumments array.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  retention::info ref;
  ref.set("created", "1300000");
  ref.set("version", "version");
  ref.set("update_available", "update_available");
  ref.set("update_uid", "update_uid");
  ref.set("last_version", "last_version");
  ref.set("new_version", "new_version");

  // Check copy constructor.
  {
    retention::info copy(ref);
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy constructor or operator "
                              "equal failed");
  }

  // Check copy operator.
  {
    retention::info copy;
    copy = ref;
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy operator or operator "
                              "equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::info empty;
    if (empty == ref || !(empty != ref))
      throw(engine_error() << "operator equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::info diff(ref);
    diff.set("created", "1300001");
    if (diff == ref || !(diff != ref))
      throw(engine_error() << "copy operator or operator "
                              "equal failed");
  }
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
