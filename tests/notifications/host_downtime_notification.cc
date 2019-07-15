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

#include <cstring>
#include <iostream>
#include <memory>
#include <time.h>
#include "../test_engine.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/clib.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/hostescalation.hh"
#include "com/centreon/engine/timezone_manager.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class HostDowntimeNotification : public TestEngine {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    if (!config)
      config = new configuration::state;
    timezone_manager::load();
    configuration::applier::state::load();  // Needed to create a contact
    // Do not unload this in the tear down function, it is done by the
    // other unload function... :-(
    checks::checker::load();

    configuration::applier::contact ct_aply;
    configuration::contact ctct{new_configuration_contact("admin", true)};
    ct_aply.add_object(ctct);
    ct_aply.expand_objects(*config);
    ct_aply.resolve_object(ctct);

    configuration::host hst{new_configuration_host("test_host", "admin")};
    configuration::applier::host aply;
    aply.add_object(hst);
    aply.resolve_object(hst);

    host_map const& hm{engine::host::hosts};
    _host = hm.begin()->second;
    _host->set_current_state(engine::host::state_up);
    _host->set_state_type(checkable::hard);
    _host->set_problem_has_been_acknowledged(false);
    _host->set_notify_on(static_cast<uint32_t>(-1));
  }

  void TearDown() override {
    configuration::applier::state::unload();
    checks::checker::unload();
    delete config;
    config = nullptr;
    timezone_manager::unload();
    com::centreon::logging::engine::unload();
    clib::unload();
  }

 protected:
  std::shared_ptr<engine::host> _host;
};

