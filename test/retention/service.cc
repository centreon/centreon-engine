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

#include <ctime>
#include "com/centreon/engine/retention/service.hh"
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check service.
 *
 *  @param[in] argc Size of argv array.
 *  @param[in] argv Argumments array.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  retention::service ref;
  ref.set("acknowledgement_type", "1");
  ref.set("active_checks_enabled", "1");
  ref.set("check_command", "check_command");
  ref.set("check_execution_time", "10");
  ref.set("check_flapping_recovery_notification", "2");
  ref.set("check_latency", "55.3");
  ref.set("check_options", "3");
  ref.set("check_period", "check_period");
  ref.set("check_type", "5");
  ref.set("current_attempt", "33");
  ref.set("current_event_id", "42");
  ref.set("current_notification_id", "24");
  ref.set("current_notification_number", "54");
  ref.set("current_problem_id", "88");
  ref.set("current_state", "2");
  ref.set("event_handler", "event_handler");
  ref.set("event_handler_enabled", "1");
  ref.set("failure_prediction_enabled", "1");
  ref.set("flap_detection_enabled", "1");
  ref.set("has_been_checked", "1");
  ref.set("host_name", "host_name");
  ref.set("is_flapping", "1");
  ref.set("last_check", "1300000");
  ref.set("last_event_id", "66");
  ref.set("last_hard_state", "1300001");
  ref.set("last_hard_state_change", "1300002");
  ref.set("last_notification", "1300003");
  ref.set("last_problem_id", "99");
  ref.set("last_state", "1300004");
  ref.set("last_state_change", "1300005");
  ref.set("last_time_critical", "1300006");
  ref.set("last_time_ok", "1300007");
  ref.set("last_time_unknown", "1300008");
  ref.set("last_time_warning", "1300009");
  ref.set("long_plugin_output", "long_plugin_output");
  ref.set("max_attempts", "13");
  ref.set("modified_attributes", "2");
  ref.set("next_check", "1300010");
  ref.set("normal_check_interval", "31");
  ref.set("notification_period", "notification_period");
  ref.set("notifications_enabled", "1");
  ref.set("notified_on_critical", "1");
  ref.set("notified_on_unknown", "1");
  ref.set("notified_on_warning", "1");
  ref.set("obsess_over_service", "55");
  ref.set("passive_checks_enabled", "1");
  ref.set("percent_state_change", "33.2");
  ref.set("performance_data", "performance_data");
  ref.set("plugin_output", "plugin_output");
  ref.set("problem_has_been_acknowledged", "1");
  ref.set("process_performance_data", "66");
  ref.set("retry_check_interval", "77");
  ref.set("service_description", "service_description");
  ref.set("state_history", "0,1,2,3,4,5,6,7");
  ref.set("state_type", "2");

  // Check copy constructor.
  {
    retention::service copy(ref);
    if (!(copy == ref) || copy != ref)
      throw (engine_error() << "copy constructor or operator "
             "equal failed");
  }

  // Check copy operator.
  {
    retention::service copy;
    copy = ref;
    if (!(copy == ref) || copy != ref)
      throw (engine_error() << "copy operator or operator "
             "equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::service empty;
    if (empty == ref || !(empty != ref))
      throw (engine_error() << "operator equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::service diff(ref);
    diff.set("acknowledgement_type", "0");
    if (diff == ref || !(diff != ref))
      throw (engine_error() << "copy operator or operator "
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
