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

#include <gtest/gtest.h>
#include <time.h>
#include <cstring>
#include <iostream>
#include <memory>
#include "../test_engine.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/configuration/applier/serviceescalation.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/serviceescalation.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ServiceCheck : public TestEngine {
 public:
  void SetUp() override {
    if (!config)
      config = new configuration::state;

    config->contacts().clear();
    configuration::applier::contact ct_aply;
    configuration::contact ctct{new_configuration_contact("admin", true)};
    ct_aply.add_object(ctct);
    ct_aply.expand_objects(*config);
    ct_aply.resolve_object(ctct);

    configuration::host hst{new_configuration_host("test_host", "admin")};
    configuration::applier::host hst_aply;
    hst_aply.add_object(hst);

    configuration::service svc{
        new_configuration_service("test_host", "test_svc", "admin")};
    configuration::applier::service svc_aply;
    svc_aply.add_object(svc);

    hst_aply.resolve_object(hst);
    svc_aply.resolve_object(svc);

    host_map const& hm{engine::host::hosts};
    _host = hm.begin()->second;
    _host->set_current_state(engine::host::state_up);
    _host->set_state_type(checkable::hard);
    _host->set_problem_has_been_acknowledged(false);
    _host->set_notify_on(static_cast<uint32_t>(-1));

    service_map const& sm{engine::service::services};
    _svc = sm.begin()->second;
    _svc->set_current_state(engine::service::state_ok);
    _svc->set_state_type(checkable::hard);
    _svc->set_problem_has_been_acknowledged(false);
    _svc->set_notify_on(static_cast<uint32_t>(-1));
  }

  void TearDown() override {
    deinit_config_state();
//    configuration::applier::state::instance().clear();
//    delete config;
//    config = nullptr;
  }

 protected:
  std::shared_ptr<engine::host> _host;
  std::shared_ptr<engine::service> _svc;
};

/* The following test comes from this array (inherited from Nagios behaviour):
 *
 * | Time | Check # | State | State type | State change |
 * ------------------------------------------------------
 * | 0    | 1       | OK    | HARD       | No           |
 * | 1    | 1       | CRTCL | SOFT       | Yes          |
 * | 2    | 2       | WARN  | SOFT       | Yes          |
 * | 3    | 3       | CRTCL | HARD       | Yes          |
 * | 4    | 3       | WARN  | HARD       | Yes          |
 * | 5    | 3       | WARN  | HARD       | No           |
 * | 6    | 1       | OK    | HARD       | Yes          |
 * | 7    | 1       | OK    | HARD       | No           |
 * | 8    | 1       | UNKNWN| SOFT       | Yes          |
 * | 9    | 2       | OK    | SOFT       | Yes          |
 * | 10   | 1       | OK    | HARD       | No           |
 * ------------------------------------------------------
 */
TEST_F(ServiceCheck, SimpleCheck) {
  set_time(50000);
  _svc->set_current_state(engine::service::state_ok);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(50000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_current_attempt(1);

  set_time(50500);

  std::ostringstream oss;
  std::time_t now{std::time(nullptr)};
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);

  set_time(51000);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;1;service warning";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_warning);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 2);

  set_time(51500);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 3);

  set_time(52000);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;1;service warning";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_warning);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 3);

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
}

/* The following test comes from this array (inherited from Nagios behaviour):
 *
 * | Time | Check # | State | State type | State change |
 * ------------------------------------------------------
 * | 0    | 1       | OK    | HARD       | No           |
 * | 1    | 1       | CRTCL | SOFT       | Yes          |
 * | 2    | 2       | CRTCL | SOFT       | No           |
 * | 3    | 3       | CRTCL | HARD       | No           |
 * ------------------------------------------------------
 */
TEST_F(ServiceCheck, OkCritical) {
  std::ostringstream oss;
  set_time(55000);

  time_t now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  std::string cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);

  set_time(55500);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);

  set_time(56000);

  time_t previous = now;
  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_state_change(), previous);
  ASSERT_EQ(_svc->get_current_attempt(), 2);

  set_time(56500);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 3);
}

/* The following test comes from this array (inherited from Nagios behaviour):
 *
 * | Time | Check # | State | State type | State change |
 * ------------------------------------------------------
 * | 0    | 2       | OK    | SOFT       | No           |
 * | 1    | 1       | CRTCL | SOFT       | Yes          |
 * | 2    | 2       | CRTCL | SOFT       | No           |
 * | 3    | 3       | CRTCL | HARD       | No           |
 * ------------------------------------------------------
 */
TEST_F(ServiceCheck, OkSoft_Critical) {
  std::ostringstream oss;
  set_time(55000);

  time_t now = std::time(nullptr);
  _svc->set_current_state(engine::service::state_ok);
  _svc->set_last_state_change(55000);
  _svc->set_current_attempt(2);
  _svc->set_state_type(checkable::soft);
  _svc->set_accept_passive_checks(true);

  set_time(55500);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical";
  std::string cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 1);

  set_time(56000);

  time_t previous = now;
  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::soft);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_state_change(), previous);
  ASSERT_EQ(_svc->get_current_attempt(), 2);

  set_time(56500);

  now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service critical";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_critical);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_current_attempt(), 3);
}
