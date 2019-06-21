/*
** Copyright 2017-2018 Centreon
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

#include <cstring>
#include <iostream>
#include <memory>
#include <gtest/gtest.h>
#include <time.h>
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/timezone_manager.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class HostNotification : public ::testing::Test {
 public:
  void SetUp() override {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
    // Do not unload this in the tear down function, it is done by the
    // other unload function... :-(
    timezone_manager::load();

    configuration::applier::host hst_aply;
    configuration::host hst;
    hst.parse("host_name", "test_host");
    hst.parse("address", "127.0.0.1");
    hst.parse("_HOST_ID", "12");
    hst_aply.add_object(hst);
    host_map const& hm{engine::host::hosts};
    _host = hm.begin()->second;
    _host->set_current_state(engine::host::state_up);
    _host->set_state_type(checkable::hard);
    _host->set_problem_has_been_acknowledged(false);
    _host->set_notify_on(static_cast<uint32_t>(-1));
  }

  void TearDown() override {
    timezone_manager::unload();
    configuration::applier::state::unload();
    delete config;
    config = NULL;
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
  ASSERT_EQ(_host->notify(notifier::notification_normal, "", "", notifier::notification_option_none), OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
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
  ASSERT_EQ(_host->notify(notifier::notification_normal, "", "", notifier::notification_option_none), OK);
  ASSERT_EQ(id, _host->get_next_notification_id());
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
  ASSERT_EQ(
      _host->notify(
          notifier::notification_normal, "", "", notifier::notification_option_forced),
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
  ASSERT_EQ(
      _host->notify(
          notifier::notification_normal, "", "", notifier::notification_option_none),
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
  ASSERT_EQ(
      _host->notify(
          notifier::notification_normal, "", "", notifier::notification_option_none),
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
  ASSERT_EQ(
      _host->notify(
          notifier::notification_normal, "", "", notifier::notification_option_none),
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
  ASSERT_EQ(
      _host->notify(
          notifier::notification_normal, "", "", notifier::notification_option_none),
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
  ASSERT_EQ(
      _host->notify(
          notifier::notification_normal, "", "", notifier::notification_option_none),
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
  ASSERT_EQ(
      _host->notify(
          notifier::notification_normal, "", "", notifier::notification_option_none),
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
  ASSERT_EQ(
      _host->notify(
          notifier::notification_normal, "", "", notifier::notification_option_none),
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
          notifier::notification_normal, "", "", notifier::notification_option_none),
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
          notifier::notification_normal, "", "", notifier::notification_option_none),
      OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
}