// Given a host UP with a contact admin configured to get downtime notifications
// When the host is in downtime
// Then it can throw a downtimestart notification received by the contact
// When it is no more in downtime, it can throw a downtimeend notification
// also received by the contact.
TEST_F(HostDowntimeNotification, SimpleHostDowntime) {
  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
   * If we call time(), it is not the glibc time() function that will be called.
   */
  set_time(43000);
  _host->set_last_hard_state_change(43000);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  for (size_t i = 0; i < tperiod->days.size(); ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->set_notification_period_ptr(tperiod.get());
  testing::internal::CaptureStdout();
  ASSERT_EQ(_host->notify(notifier::reason_downtimestart, "", "",
                          notifier::notification_option_none),
            OK);
  _host->set_scheduled_downtime_depth(2);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
  set_time(43500);
  _host->set_scheduled_downtime_depth(0);
  ASSERT_EQ(_host->notify(notifier::reason_downtimeend, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 2, _host->get_next_notification_id());

  std::string out{testing::internal::GetCapturedStdout()};
  size_t step1{out.find("HOST NOTIFICATION: admin;test_host;DOWNTIMESTART (UP);cmd;")};
  size_t step2{out.find("HOST NOTIFICATION: admin;test_host;DOWNTIMEEND (UP);cmd;")};
  ASSERT_NE(step1, std::string::npos);
  ASSERT_NE(step2, std::string::npos);
  ASSERT_LE(step1, step2);
}

// Given a host UP with a contact admin configured to get downtime notifications
// When the host is in downtime
// Then it can throw a downtimestart notification received by the contact
// When it is no more in downtime, it can throw a downtimeend notification
// also received by the contact.
TEST_F(HostDowntimeNotification, SimpleHostDowntimeWithContactNotReceivingNotif) {
  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
   * If we call time(), it is not the glibc time() function that will be called.
   */
  set_time(43000);
  _host->set_last_hard_state_change(43000);
  contact_map::iterator it{engine::contact::contacts.find("admin")};
  engine::contact* ctct{it->second.get()};
  ctct->set_notify_on(notifier::host_notification,
                      notifier::none);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  for (size_t i = 0; i < tperiod->days.size(); ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->set_notification_period_ptr(tperiod.get());
  testing::internal::CaptureStdout();
  ASSERT_EQ(_host->notify(notifier::reason_downtimestart, "", "",
                          notifier::notification_option_none),
            OK);
  _host->set_scheduled_downtime_depth(2);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
  set_time(43500);
  _host->set_scheduled_downtime_depth(0);
  ASSERT_EQ(_host->notify(notifier::reason_downtimeend, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 2, _host->get_next_notification_id());
  std::string out{testing::internal::GetCapturedStdout()};
  size_t step1{out.find("HOST NOTIFICATION: admin;test_host;DOWNTIMESTART (UP);cmd;")};
  size_t step2{out.find("HOST NOTIFICATION: admin;test_host;DOWNTIMEEND (UP);cmd;")};
  ASSERT_EQ(step1, std::string::npos);
  ASSERT_EQ(step2, std::string::npos);
}

//// Given a host UP
//// When it is flapping
//// Then it can throw a flappingstart notification followed by a recovery
//// notification.
//// When a second flappingstart notification is sent
//// Then no notification is sent (because already sent).
//TEST_F(HostDowntimeNotification, SimpleHostDowntimeStartTwoTimes) {
//  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
//   * If we call time(), it is not the glibc time() function that will be called.
//   */
//  set_time(43000);
//  _host->set_notification_interval(2);
//  std::unique_ptr<engine::timeperiod> tperiod{
//      new engine::timeperiod("tperiod", "alias")};
//  for (uint32_t i = 0; i < tperiod->days.size(); ++i)
//    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));
//
//  std::unique_ptr<engine::hostescalation> host_escalation{
//      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};
//
//  ASSERT_TRUE(host_escalation);
//  uint64_t id{_host->get_next_notification_id()};
//  _host->notification_period_ptr = tperiod.get();
//  _host->set_is_flapping(true);
//  ASSERT_EQ(_host->notify(notifier::reason_flappingstart, "", "",
//                          notifier::notification_option_none),
//            OK);
//  ASSERT_EQ(id + 1, _host->get_next_notification_id());
//
//  set_time(43050);
//  /* Notification already sent, no notification should be sent. */
//  ASSERT_EQ(_host->notify(notifier::reason_flappingstart, "", "",
//                          notifier::notification_option_none),
//            OK);
//  ASSERT_EQ(id + 1, _host->get_next_notification_id());
//}
//
//// Given a host UP
//// When it is flapping
//// Then it can throw a flappingstart notification followed by a recovery
//// notification.
//// When a flappingstop notification is sent
//// Then it is sent.
//// When a second flappingstop notification is sent
//// Then nothing is sent.
//TEST_F(HostDowntimeNotification, SimpleHostDowntimeStopTwoTimes) {
//  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
//   * If we call time(), it is not the glibc time() function that will be called.
//   */
//  set_time(43000);
//  _host->set_notification_interval(2);
//  std::unique_ptr<engine::timeperiod> tperiod{
//      new engine::timeperiod("tperiod", "alias")};
//  for (uint32_t i = 0; i < tperiod->days.size(); ++i)
//    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));
//
//  std::unique_ptr<engine::hostescalation> host_escalation{
//      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};
//
//  ASSERT_TRUE(host_escalation);
//  uint64_t id{_host->get_next_notification_id()};
//  _host->notification_period_ptr = tperiod.get();
//  _host->set_is_flapping(true);
//  ASSERT_EQ(_host->notify(notifier::reason_flappingstart, "", "",
//                          notifier::notification_option_none),
//            OK);
//  ASSERT_EQ(id + 1, _host->get_next_notification_id());
//
//  set_time(43050);
//  /* Downtimestop notification: sent. */
//  ASSERT_EQ(_host->notify(notifier::reason_flappingstop, "", "",
//                          notifier::notification_option_none),
//            OK);
//  ASSERT_EQ(id + 2, _host->get_next_notification_id());
//
//  set_time(43100);
//  /* Second flappingstop notification: not sent. */
//  ASSERT_EQ(_host->notify(notifier::reason_flappingstop, "", "",
//                          notifier::notification_option_none),
//            OK);
//  ASSERT_EQ(id + 2, _host->get_next_notification_id());
//}
