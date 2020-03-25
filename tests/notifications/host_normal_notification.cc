/*
 * Copyright 2017 - 2019 Centreon (https://www.centreon.com/)
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
#include <regex>
#include "../test_engine.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/hostdependency.hh"
#include "com/centreon/engine/configuration/applier/hostescalation.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/hostescalation.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "../helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::retention;

class HostNotification : public TestEngine {
 public:
  void SetUp() override {
    init_config_state();
    // Do not unload this in the tear down function, it is done by the
    // other unload function... :-(

    configuration::applier::contact ct_aply;
    configuration::contact ctct{new_configuration_contact("admin", true)};
    ct_aply.add_object(ctct);
    ct_aply.expand_objects(*config);
    ct_aply.resolve_object(ctct);

    configuration::host hst{new_configuration_host("test_host", "admin")};
    configuration::applier::host hst_aply;
    hst_aply.add_object(hst);
    hst_aply.resolve_object(hst);
    host_map const& hm{engine::host::hosts};
    _host = hm.begin()->second;
    _host->set_current_state(engine::host::state_up);
    _host->set_state_type(checkable::hard);
    _host->set_problem_has_been_acknowledged(false);
    _host->set_notify_on(static_cast<uint32_t>(-1));
  }

  void TearDown() override {
    downtime_manager::instance().clear_scheduled_downtimes();
    deinit_config_state();
  }

 protected:
  std::shared_ptr<engine::host> _host;
};

TEST_F(HostNotification, SimpleNormalHostNotification) {
  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
   * If we call time(), it is not the glibc time() function that will be called.
   */
  set_time(43200);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7, Uuid())};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->set_notification_period_ptr(tperiod.get());
  _host->set_current_state(engine::host::state_down);
  _host->set_last_state(engine::host::state_down);
  _host->set_last_hard_state_change(43200);
  _host->set_state_type(checkable::hard);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationNotificationsdisabled) {
  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
   * If we call time(), it is not the glibc time() function that will be called.
   */
  config->enable_notifications(false);
  set_time(43200);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7, Uuid())};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->set_notification_period_ptr(tperiod.get());
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationNotifierNotifdisabled) {
  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
   * If we call time(), it is not the glibc time() function that will be called.
   */
  set_time(43200);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7, Uuid())};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->set_notifications_enabled(false);
  _host->set_notification_period_ptr(tperiod.get());
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationOutsideTimeperiod) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(
        std::make_shared<engine::timerange>(43200, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification,
       SimpleNormalHostNotificationForcedWithNotificationDisabled) {
  config->enable_notifications(false);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(
        std::make_shared<engine::timerange>(43200, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_forced),
            OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationForcedNotification) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(
        std::make_shared<engine::timerange>(43200, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_forced),
            OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationWithDowntime) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  _host->set_scheduled_downtime_depth(30);
  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationWithFlapping) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  _host->set_is_flapping(true);
  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationWithSoftState) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  _host->set_state_type(checkable::soft);
  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification,
       SimpleNormalHostNotificationWithHardStateAcknowledged) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  _host->set_problem_has_been_acknowledged(true);
  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationAfterPreviousTooSoon) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  _host->set_problem_has_been_acknowledged(true);
  ASSERT_TRUE(host_escalation);
  _host->set_last_notification(19999);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification,
       SimpleNormalHostNotificationAfterPreviousWithNullInterval) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  _host->set_problem_has_been_acknowledged(true);
  ASSERT_TRUE(host_escalation);
  _host->set_last_notification(19500);
  _host->set_notification_number(1);
  _host->set_notification_interval(0);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationOnStateNotNotified) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  _host->set_problem_has_been_acknowledged(false);
  ASSERT_TRUE(host_escalation);
  _host->remove_notify_on(notifier::down);
  _host->set_current_state(engine::host::state_down);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification,
       SimpleNormalHostNotificationOnStateBeforeFirstNotifDelay) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  _host->set_problem_has_been_acknowledged(false);
  ASSERT_TRUE(host_escalation);
  _host->set_current_state(engine::host::state_down);
  _host->set_last_hard_state_change(20000 - 200);
  /* It is multiplicated by config->interval_length(): we set 5 for 5*60 */
  _host->set_first_notification_delay(5);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification,
       SimpleNormalHostNotificationOnStateAfterFirstNotifDelay) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7, Uuid())};
  _host->set_notification_period_ptr(tperiod.get());

  _host->set_problem_has_been_acknowledged(false);
  ASSERT_TRUE(host_escalation);
  _host->set_current_state(engine::host::state_down);
  _host->set_last_hard_state_change(20000 - 400);
  _host->set_first_notification_delay(5);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationNotifierDelayTooShort) {
  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
   * If we call time(), it is not the glibc time() function that will be called.
   */
  set_time(43200);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  for (uint32_t i = 0; i < tperiod->days.size(); ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7, Uuid())};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  /* We configure the notification interval to 2 minutes */
  _host->set_notification_interval(2);
  _host->set_notification_period_ptr(tperiod.get());
  _host->set_current_state(engine::host::state_down);
  _host->set_last_state(engine::host::state_down);
  _host->set_last_hard_state_change(43200);
  _host->set_state_type(checkable::hard);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());

  /* Only 100 seconds since the previous notification. */
  set_time(43300);
  id = _host->get_next_notification_id();
  /* Because of the notification not totally implemented, we must force the
   * notification number to be greater than 0 */
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);

  /* No notification, because the delay is too short */
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleCheck) {
  set_time(50000);
  _host->set_current_state(engine::host::state_up);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(50000);
  _host->set_state_type(checkable::hard);
  testing::internal::CaptureStdout();
  for (int i = 0; i < 3; i++) {
    // When i == 0, the state_down is soft => no notification
    // When i == 1, the state_down is soft => no notification
    // When i == 2, the state_down is hard down => notification
    set_time(50500 + i * 500);
    _host->set_last_state(_host->get_current_state());
    if (notifier::hard == _host->get_state_type())
      _host->set_last_hard_state(_host->get_current_state());
    _host->process_check_result_3x(engine::host::state_down, "The host is down",
                                   CHECK_OPTION_NONE, 0, true, 0);
  }

  for (int i = 0; i < 2; i++) {
    // When i == 0, the state_up is hard (return to up) => Recovery notification
    // When i == 1, the state_up is still here (no change) => no notification
    set_time(52500 + i * 500);
    _host->set_last_state(_host->get_current_state());
    if (notifier::hard == _host->get_state_type())
      _host->set_last_hard_state(_host->get_current_state());
    _host->process_check_result_3x(engine::host::state_up, "The host is up",
                                   CHECK_OPTION_NONE, 0, true, 0);
  }
  std::string out{testing::internal::GetCapturedStdout()};
  // Only sent when i == 2
  size_t step1{out.find("HOST ALERT: test_host;DOWN;HARD;1;")};
  // Not found because the alert is sent only one time.
  size_t step2{out.find("HOST ALERT: test_host;DOWN;HARD;1;", step1 + 1)};
  // Sent when i == 0 on the second loop.
  size_t step3{
      out.find("HOST NOTIFICATION: admin;test_host;RECOVERY (UP);cmd;")};
  ASSERT_LE(step1, step3);
  ASSERT_EQ(step2, std::string::npos);
  ASSERT_NE(step3, std::string::npos);
}

