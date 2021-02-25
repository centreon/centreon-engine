/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
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

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <time.h>

#include <cstring>
#include <iostream>
#include <memory>

#include <com/centreon/engine/configuration/applier/timeperiod.hh>
#include <com/centreon/engine/macros.hh>
#include <com/centreon/engine/macros/grab_host.hh>
#include <com/centreon/engine/macros/process.hh>
#include "../test_engine.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/configuration/applier/serviceescalation.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/serviceescalation.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

// extern configuration::state* config;

class ServiceTimePeriodNotification : public TestEngine {
 public:
  void SetUp() override {
    init_config_state();

    configuration::applier::contact ct_aply;
    configuration::contact ctct{new_configuration_contact("admin", true)};
    ct_aply.add_object(ctct);
    configuration::contact ctct1{
        new_configuration_contact("admin1", false, "c,r")};
    ct_aply.add_object(ctct1);
    ct_aply.expand_objects(*config);
    ct_aply.resolve_object(ctct);
    ct_aply.resolve_object(ctct1);

    configuration::host hst{new_configuration_host("test_host", "admin")};
    configuration::applier::host hst_aply;
    hst_aply.add_object(hst);

    configuration::service svc{
        new_configuration_service("test_host", "test_svc", "admin,admin1")};
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

    _tp = _creator.new_timeperiod();
    for (int i(0); i < 7; ++i)
      _creator.new_timerange(0, 0, 24, 0, i);
    _now = strtotimet("2016-11-24 08:00:00");
    set_time(_now);
  }

  void TearDown() override {
    _svc.reset();
    _host.reset();
    deinit_config_state();
  }

 protected:
  std::shared_ptr<engine::host> _host;
  std::shared_ptr<engine::service> _svc;
  com::centreon::engine::timeperiod* _tp;
  timeperiod_creator _creator;
  time_t _now;
};

