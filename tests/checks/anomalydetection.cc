/*
 * Copyright 2020 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#include "com/centreon/engine/configuration/applier/anomalydetection.hh"

#include <gtest/gtest.h>
#include <time.h>
#include <fmt/format.h>

#include <cstring>
#include <memory>

#include "../test_engine.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/anomalydetection.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

class AnomalydetectionCheck : public TestEngine {
 public:
  void SetUp() override {
    init_config_state();

    configuration::applier::contact ct_aply;
    configuration::contact ctct{new_configuration_contact("admin", true)};
    ct_aply.add_object(ctct);
    ct_aply.expand_objects(*config);
    ct_aply.resolve_object(ctct);

    configuration::host hst{new_configuration_host("test_host", "admin")};
    configuration::applier::host hst_aply;
    hst_aply.add_object(hst);

    configuration::service svc{
        new_configuration_service("test_host", "test_svc", "admin", 8)};
    configuration::applier::service svc_aply;
    svc_aply.add_object(svc);

    hst_aply.resolve_object(hst);
    svc_aply.resolve_object(svc);

    configuration::anomalydetection ad{new_configuration_anomalydetection(
        "test_host", "test_ad", "admin", 9, 8,
        "/tmp/thresholds_status_change.json")};
    configuration::applier::anomalydetection ad_aply;
    ad_aply.add_object(ad);

    ad_aply.resolve_object(ad);

    host_map const& hm{engine::host::hosts};
    _host = hm.begin()->second;
    _host->set_current_state(engine::host::state_up);
    _host->set_state_type(checkable::hard);
    _host->set_problem_has_been_acknowledged(false);
    _host->set_notify_on(static_cast<uint32_t>(-1));

    service_map const& sm{engine::service::services};
    for (auto& p : sm) {
      std::shared_ptr<engine::service> svc = p.second;
      if (svc->get_service_id() == 8)
        _svc = svc;
      else
        _ad = std::static_pointer_cast<engine::anomalydetection>(svc);
    }
    _svc->set_current_state(engine::service::state_ok);
    _svc->set_state_type(checkable::hard);
    _svc->set_problem_has_been_acknowledged(false);
    _svc->set_notify_on(static_cast<uint32_t>(-1));
  }

  void TearDown() override {
    _host.reset();
    _svc.reset();
    _ad.reset();
    deinit_config_state();
  }

  void CreateFile(std::string const& filename, std::string const& content) {
    std::ofstream oss(filename);
    oss << content;
    oss.close();
  }

 protected:
  std::shared_ptr<engine::host> _host;
  std::shared_ptr<engine::service> _svc;
  std::shared_ptr<engine::anomalydetection> _ad;
};

/* The following test comes from this array (inherited from Nagios behaviour):
 *
 * | Time | Check # | State | State type | State change |
 * ------------------------------------------------------
 * | 0    | 1       | OK    | HARD       | No           |
 * | 1    | 1       | CRTCL | SOFT       | Yes          |
 * | 2    | 2       | CRTCL | SOFT       | Yes          |
 * | 3    | 3       | CRTCL | HARD       | Yes          |
 * | 4    | 3       | CRTCL | HARD       | Yes          |
 * | 5    | 3       | CRTCL | HARD       | No           |
 * | 6    | 1       | OK    | HARD       | Yes          |
 * | 7    | 1       | OK    | HARD       | No           |
 * | 8    | 1       | CRTCL | SOFT       | Yes          |
 * | 9    | 2       | OK    | SOFT       | Yes          |
 * | 10   | 1       | OK    | HARD       | No           |
 * ------------------------------------------------------
 */