TEST_F(HostNotification, CheckFirstNotificationDelay) {
  set_time(50000);
  _host->set_current_state(engine::host::state_up);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(50000);
  _host->set_state_type(checkable::hard);
  _host->set_first_notification_delay(3);
  testing::internal::CaptureStdout();
  std::cout << "notification interval: " << _host->get_notification_interval()
            << std::endl;
  for (int i = 1; i < 40; i++) {
    // When i == 0, the state_down is soft => no notification
    // When i == 1, the state_down is soft => no notification
    // When i == 2, the state_down is hard down => notification
    std::cout << "Step " << i << ":";
    set_time(50000 + i * 60);
    _host->set_last_state(_host->get_current_state());
    if (notifier::hard == _host->get_state_type())
      _host->set_last_hard_state(_host->get_current_state());
    _host->process_check_result_3x(engine::host::state_down, "The host is down",
                                   CHECK_OPTION_NONE, 0, true, 0);
  }

  for (int i = 0; i < 3; i++) {
    // When i == 0, the state_up is hard (return to up) => Recovery notification
    // When i == 1, the state_up is still here (no change) => no notification
    std::cout << "New step " << i << std::endl;
    set_time(50600 + i * 60);
    _host->set_last_state(_host->get_current_state());
    if (notifier::hard == _host->get_state_type())
      _host->set_last_hard_state(_host->get_current_state());
    _host->process_check_result_3x(engine::host::state_up, "The host is up",
                                   CHECK_OPTION_NONE, 0, true, 0);
  }
  std::string out{testing::internal::GetCapturedStdout()};
  size_t m1{out.find("Step 5:")};
  size_t m2{out.find(" HOST NOTIFICATION: admin;test_host;DOWN;cmd;", m1 + 1)};
  size_t m3{out.find("Step 35:", m2 + 1)};
  size_t m4{out.find(" HOST NOTIFICATION: admin;test_host;DOWN;cmd;", m3 + 1)};
  size_t m5{out.find(" HOST NOTIFICATION: admin;test_host;RECOVERY (UP);cmd;",
                     m4 + 1)};
  ASSERT_NE(m5, std::string::npos);
}

