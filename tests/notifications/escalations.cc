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

class HostEscalation : public ::testing::Test {
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
    umap<uint64_t, std::shared_ptr<engine::host> > const& hm{
        configuration::applier::state::instance().hosts()};
    _host = hm.begin()->second;
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

TEST_F(HostEscalation, SimpleNormalHostNotification) {
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
  _host->notification_period_ptr = tperiod.release();
  ASSERT_EQ(_host->notify(notifier::notification_normal, "", "", notifier::notification_option_none), OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
}

TEST_F(HostEscalation, SimpleNormalHostNotificationOutsideTimeperiod) {
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

TEST_F(HostEscalation, SimpleNormalHostNotificationForcedNotification) {
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  set_time(20000);

  uint64_t id{_host->get_next_notification_id()};
  for (int i = 0; i < 7; ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(43200, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "", 7)};

  ASSERT_TRUE(host_escalation);
  ASSERT_EQ(
      _host->notify(
          notifier::notification_normal, "", "", notifier::notification_option_forced),
      OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
}
