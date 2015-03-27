/*
** Copyright 2011-2013,2015 Merethis
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

#include <ctime>
#include "com/centreon/engine/retention/host.hh"
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check host.
 *
 *  @param[in] argc Size of argv array.
 *  @param[in] argv Argumments array.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  retention::host ref;
  ref.set("active_checks_enabled", "1");
  ref.set("check_command", "check_command");
  ref.set("check_execution_time", "10");
  ref.set("check_latency", "10");
  ref.set("check_options", "1");
  ref.set("check_period", "check_period");
  ref.set("check_type", "1");
  ref.set("current_attempt", "1");
  ref.set("current_event_id", "1");
  ref.set("current_problem_id", "1");
  ref.set("current_state", "1");
  ref.set("event_handler", "event_handler");
  ref.set("event_handler_enabled", "1");
  ref.set("flap_detection_enabled", "1");
  ref.set("has_been_checked", "1");
  ref.set("host_name", "host_name");
  ref.set("is_flapping", "1");
  ref.set("last_check", "1300000");
  ref.set("last_event_id", "1");
  ref.set("last_hard_state", "1300001");
  ref.set("last_hard_state_change", "1300002");
  ref.set("last_problem_id", "2");
  ref.set("last_state", "1300004");
  ref.set("last_state_change", "1300005");
  ref.set("last_time_down", "1300006");
  ref.set("last_time_unreachable", "1300007");
  ref.set("last_time_up", "1300008");
  ref.set("long_plugin_output", "long_plugin_output");
  ref.set("max_attempts", "4");
  ref.set("modified_attributes", "4");
  ref.set("next_check", "1300009");
  ref.set("normal_check_interval", "20");
  ref.set("obsess_over_host", "10");
  ref.set("passive_checks_enabled", "1");
  ref.set("percent_state_change", "44.4");
  ref.set("performance_data", "performance_data");
  ref.set("plugin_output", "plugin_output");
  ref.set("retry_check_interval", "55");
  ref.set("state_history", "0,5,2,6,3,1");
  ref.set("state_type", "1");

  // Check copy constructor.
  {
    retention::host copy(ref);
    if (!(copy == ref) || copy != ref)
      throw (engine_error()
             << "copy constructor or (in)equality operator failed");
  }

  // Check assignment operator.
  {
    retention::host copy;
    copy = ref;
    if (!(copy == ref) || copy != ref)
      throw (engine_error()
             << "assignment operator or (in)equality operator failed");
  }

  // Check operator equal and not equal.
  {
    retention::host empty;
    if (empty == ref || !(empty != ref))
      throw (engine_error() << "(in)equality operator failed");
  }

  // Check operator equal and not equal.
  {
    retention::host diff(ref);
    diff.set("retry_check_interval", "42");
    if (diff == ref || !(diff != ref))
      throw (engine_error() << "(in)equality operator failed");
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