// Given a host with a notification interval = 2, a
// first_delay_notification = 0, an escalation from 2 to 12 with a contactgroup
// and notification_interval = 4
// When a normal notification is sent 11 times,
// Then contacts from the escalation are notified when notification number
// is in [2,6] and are separated by at less 4*60s.
TEST_F(HostNotification, HostEscalation) {
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

  configuration::applier::hostescalation he_aply;
  configuration::hostescalation he{
      new_configuration_hostescalation("test_host", "test_cg")};
  he_aply.add_object(he);
  he_aply.expand_objects(*config);
  he_aply.resolve_object(he);

  int now{50000};
  set_time(now);

  _host->set_current_state(engine::host::state_up);
  _host->set_notification_interval(1);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(50000);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(now);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);

  testing::internal::CaptureStdout();
  for (int i = 0; i < 12; i++) {
    // When i == 0, the state_critical is soft => no notification
    // When i == 1, the state_critical is soft => no notification
    // When i == 2, the state_critical is hard down => notification
    now += 300;
    std::cout << "NOW = " << now << std::endl;
    set_time(now);
    _host->set_last_state(_host->get_current_state());
    if (notifier::hard == _host->get_state_type())
      _host->set_last_hard_state(_host->get_current_state());

    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_HOST_CHECK_RESULT;test_host;1;Down host";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  // When i == 0, the state_ok is hard (return to up) => Recovery
  // notification When i == 1, the state_ok is still here (no change) => no
  // notification
  now += 300;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']' << " PROCESS_HOST_CHECK_RESULT;test_host;0;Host up";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 50300")};
  ASSERT_NE(step1, std::string::npos);
  size_t step2{
      out.find("HOST NOTIFICATION: "
               "admin;test_host;DOWN;cmd;Down host",
               step1 + 1)};
  ASSERT_NE(step2, std::string::npos);
  size_t step3{out.find("NOW = 50600", step2 + 1)};
  ASSERT_NE(step3, std::string::npos);
  size_t step4{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;DOWN;cmd;Down host",
               step3 + 1)};
  ASSERT_NE(step4, std::string::npos);
  size_t step5{out.find("NOW = 51200", step4 + 1)};
  ASSERT_NE(step5, std::string::npos);
  size_t step6{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;DOWN;cmd;Down host",
               step5 + 1)};
  ASSERT_NE(step6, std::string::npos);
  size_t step7{out.find("NOW = 51800", step6 + 1)};
  ASSERT_NE(step7, std::string::npos);
  size_t step8{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;DOWN;cmd;Down host",
               step7 + 1)};
  ASSERT_NE(step8, std::string::npos);
  size_t step9{out.find("NOW = 52400", step8 + 1)};
  ASSERT_NE(step9, std::string::npos);
  size_t step10{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;DOWN;cmd;Down host",
               step9 + 1)};
  ASSERT_NE(step10, std::string::npos);
  size_t step11{out.find("NOW = 53000", step10 + 1)};
  ASSERT_NE(step11, std::string::npos);
  size_t step12{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;DOWN;cmd;Down host",
               step11 + 1)};
  ASSERT_NE(step12, std::string::npos);
  size_t step13{out.find("NOW = 53600", step12 + 1)};
  ASSERT_NE(step13, std::string::npos);
  size_t step14{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;DOWN;cmd;Down host",
               step13 + 1)};
  ASSERT_NE(step14, std::string::npos);
  size_t step15{
      out.find("HOST NOTIFICATION: test_contact;test_host;RECOVERY "
               "(UP);cmd;Host up",
               step14 + 1)};
  ASSERT_NE(step15, std::string::npos);
}

