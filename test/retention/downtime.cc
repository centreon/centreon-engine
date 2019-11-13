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

#include "com/centreon/engine/retention/downtime.hh"
#include <ctime>
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check downtime.
 *
 *  @param[in] argc Size of argv array.
 *  @param[in] argv Argumments array.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  retention::downtime ref(retention::downtime::host);
  ref.set("author", "author");
  ref.set("comment", "comment");
  ref.set("downtime_id", "1");
  ref.set("duration", "2");
  ref.set("end_time", "1300002");
  ref.set("entry_time", "1300000");
  ref.set("fixed", "1");
  ref.set("host_name", "host_name");
  ref.set("service_description", "service_description");
  ref.set("start_time", "1300000");
  ref.set("triggered_by", "1");

  // Check copy constructor.
  {
    retention::downtime copy(ref);
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy constructor or operator "
                              "equal failed");
  }

  // Check copy operator.
  {
    retention::downtime copy(retention::downtime::service);
    copy = ref;
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy operator or operator "
                              "equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::downtime empty(retention::downtime::host);
    if (empty == ref || !(empty != ref))
      throw(engine_error() << "operator equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::downtime diff(ref);
    diff.set("author", "new_author");
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
