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
#include <cstring>
#include <iostream>
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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/checks/checker.hh"
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
        "test_host", "test_ad", "admin", 9, 8, "/tmp/thresholds_status_change.json")};
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

  void TearDown() override { deinit_config_state(); }

  void CreateFile(std::string const& filename, std::string const& content) {
    std::ofstream oss(filename);
    oss << content;
  }

 protected:
  std::shared_ptr<engine::host> _host;
  std::shared_ptr<engine::service> _svc;
  std::shared_ptr<engine::anomalydetection> _ad;
};

TEST_F(AnomalydetectionCheck, ServiceSimpleCheck) {
  ASSERT_EQ(engine::commands::command::commands.size(), 2u);
  set_time(50000);
  int check_options = 0;
  double latency = 0;
  bool time_is_valid;
  time_t preferred_time = 0;

  /* Let's schedule a check for the service */
  ASSERT_EQ(_svc->run_async_check(check_options, latency, true, true,
                                  &time_is_valid, &preferred_time),
            0);

  /* Let's force checks to be done. */
  sleep(1);
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_plugin_output(), "output");
}

TEST_F(AnomalydetectionCheck, SimpleCheck) {
  ASSERT_EQ(engine::commands::command::commands.size(), 2u);
  set_time(50000);
  int check_options = 0;
  double latency = 0;
  bool time_is_valid;
  time_t preferred_time = 0;

  /* Let's schedule a check for the service */
  ASSERT_EQ(_ad->run_async_check(check_options, latency, true, true,
                                 &time_is_valid, &preferred_time),
            0);

  /* Let's force checks to be done. */
  sleep(1);
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_plugin_output(), "output");
}

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
      "[{\n \"host_id\": 12,\n \"service_id\": 9,\n \"metric_name\": "
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
         "metric=90;25;60";
  std::string cmd{oss.str()};
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
  ASSERT_EQ(_ad->get_state_type(), checkable::soft);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_ad->get_last_state_change(), now);
  ASSERT_EQ(_ad->get_current_attempt(), 1);
  ASSERT_EQ(_ad->get_plugin_output(), "service critical");
  ASSERT_EQ(_ad->get_perf_data(), "metric=90;25;60");

  set_time(51000);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
    << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;1;service warning| metric=50;25;60";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_warning);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 2);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  ASSERT_EQ(_ad->get_state_type(), checkable::soft);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);

  set_time(51500);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
    << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical| metric=110;25;60";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 3);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_ad->get_last_hard_state_change(), now);
  ASSERT_EQ(_ad->get_state_type(), checkable::hard);

  set_time(52000);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
    << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;1;service warning| metric=30;25;60";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_warning);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 3);
  _ad->run_async_check(check_options, latency, true, true, &time_is_valid,
                       &preferred_time);
  ASSERT_EQ(_ad->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_ad->get_last_hard_state_change(), now);


  set_time(52500);

  time_t previous = now;
  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
    << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;1;service warning";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_warning);
  ASSERT_EQ(_svc->get_last_hard_state_change(), previous);
  ASSERT_EQ(_svc->get_current_attempt(), 3);

  set_time(53000);

  previous = now;
  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
    << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);

  set_time(53500);

  previous = now;
  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
    << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_hard_state_change(), previous);
  ASSERT_EQ(_svc->get_current_attempt(), 1);

  set_time(54000);

  previous = now;
  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
    << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;4;service unknown";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_unknown);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now - 1000);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);

  set_time(54500);

  previous = now;
  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
    << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 2);

  set_time(55000);

  previous = now;
  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
    << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);
  ::unlink("/tmp/thresholds_status_change.json");
}