TEST_F(HostNotification, HostDependency) {
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

  configuration::applier::host h_aply;
  configuration::host h{new_configuration_host("dep_host", "admin", 15)};
  h_aply.add_object(h);
  h_aply.expand_objects(*config);
  h_aply.resolve_object(h);

  configuration::applier::hostdependency hd_aply;
  configuration::hostdependency hd{
      new_configuration_hostdependency("test_host", "dep_host")};
  hd_aply.expand_objects(*config);
  hd_aply.add_object(hd);
  hd_aply.resolve_object(hd);

  int now{50000};
  set_time(now);

  int w{0}, e{0};
  pre_flight_circular_check(&w, &e);

  ASSERT_EQ(w, 0);
  ASSERT_EQ(e, 0);

  _host->set_current_state(engine::host::state_up);
  _host->set_notification_interval(1);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(50000);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(now);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);

  host_map& hm(engine::host::hosts);
  host_map::iterator it{hm.find("dep_host")};
  ASSERT_NE(it, hm.end());
  engine::host* dep_host{it->second.get()};

  testing::internal::CaptureStdout();
  _host->set_last_state(engine::host::state_up);
  if (notifier::hard == _host->get_state_type())
    _host->set_last_hard_state(_host->get_current_state());

  dep_host->set_current_state(engine::host::state_up);
  dep_host->set_last_state(dep_host->get_current_state());
  if (notifier::hard == dep_host->get_state_type())
    dep_host->set_last_hard_state(dep_host->get_current_state());

  // Here, we get a notification
  for (int i = 0; i < 3; ++i) {
    dep_host->set_last_state(dep_host->get_current_state());
    if (notifier::hard == dep_host->get_state_type())
      dep_host->set_last_hard_state(dep_host->get_last_state());
    now += 300;
    set_time(now);
    std::ostringstream oss;
    oss << '[' << now << ']'
        << " PROCESS_HOST_CHECK_RESULT;dep_host;1;host down";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
    dep_host->set_last_state(dep_host->get_current_state());
  }

  dep_host->set_last_state(dep_host->get_current_state());
  if (notifier::hard == dep_host->get_state_type())
    dep_host->set_last_hard_state(dep_host->get_last_state());
  now += 300;
  set_time(now);
  std::ostringstream oss;
  oss << '[' << now << ']' << " PROCESS_HOST_CHECK_RESULT;dep_host;0;host up";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  dep_host->set_last_state(dep_host->get_current_state());

  for (int i = 0; i < 3; ++i) {
    _host->set_last_state(_host->get_current_state());
    if (notifier::hard == _host->get_state_type())
      _host->set_last_hard_state(_host->get_last_state());
    now += 300;
    set_time(now);
    std::ostringstream oss;
    oss << '[' << now << ']'
        << " PROCESS_HOST_CHECK_RESULT;test_host;1;host down";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  // Here, we won't have any notification
  for (int i = 0; i < 3; ++i) {
    dep_host->set_last_state(dep_host->get_current_state());
    if (notifier::hard == dep_host->get_state_type())
      dep_host->set_last_hard_state(dep_host->get_last_state());
    now += 300;
    set_time(now);
    std::ostringstream oss;
    oss << '[' << now << ']'
        << " PROCESS_HOST_CHECK_RESULT;dep_host;1;how down";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
    dep_host->set_last_state(dep_host->get_current_state());
  }

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << "out = " << out << std::endl;
  size_t step1{
      out.find("HOST NOTIFICATION: admin;dep_host;DOWN;cmd;host down")};
  size_t step2{
      out.find("HOST NOTIFICATION: admin;dep_host;RECOVERY (UP);cmd;host up",
               step1 + 1)};
  size_t step3{out.find("HOST NOTIFICATION: admin;test_host;DOWN;cmd;host down",
                        step2 + 1)};
  size_t step4{out.find("HOST NOTIFICATION: admin;dep_host;DOWN;cmd;host down",
                        step3 + 1)};
  ASSERT_NE(step3, std::string::npos);
  ASSERT_EQ(step4, std::string::npos);
}

