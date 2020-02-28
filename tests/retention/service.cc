/*
** Copyright 2011-2019 Centreon
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

#include "com/centreon/engine/retention/service.hh"
#include <gtest/gtest.h>
#include <ctime>
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/retention/dump.hh"

using namespace com::centreon::engine;

class RetentionServiceTest : public ::testing::Test {
 public:
  void SetUp() {
    _ref.set("acknowledgement_type", "1");
    _ref.set("active_checks_enabled", "1");
    _ref.set("check_command", "check_command");
    _ref.set("check_execution_time", "10");
    _ref.set("check_flapping_recovery_notification", "2");
    _ref.set("check_latency", "55.3");
    _ref.set("check_options", "3");
    _ref.set("check_period", "check_period");
    _ref.set("check_type", "5");
    _ref.set("current_attempt", "33");
    _ref.set("current_event_id", "42");
    _ref.set("current_notification_id", "24");
    _ref.set("current_notification_number", "54");
    _ref.set("current_problem_id", "88");
    _ref.set("current_state", "2");
    _ref.set("event_handler", "event_handler");
    _ref.set("event_handler_enabled", "1");
    _ref.set("failure_prediction_enabled", "1");
    _ref.set("flap_detection_enabled", "1");
    _ref.set("has_been_checked", "1");
    _ref.set("host_name", "host_name");
    _ref.set("is_flapping", "1");
    _ref.set("last_check", "1300000");
    _ref.set("last_event_id", "66");
    _ref.set("last_hard_state", "1300001");
    _ref.set("last_hard_state_change", "1300002");
    _ref.set("last_notification", "1300003");
    _ref.set("last_problem_id", "99");
    _ref.set("last_state", "1300004");
    _ref.set("last_state_change", "1300005");
    _ref.set("last_time_critical", "1300006");
    _ref.set("last_time_ok", "1300007");
    _ref.set("last_time_unknown", "1300008");
    _ref.set("last_time_warning", "1300009");
    _ref.set("long_plugin_output", "long_plugin_output");
    _ref.set("max_attempts", "13");
    _ref.set("modified_attributes", "2");
    _ref.set("next_check", "1300010");
    _ref.set("normal_check_interval", "31");
    _ref.set("notification_period", "notification_period");
    _ref.set("notifications_enabled", "1");
    _ref.set("notified_on_critical", "1");
    _ref.set("notified_on_unknown", "1");
    _ref.set("notified_on_warning", "1");
    _ref.set("obsess_over_service", "55");
    _ref.set("passive_checks_enabled", "1");
    _ref.set("percent_state_change", "33.2");
    _ref.set("performance_data", "performance_data");
    _ref.set("plugin_output", "plugin_output");
    _ref.set("problem_has_been_acknowledged", "1");
    _ref.set("process_performance_data", "66");
    _ref.set("retry_check_interval", "77");
    _ref.set("service_description", "service_description");
    _ref.set("state_history", "0,1,2,3,4,5,6,7");
    _ref.set("state_type", "2");
  }

 protected:
  retention::service _ref;
};

TEST_F(RetentionServiceTest, CopyConstructor) {
  retention::service copy(_ref);
  ASSERT_FALSE(!(copy == _ref) || copy != _ref);
}

TEST_F(RetentionServiceTest, CopyOperator) {
  retention::service copy;
  copy = _ref;
  ASSERT_FALSE(!(copy == _ref) || copy != _ref);
}

TEST_F(RetentionServiceTest, EmptyComparison) {
  retention::service empty;
  ASSERT_FALSE(empty == _ref || !(empty != _ref));
}

TEST_F(RetentionServiceTest, DiffComparison) {
  retention::service diff(_ref);
  diff.set("acknowledgement_type", "0");
  ASSERT_FALSE(diff == _ref || !(diff != _ref));
}