TEST_F(AnomalydetectionCheck, StatusChanges) {
  CreateFile(
      "/tmp/thresholds_status_change.json",
      "[{\n \"host_id\": \"12\",\n \"service_id\": \"9\",\n \"metric_name\": "
      "\"metric\",\n \"predict\": [{\n \"timestamp\": 50000,\n \"upper\": "
      "84,\n \"lower\": 74,\n \"fit\": 79\n }, {\n \"timestamp\": 100000,\n "
      "\"upper\": 10,\n \"lower\": 5,\n \"fit\": 51.5\n }, {\n \"timestamp\": "
      "150000,\n \"upper\": 100,\n \"lower\": 93,\n \"fit\": 96.5\n }, {\n "
      "\"timestamp\": 200000,\n \"upper\": 100,\n \"lower\": 97,\n \"fit\": "
      "98.5\n }, {\n \"timestamp\": 250000,\n \"upper\": 100,\n \"lower\": "
      "21,\n \"fit\": 60.5\n }\n]}]");
  _ad->init_thresholds();
  _ad->set_status_change(true);

  set_time(50000);
  _svc->set_current_state(engine::service::state_ok);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(50000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_current_attempt(1);
  _svc->set_last_check(50000);

  _ad->set_current_state(engine::service::state_ok);
  _ad->set_last_hard_state(engine::service::state_ok);
  _ad->set_last_hard_state_change(50000);
  _ad->set_state_type(checkable::hard);
  _ad->set_current_attempt(1);
  _ad->set_last_check(50000);

  set_time(50500);
  time_t now = std::time(nullptr);
  std::string cmd(fmt::format(
      "[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
      "critical| metric=90;25;60",
      now));
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);
  ASSERT_EQ(_svc->get_plugin_output(), "service critical");
  ASSERT_EQ(_svc->get_perf_data(), "metric=90;25;60");
  int check_options = 0;
  int latency = 0;
  bool time_is_valid;
  time_t preferred_time;
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::soft);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_ad->get_last_state_change(), now);
  ASSERT_EQ(_ad->get_current_attempt(), 1);
  ASSERT_EQ(_ad->get_plugin_output(),
            "NON-OK: Unusual activity, the actual value of metric is 90.00 "
            "which is outside the forecasting range [73.31 : 83.26]");
  ASSERT_EQ(_ad->get_perf_data(),
            "metric=90 metric_lower_thresholds=73.31 "
            "metric_upper_thresholds=83.26");

  set_time(51000);

  now = std::time(nullptr);
  cmd = fmt::format("[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;1;service warning| "
         "metric=50;25;60", now);
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_warning);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 2);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::soft);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_ad->get_plugin_output(),
            "NON-OK: Unusual activity, the actual value of metric is 50.00 "
            "which is outside the forecasting range [72.62 : 82.52]");
  ASSERT_EQ(_ad->get_perf_data(),
            "metric=50 metric_lower_thresholds=72.62 "
            "metric_upper_thresholds=82.52");
  ASSERT_EQ(_ad->get_current_attempt(), 2);

  set_time(51500);

  now = std::time(nullptr);
  time_t previous = now;
  cmd = fmt::format("[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical| "
         "metric=110foo;25;60", now);
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 3);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_ad->get_last_hard_state_change(), now);
  ASSERT_EQ(_ad->get_state_type(), checkable::hard);
  ASSERT_EQ(_ad->get_plugin_output(),
            "NON-OK: Unusual activity, the actual value of metric is 110.00foo "
            "which is outside the forecasting range [71.93 : 81.78]");
  ASSERT_EQ(_ad->get_perf_data(),
            "metric=110foo metric_lower_thresholds=71.93foo "
            "metric_upper_thresholds=81.78foo");
  ASSERT_EQ(_ad->get_current_attempt(), 3);

  set_time(52000);

  now = std::time(nullptr);
  cmd = fmt::format("[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;1;service warning| "
         "metric=30%;25;60", now);
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_warning);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 3);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::hard);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_ad->get_last_hard_state_change(), previous);
  ASSERT_EQ(_ad->get_plugin_output(),
            "NON-OK: Unusual activity, the actual value of metric is 30.00% "
            "which is outside the forecasting range [71.24 : 81.04]");
  ASSERT_EQ(_ad->get_perf_data(),
            "metric=30% metric_lower_thresholds=71.24% "
            "metric_upper_thresholds=81.04%");
  ASSERT_EQ(_ad->get_current_attempt(), 3);

  set_time(52500);

  previous = now;
  now = std::time(nullptr);
  cmd = fmt::format("[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;1;service warning| "
         "metric=35%;25;60", now);
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_warning);
  ASSERT_EQ(_svc->get_last_hard_state_change(), previous);
  ASSERT_EQ(_svc->get_current_attempt(), 3);
  ASSERT_EQ(_svc->get_plugin_output(), "service warning");
  ASSERT_EQ(_svc->get_perf_data(), "metric=35%;25;60");
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::hard);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_ad->get_plugin_output(),
            "NON-OK: Unusual activity, the actual value of metric is 35.00% "
            "which is outside the forecasting range [70.55 : 80.30]");
  ASSERT_EQ(_ad->get_perf_data(),
            "metric=35% metric_lower_thresholds=70.55% "
            "metric_upper_thresholds=80.30%");
  ASSERT_EQ(_ad->get_current_attempt(), 3);

  set_time(53000);

  previous = now;
  now = std::time(nullptr);
  cmd = fmt::format("[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok| "
         "metric=70%;80;90", now);
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::hard);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_ad->get_last_hard_state_change(), now);
  ASSERT_EQ(_ad->get_plugin_output(), "OK: Regular activity, metric=70.00%");
  ASSERT_EQ(_ad->get_perf_data(),
            "metric=70% metric_lower_thresholds=69.86% "
            "metric_upper_thresholds=79.56%");
  ASSERT_EQ(_ad->get_current_attempt(), 1);

  set_time(53500);

  previous = now;
  now = std::time(nullptr);
  cmd = fmt::format("[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok| "
         "metric=71%;80;90", now);
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_hard_state_change(), previous);
  ASSERT_EQ(_svc->get_current_attempt(), 1);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::hard);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_ad->get_last_hard_state_change(), previous);
  ASSERT_EQ(_ad->get_plugin_output(), "OK: Regular activity, metric=71.00%");
  ASSERT_EQ(_ad->get_perf_data(),
            "metric=71% metric_lower_thresholds=69.17% "
            "metric_upper_thresholds=78.82%");
  ASSERT_EQ(_ad->get_current_attempt(), 1);

  set_time(54000);

  previous = now;
  now = std::time(nullptr);
  cmd = fmt::format("[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;4;service unknown", now);
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_unknown);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now - 1000);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::soft);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_unknown);
  ASSERT_EQ(_ad->get_last_hard_state_change(), now - 1000);
  ASSERT_EQ(_ad->get_plugin_output(),
            "UNKNOWN: Unknown activity, metric did not return any values");
  ASSERT_EQ(_ad->get_current_attempt(), 1);

  set_time(54500);

  previous = now;
  now = std::time(nullptr);
  cmd = fmt::format("[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok| "
         "metric=72%;80;90", now);
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 2);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::soft);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_ad->get_last_state_change(), now);
  ASSERT_EQ(_ad->get_plugin_output(), "OK: Regular activity, metric=72.00%");
  ASSERT_EQ(_ad->get_perf_data(),
            "metric=72% metric_lower_thresholds=67.79% "
            "metric_upper_thresholds=77.34%");
  ASSERT_EQ(_ad->get_current_attempt(), 2);

  set_time(55000);

  previous = now;
  now = std::time(nullptr);
  cmd = fmt::format("[{}] PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok| "
         "metric=71.7%;80;90;10;100", now);
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::hard);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_ad->get_last_hard_state_change(), now);
  ASSERT_EQ(_ad->get_plugin_output(), "OK: Regular activity, metric=71.70%");
  ASSERT_EQ(_ad->get_perf_data(),
            "metric=71.7%;;;10;100 metric_lower_thresholds=67.10%;;;10;100 "
            "metric_upper_thresholds=76.60%;;;10;100");
  ASSERT_EQ(_ad->get_current_attempt(), 1);
  ::unlink("/tmp/thresholds_status_change.json");
}