// Given a host with a notification interval = 1, a
// first_delay_notification = 0, an escalation from 1 to 0 with a contactgroup
// and notification_interval = 9
// When a normal notification is sent 1 time, and then a recovery notification
// is sent 1 time
// Then both are sent to contacts from the escalation.
TEST_F(HostNotification, HostEscalationOneTime) {
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

  configuration::applier::hostescalation he_aply;
  configuration::hostescalation he{
      new_configuration_hostescalation("test_host", "test_cg", 1, 0)};
  he_aply.add_object(he);
  he_aply.expand_objects(*config);
  he_aply.resolve_object(he);

  int now{50000};
  set_time(now);

  _host->set_current_state(engine::host::state_up);
  _host->set_notification_interval(1);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(50000);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(now);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);

  testing::internal::CaptureStdout();
  now += 300;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  _host->set_last_state(_host->get_current_state());
  if (notifier::hard == _host->get_state_type())
    _host->set_last_hard_state(_host->get_current_state());

  std::ostringstream oss;
  // std::time_t now{std::time(nullptr)};
  oss << '[' << now << ']'
      << " PROCESS_HOST_CHECK_RESULT;test_host;1;Down host";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  // When i == 0, the state_ok is hard (return to up) => Recovery
  // notification When i == 1, the state_ok is still here (no change) => no
  // notification
  for (int i = 0; i < 2; i++) {
    now += 300;
    set_time(now);
    std::ostringstream oss;
    oss << '[' << now << ']'
        << " PROCESS_HOST_CHECK_RESULT;test_host;0;Host up";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 50300")};
  ASSERT_NE(step1, std::string::npos);
  size_t step2{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;DOWN;cmd;Down host",
               step1 + 1)};
  ASSERT_NE(step2, std::string::npos);
  size_t step3{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;RECOVERY (UP);cmd;Host up",
               step2 + 1)};
  ASSERT_NE(step3, std::string::npos);
  size_t step4{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;UP;cmd;Host up",
               step3 + 1)};
  ASSERT_EQ(step4, std::string::npos);
}

