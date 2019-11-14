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

#include "com/centreon/engine/retention/comment.hh"
#include <ctime>
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check comment.
 *
 *  @param[in] argc Size of argv array.
 *  @param[in] argv Argumments array.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  retention::comment ref(retention::comment::host);
  ref.set("author", "author");
  ref.set("comment_data", "comment_data");
  ref.set("comment_id", "1");
  ref.set("entry_time", "1300000");
  ref.set("entry_type", "1");
  ref.set("expire_time", "1300001");
  ref.set("expires", "0");
  ref.set("host_name", "host_name");
  ref.set("persistent", "1");
  ref.set("service_description", "service_description");
  ref.set("source", "1");

  // Check copy constructor.
  {
    retention::comment copy(ref);
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy constructor or operator "
                              "equal failed");
  }

  // Check copy operator.
  {
    retention::comment copy(retention::comment::service);
    copy = ref;
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy operator or operator "
                              "equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::comment empty(retention::comment::host);
    if (empty == ref || !(empty != ref))
      throw(engine_error() << "operator equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::comment diff(ref);
    diff.set("host_name", "new_host_name");
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