TEST_F(AnomalydetectionCheck, MetricWithQuotes) {
  CreateFile(
      "/tmp/thresholds_status_change.json",
      "[{\n \"host_id\": \"12\",\n \"service_id\": \"9\",\n \"metric_name\": "
      "\"metric\",\n \"predict\": [{\n \"timestamp\": 50000,\n \"upper\": "
      "84,\n \"lower\": 74,\n \"fit\": 79\n }, {\n \"timestamp\": 100000,\n "
      "\"upper\": 10,\n \"lower\": 5,\n \"fit\": 51.5\n }, {\n \"timestamp\": "
      "150000,\n \"upper\": 100,\n \"lower\": 93,\n \"fit\": 96.5\n }, {\n "
      "\"timestamp\": 200000,\n \"upper\": 100,\n \"lower\": 97,\n \"fit\": "
      "98.5\n }, {\n \"timestamp\": 250000,\n \"upper\": 100,\n \"lower\": "
      "21,\n \"fit\": 60.5\n }\n]}]");
  _ad->init_thresholds();
  _ad->set_status_change(true);

  set_time(50000);
  _svc->set_current_state(engine::service::state_ok);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(50000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_current_attempt(1);
  _svc->set_last_check(50000);

  _ad->set_current_state(engine::service::state_ok);
  _ad->set_last_hard_state(engine::service::state_ok);
  _ad->set_last_hard_state_change(50000);
  _ad->set_state_type(checkable::hard);
  _ad->set_current_attempt(1);
  _ad->set_last_check(50000);

  set_time(50500);
  std::ostringstream oss;
  std::time_t now{std::time(nullptr)};
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical| "
         "'metric'=90MT;25;60;0;100";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);
  ASSERT_EQ(_svc->get_plugin_output(), "service critical");
  ASSERT_EQ(_svc->get_perf_data(), "'metric'=90MT;25;60;0;100");
  int check_options = 0;
  int latency = 0;
  bool time_is_valid;
  time_t preferred_time;
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  checks::checker::instance().reap();
  ASSERT_EQ(_ad->get_state_type(), checkable::soft);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_ad->get_last_state_change(), now);
  ASSERT_EQ(_ad->get_current_attempt(), 1);
  ASSERT_EQ(_ad->get_plugin_output(),
            "NON-OK: Unusual activity, the actual value of metric is 90.00MT "
            "which is outside the forecasting range [73.31 : 83.26]");
  ASSERT_EQ(_ad->get_perf_data(),
            "'metric'=90MT;;;0;100 metric_lower_thresholds=73.31MT;;;0;100 "
            "metric_upper_thresholds=83.26MT;;;0;100");
}