// Given a host with a notification interval = 1, a
// first_delay_notification = 0, an escalation from 1 to 0 with a contactgroup
// and notification_interval = 0
// When a normal notification is sent 1 time, and then a recovery notification
// is sent 1 time
// Then both are sent to contacts from the escalation.
TEST_F(HostNotification, HostEscalationOneTimeNotifInter0) {
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

  configuration::applier::hostescalation he_aply;
  configuration::hostescalation he{
      new_configuration_hostescalation("test_host", "test_cg", 1, 0, 0)};
  he_aply.add_object(he);
  he_aply.expand_objects(*config);
  he_aply.resolve_object(he);

  int now{50000};
  set_time(now);

  _host->set_current_state(engine::host::state_up);
  _host->set_notification_interval(1);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(50000);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(now);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);

  testing::internal::CaptureStdout();
  now += 300;
  std::cout << "NOW = " << now << std::endl;
  set_time(now);
  _host->set_last_state(_host->get_current_state());
  if (notifier::hard == _host->get_state_type())
    _host->set_last_hard_state(_host->get_current_state());

  std::ostringstream oss;
  // std::time_t now{std::time(nullptr)};
  oss << '[' << now << ']'
      << " PROCESS_HOST_CHECK_RESULT;test_host;1;Down host";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  // When i == 0, the state_ok is hard (return to up) => Recovery
  // notification When i == 1, the state_ok is still here (no change) => no
  // notification
  for (int i = 0; i < 2; i++) {
    now += 300;
    set_time(now);
    std::ostringstream oss;
    oss << '[' << now << ']'
        << " PROCESS_HOST_CHECK_RESULT;test_host;0;Host up";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();
  }

  std::string out{testing::internal::GetCapturedStdout()};
  std::cout << out << std::endl;
  size_t step1{out.find("NOW = 50300")};
  ASSERT_NE(step1, std::string::npos);
  size_t step2{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;DOWN;cmd;Down host",
               step1 + 1)};
  ASSERT_NE(step2, std::string::npos);
  size_t step3{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;RECOVERY (UP);cmd;Host up",
               step2 + 1)};
  ASSERT_NE(step3, std::string::npos);
  size_t step4{
      out.find("HOST NOTIFICATION: "
               "test_contact;test_host;UP;cmd;Host up",
               step3 + 1)};
  ASSERT_EQ(step4, std::string::npos);
}

// Given a host with a notification interval = 1, a
// first_delay_notification = 0, an escalation from 1 to 0 with a contactgroup
// and notification_interval = 0
// When a normal notification is sent 1 time, and then a recovery notification
// is sent 1 time
// Then both are sent to contacts from the escalation.
TEST_F(HostNotification, HostEscalationRetention) {
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

  configuration::applier::hostescalation he_aply;
  configuration::hostescalation he{
      new_configuration_hostescalation("test_host", "test_cg", 1, 0, 0)};
  he_aply.add_object(he);
  he_aply.expand_objects(*config);
  he_aply.resolve_object(he);

  int now{50000};
  set_time(now);

  _host->set_current_state(engine::host::state_up);
  _host->set_notification_interval(1);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(50000);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);
  _host->set_last_hard_state(engine::host::state_up);
  _host->set_last_hard_state_change(now);
  _host->set_state_type(checkable::hard);
  _host->set_accept_passive_checks(true);

  now += 300;
  set_time(now);
  _host->set_last_state(_host->get_current_state());
  if (notifier::hard == _host->get_state_type())
    _host->set_last_hard_state(_host->get_current_state());

  std::ostringstream oss;
  oss << '[' << now << ']'
      << " PROCESS_HOST_CHECK_RESULT;test_host;1;Down host";
  std::string cmd{oss.str()};
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();

  oss.str("");
  dump::host(oss, *_host);
  std::string retention{oss.str()};

  std::size_t pos =
      retention.find("notification_0=") + strlen("notification_0=");
  std::size_t end = retention.find("\n", pos + 1);
  std::string notification0 = retention.substr(pos, end - pos);
  _host->set_notification(0, notification0);
  oss.str("");

  dump::host(oss, *_host);
  retention = oss.str();
  pos = retention.find("notification_0=") + strlen("notification_0=");
  end = retention.find("\n", pos + 1);
  std::string notification1 = retention.substr(pos, end - pos);

  ASSERT_EQ(notification0, notification1);

  notification0 =
      "type: 1, author: admin, options: 5, escalated: 1, id: 28, number: 2, "
      "interval: 3";
  _host->set_notification(0, notification0);
  oss.str("");

  dump::host(oss, *_host);
  retention = oss.str();
  pos = retention.find("notification_0=") + strlen("notification_0=");
  end = retention.find("\n", pos + 1);
  notification1 = retention.substr(pos, end - pos);

  ASSERT_EQ(notification0, notification1);
}
