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

#include "com/centreon/engine/retention/program.hh"
#include <ctime>
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check program.
 *
 *  @param[in] argc Size of argv array.
 *  @param[in] argv Argumments array.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  retention::program ref;
  ref.set("active_host_checks_enabled", "1");
  ref.set("active_service_checks_enabled", "0");
  ref.set("check_host_freshness", "1");
  ref.set("check_service_freshness", "0");
  ref.set("enable_event_handlers", "1");
  ref.set("enable_failure_prediction", "0");
  ref.set("enable_flap_detection", "1");
  ref.set("enable_notifications", "0");
  ref.set("global_host_event_handler", "global_host_event_handler");
  ref.set("global_service_event_handler", "global_service_event_handler");
  ref.set("modified_host_attributes", "1");
  ref.set("modified_service_attributes", "2");
  ref.set("next_comment_id", "3");
  ref.set("next_downtime_id", "4");
  ref.set("next_event_id", "5");
  ref.set("next_notification_id", "6");
  ref.set("next_problem_id", "7");
  ref.set("obsess_over_hosts", "1");
  ref.set("obsess_over_services", "0");
  ref.set("passive_host_checks_enabled", "1");
  ref.set("passive_service_checks_enabled", "0");
  ref.set("process_performance_data", "1");

  // Check copy constructor.
  {
    retention::program copy(ref);
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy constructor or operator "
                              "equal failed");
  }

  // Check copy operator.
  {
    retention::program copy;
    copy = ref;
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy operator or operator "
                              "equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::program empty;
    if (empty == ref || !(empty != ref))
      throw(engine_error() << "operator equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::program diff(ref);
    diff.set("active_host_checks_enabled", "0");
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
