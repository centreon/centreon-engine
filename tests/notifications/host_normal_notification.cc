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

#include <cstring>
#include <regex>
#include <iostream>
#include <memory>
#include <gtest/gtest.h>
#include <time.h>
#include "../test_engine.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include <com/centreon/process_manager.hh>
#include "com/centreon/engine/error.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/timezone_manager.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class HostNotification : public TestEngine {
 public:
  void SetUp() override {
    if (!config)
      config = new configuration::state;
    timezone_manager::load();
    configuration::applier::state::load();  // Needed to create a contact
    // Do not unload this in the tear down function, it is done by the
    // other unload function... :-(

    configuration::applier::contact ct_aply;
    configuration::contact ctct{valid_contact_config()};
    process_manager::load();
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
    configuration::applier::state::unload();
    delete config;
    config = NULL;
    timezone_manager::unload();
    process_manager::unload();
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
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->notification_period_ptr = tperiod.get();
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
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->notification_period_ptr = tperiod.get();
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
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->set_notifications_enabled(false);
  _host->notification_period_ptr = tperiod.get();
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
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(43200, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationForcedWithNotificationDisabled) {
  config->enable_notifications(false);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(43200, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

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
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(43200, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

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
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

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
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

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
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationWithHardStateAcknowledged) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

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
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

  _host->set_problem_has_been_acknowledged(true);
  ASSERT_TRUE(host_escalation);
  _host->set_last_notification(19999);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationAfterPreviousWithNullInterval) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

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
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

  _host->set_problem_has_been_acknowledged(false);
  ASSERT_TRUE(host_escalation);
  _host->remove_notify_on(notifier::down);
  _host->set_current_state(engine::host::state_down);
  ASSERT_EQ(_host->notify(notifier::reason_normal, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationOnStateBeforeFirstNotifDelay) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

  _host->set_problem_has_been_acknowledged(false);
  ASSERT_TRUE(host_escalation);
  _host->set_current_state(engine::host::state_down);
  _host->set_last_hard_state_change(20000 - 200);
  /* It is multiplicated by config->interval_length(): we set 5 for 5*60 */
  _host->set_first_notification_delay(5);
  ASSERT_EQ(
      _host->notify(
          notifier::reason_normal, "", "", notifier::notification_option_none),
      OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
}

TEST_F(HostNotification, SimpleNormalHostNotificationOnStateAfterFirstNotifDelay) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};
  _host->notification_period_ptr = tperiod.get();

  _host->set_problem_has_been_acknowledged(false);
  ASSERT_TRUE(host_escalation);
  _host->set_current_state(engine::host::state_down);
  _host->set_last_hard_state_change(20000 - 400);
  _host->set_first_notification_delay(5);
  ASSERT_EQ(
      _host->notify(
          notifier::reason_normal, "", "", notifier::notification_option_none),
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
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  /* We configure the notification interval to 2 minutes */
  _host->set_notification_interval(2);
  _host->notification_period_ptr = tperiod.get();
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
    _host->process_check_result_3x(engine::host::state_down,
        "The host is down",
        CHECK_OPTION_NONE,
        0,
        true,
        0);
  }

  for (int i = 0; i < 2; i++) {
    // When i == 0, the state_up is hard (return to up) => Recovery notification
    // When i == 1, the state_up is still here (no change) => no notification
    set_time(52500 + i * 500);
    _host->set_last_state(_host->get_current_state());
    if (notifier::hard == _host->get_state_type())
      _host->set_last_hard_state(_host->get_current_state());
    _host->process_check_result_3x(engine::host::state_up,
        "The host is up",
        CHECK_OPTION_NONE,
        0,
        true,
        0);
  }
  std::string out{testing::internal::GetCapturedStdout()};
  // Only sent when i == 2
  size_t step1{out.find("HOST ALERT: test_host;DOWN;HARD;1;")};
  // Not found because the alert is sent only one time.
  size_t step2{out.find("HOST ALERT: test_host;DOWN;HARD;1;", step1 + 1)};
  // Sent when i == 0 on the second loop.
  size_t step3{out.find("HOST NOTIFICATION: admin;test_host;RECOVERY (UP);cmd;")};
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
  std::cout << "notification interval: " << _host->get_notification_interval() << std::endl;
  for (int i = 1; i < 40; i++) {
    // When i == 0, the state_down is soft => no notification
    // When i == 1, the state_down is soft => no notification
    // When i == 2, the state_down is hard down => notification
    std::cout << "Step " << i << ":";
    set_time(50000 + i * 60);
    _host->set_last_state(_host->get_current_state());
    if (notifier::hard == _host->get_state_type())
      _host->set_last_hard_state(_host->get_current_state());
    _host->process_check_result_3x(engine::host::state_down,
        "The host is down",
        CHECK_OPTION_NONE,
        0,
        true,
        0);
  }

  for (int i = 0; i < 3; i++) {
    // When i == 0, the state_up is hard (return to up) => Recovery notification
    // When i == 1, the state_up is still here (no change) => no notification
    std::cout << "New step " << i << std::endl;
    set_time(50600 + i * 60);
    _host->set_last_state(_host->get_current_state());
    if (notifier::hard == _host->get_state_type())
      _host->set_last_hard_state(_host->get_current_state());
    _host->process_check_result_3x(engine::host::state_up,
        "The host is up",
        CHECK_OPTION_NONE,
        0,
        true,
        0);
  }
  std::string out{testing::internal::GetCapturedStdout()};
  size_t m1{out.find("Step 5:")};
  size_t m2{out.find(" HOST NOTIFICATION: admin;test_host;DOWN;cmd;", m1 + 1)};
  size_t m3{out.find("Step 35:", m2 + 1)};
  size_t m4{out.find(" HOST NOTIFICATION: admin;test_host;DOWN;cmd;", m3 + 1)};
  size_t m5{out.find(" HOST NOTIFICATION: admin;test_host;RECOVERY (UP);cmd;", m4 + 1)};
  ASSERT_NE(m5, std::string::npos);
}