// Given a service with a notification interval = 2, a
// first_delay_notification = 0, an escalation from 2 to 12 with a contactgroup
// and notification_interval = 4
// When a normal notification is sent 11 times,
// Then contacts from the escalation are notified when notification number
// is in [2,6] and are separated by at less 4*60s.
TEST_F(ServiceTimePeriodNotification, NoTimePeriodOk) {
  init_macros();
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  int now{20000};
  set_time(now);

  configuration::applier::contact ct_aply;
  configuration::contact ctct{new_configuration_contact("test_contact", false)};
  ct_aply.add_object(ctct);
  ct_aply.expand_objects(*config);
  ct_aply.resolve_object(ctct);

  configuration::applier::contactgroup cg_aply;
  configuration::contactgroup cg{
      new_configuration_contactgroup("test_cg", "test_contact")};
  cg_aply.add_object(cg);
  cg_aply.expand_objects(*config);
  cg_aply.resolve_object(cg);

  configuration::applier::serviceescalation se_aply;
  configuration::serviceescalation se{
      new_configuration_serviceescalation("test_host", "test_svc", "test_cg")};
  se_aply.add_object(se);
  se_aply.expand_objects(*config);
  se_aply.resolve_object(se);

  // uint64_t id{_svc->get_next_notification_id()};
  for (int i = 0; i < 7; ++i) {
    timerange_list list_time;
    list_time.push_back(std::make_shared<engine::timerange>(15000, 38000));
    list_time.push_back(std::make_shared<engine::timerange>(65000, 85000));
    tperiod->days[i] = list_time;
  }

  _svc->set_current_state(engine::service::state_ok);
  _svc->set_notification_interval(1);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(20000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(now);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_notification_period_ptr(tperiod.get());

  testing::internal::CaptureStdout();
  for (int i = 0; i < 12; i++) {
    // When i == 0, the state_critical is soft => no notification
    // When i == 1, the state_critical is soft => no notification
    // When i == 2, the state_critical is hard down => notification
    now += 3000;
    std::cout << "NOW = " << now << std::endl;
    set_time(now);
    _svc->set_last_state(_svc->get_current_state());
    if (notifier::hard == _svc->get_state_type())
      _svc->set_last_hard_state(_svc->get_current_state());

    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
           "critical";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  now += 3000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  now = 70000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 32000")};
  size_t step2{
      out.find("SERVICE NOTIFICATION: "
               "test_contact;test_host;test_svc;CRITICAL;cmd;service critical",
               step1 + 1)};
  size_t step3{out.find("NOW = 70000", step2 + 1)};
  size_t step4{
      out.find("SERVICE NOTIFICATION: test_contact;test_host;test_svc;RECOVERY "
               "(OK);cmd;service ok",
               step3 + 1)};
  ASSERT_NE(step4, std::string::npos);
}

TEST_F(ServiceTimePeriodNotification, NoTimePeriodKo) {

  init_macros();
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  int now{20000};
  set_time(now);

  configuration::applier::contact ct_aply;
  configuration::contact ctct{new_configuration_contact("test_contact", false)};
  ct_aply.add_object(ctct);
  ct_aply.expand_objects(*config);
  ct_aply.resolve_object(ctct);

  configuration::applier::contactgroup cg_aply;
  configuration::contactgroup cg{
      new_configuration_contactgroup("test_cg", "test_contact")};
  cg_aply.add_object(cg);
  cg_aply.expand_objects(*config);
  cg_aply.resolve_object(cg);

  configuration::applier::serviceescalation se_aply;
  configuration::serviceescalation se;
  se.parse("first_notification", "1");
  se.parse("last_notification", "1");
  se.parse("notification_interval", "0");
  se.parse("escalation_options", "w,u,c,r");
  se.parse("host_name", "test_host");
  se.parse("service_description", "test_svc");
  se.parse("contact_groups", "test_cg");
  se_aply.add_object(se);
  se_aply.expand_objects(*config);
  se_aply.resolve_object(se);
  for (int i = 0; i < 7; ++i) {
    timerange_list list_time;
    list_time.push_back(std::make_shared<engine::timerange>(35000, 85000));
    tperiod->days[i] = list_time;
  }

  _svc->set_current_state(engine::service::state_ok);
  _svc->set_notification_interval(0);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(20000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(now);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_notification_period_ptr(tperiod.get());

  testing::internal::CaptureStdout();

  for (int i = 0; i < 12; i++) {
    // When i == 0, the state_critical is soft => no notification
    // When i == 1, the state_critical is soft => no notification
    // When i == 2, the state_critical is hard down => notification
    now += 3000;
    std::cout << "NOW = " << now << std::endl;
    set_time(now);
    _svc->set_last_state(_svc->get_current_state());
    if (notifier::hard == _svc->get_state_type())
      _svc->set_last_hard_state(_svc->get_current_state());

    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
           "critical";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  now += 3000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  now = 70000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 35000")};
  size_t step2{
      out.find("SERVICE NOTIFICATION: "
               "test_contact;test_host;test_svc;CRITICAL;cmd;service critical",
               step1 + 1)};
  size_t step3{out.find("NOW = 59000", step2 + 1)};
  size_t step4{
      out.find("SERVICE NOTIFICATION: test_contact;test_host;test_svc;RECOVERY "
               "(OK);cmd;service ok",
               step3 + 1)};
  ASSERT_NE(step4, std::string::npos);

  size_t step5{out.find("NOW = 44000")};
  size_t step6{
      out.find("SERVICE NOTIFICATION: "
               "test_contact;test_host;test_svc;CRITICAL;cmd;service critical",
               step5 + 1)};
  ASSERT_EQ(step6, std::string::npos);
}

TEST_F(ServiceTimePeriodNotification, TimePeriodOut) {
  init_macros();
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  int now{20000};
  set_time(now);

  configuration::applier::contact ct_aply;
  configuration::contact ctct{new_configuration_contact("test_contact", false)};
  ct_aply.add_object(ctct);
  ct_aply.expand_objects(*config);
  ct_aply.resolve_object(ctct);

  configuration::applier::contactgroup cg_aply;
  configuration::contactgroup cg{
      new_configuration_contactgroup("test_cg", "test_contact")};
  cg_aply.add_object(cg);
  cg_aply.expand_objects(*config);
  cg_aply.resolve_object(cg);

  configuration::applier::serviceescalation se_aply;
  configuration::serviceescalation se{
      new_configuration_serviceescalation("test_host", "test_svc", "test_cg")};
  se_aply.add_object(se);
  se_aply.expand_objects(*config);
  se_aply.resolve_object(se);

  // uint64_t id{_svc->get_next_notification_id()};
  for (int i = 0; i < 7; ++i) {
    timerange_list list_time;
    list_time.push_back(std::make_shared<engine::timerange>(1000, 15000));
    list_time.push_back(std::make_shared<engine::timerange>(80000, 85000));
    tperiod->days[i] = list_time;
  }

  _svc->set_current_state(engine::service::state_ok);
  _svc->set_notification_interval(1);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(20000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(now);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_notification_period_ptr(tperiod.get());

  testing::internal::CaptureStdout();
  for (int i = 0; i < 12; i++) {
    // When i == 0, the state_critical is soft => no notification
    // When i == 1, the state_critical is soft => no notification
    // When i == 2, the state_critical is hard down => notification
    now += 3000;
    std::cout << "NOW = " << now << std::endl;
    set_time(now);
    _svc->set_last_state(_svc->get_current_state());
    if (notifier::hard == _svc->get_state_type())
      _svc->set_last_hard_state(_svc->get_current_state());

    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
           "critical";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  now += 3000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  now = 70000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 32000")};
  size_t step2{
      out.find("SERVICE NOTIFICATION: "
               "test_contact;test_host;test_svc;CRITICAL;cmd;service critical",
               step1 + 1)};
  size_t step3{out.find("NOW = 59000", step2 + 1)};
  size_t step4{
      out.find("SERVICE NOTIFICATION: test_contact;test_host;test_svc;RECOVERY "
               "(OK);cmd;service ok",
               step3 + 1)};
  ASSERT_EQ(step4, std::string::npos);
}

TEST_F(ServiceTimePeriodNotification, TimePeriodUserOut) {
  init_macros();
  std::unique_ptr<engine::timeperiod> tiperiod{
      new engine::timeperiod("tperiod", "alias")};
  int now{20000};
  set_time(now);
  configuration::timeperiod tperiod;
  tperiod.parse("timeperiod_name", "24x9");
  tperiod.parse("alias", "24x9");
  tperiod.parse("monday", "00:00-09:00");
  tperiod.parse("tuesday", "00:00-09:00");
  tperiod.parse("wednesday", "00:00-09:00");
  tperiod.parse("thursday", "00:00-09:00");
  tperiod.parse("friday", "00:00-09:00");
  tperiod.parse("saterday", "00:00-09:00");
  tperiod.parse("sunday", "00:00-09:00");
  configuration::applier::timeperiod aplyr;
  aplyr.add_object(tperiod);

  configuration::applier::contact ct_aply;
  configuration::contact ctct;
  ctct.parse("contact_name", "test_contact");
  ctct.parse("service_notification_period", "24x9");
  ctct.parse("host_notification_commands", "cmd");
  ctct.parse("service_notification_commands", "cmd");
  ctct.parse("host_notification_options", "d,r,f,s");
  ctct.parse("service_notification_options", "a");
  ctct.parse("host_notifications_enabled", "1");
  ctct.parse("service_notifications_enabled", "1");

  ct_aply.add_object(ctct);
  ct_aply.expand_objects(*config);
  ct_aply.resolve_object(ctct);

  configuration::applier::contactgroup cg_aply;
  configuration::contactgroup cg{
      new_configuration_contactgroup("test_cg", "test_contact")};
  cg_aply.add_object(cg);
  cg_aply.expand_objects(*config);
  cg_aply.resolve_object(cg);

  configuration::applier::serviceescalation se_aply;
  configuration::serviceescalation se{
      new_configuration_serviceescalation("test_host", "test_svc", "test_cg")};
  se_aply.add_object(se);
  se_aply.expand_objects(*config);
  se_aply.resolve_object(se);

  // uint64_t id{_svc->get_next_notification_id()};
  for (int i = 0; i < 7; ++i) {
    timerange_list list_time;
    // list_time.push_back(std::make_shared<engine::timerange>(1000, 15000));
    list_time.push_back(std::make_shared<engine::timerange>(8000, 85000));
    tiperiod->days[i] = list_time;
  }

  _svc->set_current_state(engine::service::state_ok);
  _svc->set_notification_interval(1);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(20000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(now);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_notification_period_ptr(tiperiod.get());

  testing::internal::CaptureStdout();
  for (int i = 0; i < 12; i++) {
    // When i == 0, the state_critical is soft => no notification
    // When i == 1, the state_critical is soft => no notification
    // When i == 2, the state_critical is hard down => notification
    now += 3000;
    std::cout << "NOW = " << now << std::endl;
    set_time(now);
    _svc->set_last_state(_svc->get_current_state());
    if (notifier::hard == _svc->get_state_type())
      _svc->set_last_hard_state(_svc->get_current_state());

    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
           "critical";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  now += 3000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  now = 70000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 32000")};
  size_t step2{
      out.find("SERVICE NOTIFICATION: "
               "test_contact;test_host;test_svc;CRITICAL;cmd;service critical",
               step1 + 1)};
  size_t step3{out.find("NOW = 59000", step2 + 1)};
  size_t step4{
      out.find("SERVICE NOTIFICATION: test_contact;test_host;test_svc;RECOVERY "
               "(OK);cmd;service ok",
               step3 + 1)};
  ASSERT_EQ(step4, std::string::npos);
}

TEST_F(ServiceTimePeriodNotification, TimePeriodUserIn) {
  init_macros();
  std::unique_ptr<engine::timeperiod> tiperiod{
      new engine::timeperiod("tperiod", "alias")};
  int now{20000};
  set_time(now);
  configuration::timeperiod tperiod;
  tperiod.parse("timeperiod_name", "24x9");
  tperiod.parse("alias", "24x9");
  tperiod.parse("monday", "09:00-20:00");
  tperiod.parse("tuesday", "09:00-20:00");
  tperiod.parse("wednesday", "09:00-20:00");
  tperiod.parse("thursday", "09:00-20:00");
  tperiod.parse("friday", "09:00-20:00");
  tperiod.parse("saterday", "09:00-20:00");
  tperiod.parse("sunday", "09:00-20:00");
  configuration::applier::timeperiod aplyr;
  aplyr.add_object(tperiod);

  configuration::applier::contact ct_aply;
  configuration::contact ctct;
  ctct.parse("contact_name", "test_contact");
  ctct.parse("service_notification_period", "24x9");
  ctct.parse("host_notification_commands", "cmd");
  ctct.parse("service_notification_commands", "cmd");
  ctct.parse("host_notification_options", "d,r,f,s");
  ctct.parse("service_notification_options", "a");
  ctct.parse("host_notifications_enabled", "1");
  ctct.parse("service_notifications_enabled", "1");

  ct_aply.add_object(ctct);
  ct_aply.expand_objects(*config);
  ct_aply.resolve_object(ctct);

  configuration::applier::contactgroup cg_aply;
  configuration::contactgroup cg{
      new_configuration_contactgroup("test_cg", "test_contact")};
  cg_aply.add_object(cg);
  cg_aply.expand_objects(*config);
  cg_aply.resolve_object(cg);

  configuration::applier::serviceescalation se_aply;
  configuration::serviceescalation se{
      new_configuration_serviceescalation("test_host", "test_svc", "test_cg")};
  se_aply.add_object(se);
  se_aply.expand_objects(*config);
  se_aply.resolve_object(se);

  // uint64_t id{_svc->get_next_notification_id()};
  for (int i = 0; i < 7; ++i) {
    timerange_list list_time;
    // list_time.push_back(std::make_shared<engine::timerange>(1000, 15000));
    list_time.push_back(std::make_shared<engine::timerange>(8000, 85000));
    tiperiod->days[i] = list_time;
  }

  _svc->set_current_state(engine::service::state_ok);
  _svc->set_notification_interval(1);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(20000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(now);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_notification_period_ptr(tiperiod.get());

  testing::internal::CaptureStdout();
  for (int i = 0; i < 12; i++) {
    // When i == 0, the state_critical is soft => no notification
    // When i == 1, the state_critical is soft => no notification
    // When i == 2, the state_critical is hard down => notification
    now += 3000;
    std::cout << "NOW = " << now << std::endl;
    set_time(now);
    _svc->set_last_state(_svc->get_current_state());
    if (notifier::hard == _svc->get_state_type())
      _svc->set_last_hard_state(_svc->get_current_state());

    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
           "critical";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  now += 3000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  now = 70000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 32000")};
  size_t step2{
      out.find("SERVICE NOTIFICATION: "
               "test_contact;test_host;test_svc;CRITICAL;cmd;service critical",
               step1 + 1)};
  size_t step3{out.find("NOW = 59000", step2 + 1)};
  size_t step4{
      out.find("SERVICE NOTIFICATION: test_contact;test_host;test_svc;RECOVERY "
               "(OK);cmd;service ok",
               step3 + 1)};
  ASSERT_NE(step4, std::string::npos);
}

TEST_F(ServiceTimePeriodNotification, TimePeriodUserAll) {
  init_macros();
  std::unique_ptr<engine::timeperiod> tiperiod{
      new engine::timeperiod("tperiod", "alias")};
  int now{20000};
  set_time(now);
  configuration::timeperiod tperiod;
  tperiod.parse("timeperiod_name", "24x9");
  tperiod.parse("alias", "24x9");
  tperiod.parse("monday", "00:00-24:00");
  tperiod.parse("tuesday", "00:00-24:00");
  tperiod.parse("wednesday", "00:00-24:00");
  tperiod.parse("thursday", "00:00-24:00");
  tperiod.parse("friday", "00:00-24:00");
  tperiod.parse("saterday", "00:00-24:00");
  tperiod.parse("sunday", "00:00-24:00");
  configuration::applier::timeperiod aplyr;
  aplyr.add_object(tperiod);

  configuration::applier::contact ct_aply;
  configuration::contact ctct;
  ctct.parse("contact_name", "test_contact");
  ctct.parse("service_notification_period", "24x9");
  ctct.parse("host_notification_commands", "cmd");
  ctct.parse("service_notification_commands", "cmd");
  ctct.parse("host_notification_options", "d,r,f,s");
  ctct.parse("service_notification_options", "a");
  ctct.parse("host_notifications_enabled", "1");
  ctct.parse("service_notifications_enabled", "1");

  ct_aply.add_object(ctct);
  ct_aply.expand_objects(*config);
  ct_aply.resolve_object(ctct);

  configuration::applier::contactgroup cg_aply;
  configuration::contactgroup cg{
      new_configuration_contactgroup("test_cg", "test_contact")};
  cg_aply.add_object(cg);
  cg_aply.expand_objects(*config);
  cg_aply.resolve_object(cg);

  configuration::applier::serviceescalation se_aply;
  configuration::serviceescalation se{
      new_configuration_serviceescalation("test_host", "test_svc", "test_cg")};
  se_aply.add_object(se);
  se_aply.expand_objects(*config);
  se_aply.resolve_object(se);

  // uint64_t id{_svc->get_next_notification_id()};
  for (int i = 0; i < 7; ++i) {
    timerange_list list_time;
    // list_time.push_back(std::make_shared<engine::timerange>(1000, 15000));
    list_time.push_back(std::make_shared<engine::timerange>(8000, 85000));
    tiperiod->days[i] = list_time;
  }

  _svc->set_current_state(engine::service::state_ok);
  _svc->set_notification_interval(1);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(20000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(now);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_notification_period_ptr(tiperiod.get());

  testing::internal::CaptureStdout();
  for (int i = 0; i < 12; i++) {
    // When i == 0, the state_critical is soft => no notification
    // When i == 1, the state_critical is soft => no notification
    // When i == 2, the state_critical is hard down => notification
    now += 3000;
    std::cout << "NOW = " << now << std::endl;
    set_time(now);
    _svc->set_last_state(_svc->get_current_state());
    if (notifier::hard == _svc->get_state_type())
      _svc->set_last_hard_state(_svc->get_current_state());

    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
           "critical";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  now += 3000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  now = 70000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 32000")};
  size_t step2{
      out.find("SERVICE NOTIFICATION: "
               "test_contact;test_host;test_svc;CRITICAL;cmd;service critical",
               step1 + 1)};
  size_t step3{out.find("NOW = 59000", step2 + 1)};
  size_t step4{
      out.find("SERVICE NOTIFICATION: test_contact;test_host;test_svc;RECOVERY "
               "(OK);cmd;service ok",
               step3 + 1)};
  ASSERT_NE(step4, std::string::npos);
}

TEST_F(ServiceTimePeriodNotification, TimePeriodUserNone) {
  init_macros();
  std::unique_ptr<engine::timeperiod> tiperiod{
      new engine::timeperiod("tperiod", "alias")};
  int now{20000};
  set_time(now);
  configuration::timeperiod tperiod;
  tperiod.parse("timeperiod_name", "24x9");
  tperiod.parse("alias", "24x9");
  configuration::applier::timeperiod aplyr;
  aplyr.add_object(tperiod);

  configuration::applier::contact ct_aply;
  configuration::contact ctct;
  ctct.parse("contact_name", "test_contact");
  ctct.parse("service_notification_period", "24x9");
  ctct.parse("host_notification_commands", "cmd");
  ctct.parse("service_notification_commands", "cmd");
  ctct.parse("host_notification_options", "d,r,f,s");
  ctct.parse("service_notification_options", "a");
  ctct.parse("host_notifications_enabled", "1");
  ctct.parse("service_notifications_enabled", "1");

  ct_aply.add_object(ctct);
  ct_aply.expand_objects(*config);
  ct_aply.resolve_object(ctct);

  configuration::applier::contactgroup cg_aply;
  configuration::contactgroup cg{
      new_configuration_contactgroup("test_cg", "test_contact")};
  cg_aply.add_object(cg);
  cg_aply.expand_objects(*config);
  cg_aply.resolve_object(cg);

  configuration::applier::serviceescalation se_aply;
  configuration::serviceescalation se{
      new_configuration_serviceescalation("test_host", "test_svc", "test_cg")};
  se_aply.add_object(se);
  se_aply.expand_objects(*config);
  se_aply.resolve_object(se);

  // uint64_t id{_svc->get_next_notification_id()};
  for (int i = 0; i < 7; ++i) {
    timerange_list list_time;
    // list_time.push_back(std::make_shared<engine::timerange>(1000, 15000));
    list_time.push_back(std::make_shared<engine::timerange>(8000, 85000));
    tiperiod->days[i] = list_time;
  }

  _svc->set_current_state(engine::service::state_ok);
  _svc->set_notification_interval(1);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(20000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(now);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_notification_period_ptr(tiperiod.get());

  testing::internal::CaptureStdout();
  for (int i = 0; i < 12; i++) {
    // When i == 0, the state_critical is soft => no notification
    // When i == 1, the state_critical is soft => no notification
    // When i == 2, the state_critical is hard down => notification
    now += 3000;
    std::cout << "NOW = " << now << std::endl;
    set_time(now);
    _svc->set_last_state(_svc->get_current_state());
    if (notifier::hard == _svc->get_state_type())
      _svc->set_last_hard_state(_svc->get_current_state());

    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
           "critical";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  now += 3000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  now = 70000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 32000")};
  size_t step2{
      out.find("SERVICE NOTIFICATION: "
               "test_contact;test_host;test_svc;CRITICAL;cmd;service critical",
               step1 + 1)};
  size_t step3{out.find("NOW = 59000", step2 + 1)};
  size_t step4{
      out.find("SERVICE NOTIFICATION: test_contact;test_host;test_svc;RECOVERY "
               "(OK);cmd;service ok",
               step3 + 1)};
  ASSERT_EQ(step4, std::string::npos);
}

TEST_F(ServiceTimePeriodNotification, NoTimePeriodUser) {
  init_macros();
  std::unique_ptr<engine::timeperiod> tiperiod{
      new engine::timeperiod("tperiod", "alias")};
  int now{20000};
  set_time(now);
  configuration::timeperiod tperiod;
  tperiod.parse("timeperiod_name", "24x9");
  tperiod.parse("alias", "24x9");
  configuration::applier::timeperiod aplyr;
  aplyr.add_object(tperiod);

  configuration::applier::contact ct_aply;
  configuration::contact ctct;
  ctct.parse("contact_name", "test_contact");
  ctct.parse("host_notification_commands", "cmd");
  ctct.parse("service_notification_commands", "cmd");
  ctct.parse("host_notification_options", "d,r,f,s");
  ctct.parse("service_notification_options", "a");
  ctct.parse("host_notifications_enabled", "1");
  ctct.parse("service_notifications_enabled", "1");

  ct_aply.add_object(ctct);
  ct_aply.expand_objects(*config);
  ct_aply.resolve_object(ctct);

  configuration::applier::contactgroup cg_aply;
  configuration::contactgroup cg{
      new_configuration_contactgroup("test_cg", "test_contact")};
  cg_aply.add_object(cg);
  cg_aply.expand_objects(*config);
  cg_aply.resolve_object(cg);

  configuration::applier::serviceescalation se_aply;
  configuration::serviceescalation se{
      new_configuration_serviceescalation("test_host", "test_svc", "test_cg")};
  se_aply.add_object(se);
  se_aply.expand_objects(*config);
  se_aply.resolve_object(se);

  // uint64_t id{_svc->get_next_notification_id()};
  for (int i = 0; i < 7; ++i) {
    timerange_list list_time;
    // list_time.push_back(std::make_shared<engine::timerange>(1000, 15000));
    list_time.push_back(std::make_shared<engine::timerange>(8000, 85000));
    tiperiod->days[i] = list_time;
  }

  _svc->set_current_state(engine::service::state_ok);
  _svc->set_notification_interval(1);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(20000);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(now);
  _svc->set_state_type(checkable::hard);
  _svc->set_accept_passive_checks(true);
  _svc->set_notification_period_ptr(tiperiod.get());

  testing::internal::CaptureStdout();
  for (int i = 0; i < 12; i++) {
    // When i == 0, the state_critical is soft => no notification
    // When i == 1, the state_critical is soft => no notification
    // When i == 2, the state_critical is hard down => notification
    now += 3000;
    std::cout << "NOW = " << now << std::endl;
    set_time(now);
    _svc->set_last_state(_svc->get_current_state());
    if (notifier::hard == _svc->get_state_type())
      _svc->set_last_hard_state(_svc->get_current_state());

    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
           "critical";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  now += 3000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  now = 70000;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;service ok";
  cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 32000")};
  size_t step2{
      out.find("SERVICE NOTIFICATION: "
               "test_contact;test_host;test_svc;CRITICAL;cmd;service critical",
               step1 + 1)};
  size_t step3{out.find("NOW = 59000", step2 + 1)};
  size_t step4{
      out.find("SERVICE NOTIFICATION: test_contact;test_host;test_svc;RECOVERY "
               "(OK);cmd;service ok",
               step3 + 1)};
  ASSERT_NE(step4, std::string::npos);
}