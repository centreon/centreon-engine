/*
** Copyright 2016 Centreon
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

#include <gtest/gtest.h>
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "tests/timeperiod/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class GetNextValidTimeNormalWeekdayTest : public ::testing::Test {
 public:
  void SetUp() override {
    configuration::applier::state::load();
  }

  void TearDown() override {
    configuration::applier::state::unload();
  }

  void default_data_set() {
    _creator.new_timeperiod();
    // tuesday 10:30-11:45,18:30-23:30
    _creator.new_timerange(10, 30, 11, 45, 2);
    _creator.new_timerange(18, 30, 23, 30, 2);
    // thursday 12:00-13:00
    _creator.new_timerange(12, 0, 13, 0, 4);
    // friday 08:30-09:15,12:15-12:45,18:30-19:45
    _creator.new_timerange(8, 30, 9, 15, 5);
    _creator.new_timerange(12, 15, 12, 45, 5);
    _creator.new_timerange(18, 30, 19, 45, 5);
  }

 protected:
  timeperiod_creator _creator;
};

// Given a timeperiod configured with normal weekdays
// And we are earlier in the week than the weekdays
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the weekday's timerange
TEST_F(GetNextValidTimeNormalWeekdayTest, BeforeWeekdays) {
  default_data_set();
  time_t now(strtotimet("2016-10-24 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-25 10:30:00"));
}

// Given a timeperiod configured with normal weekdays
// And we are between two weekdays
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next weekday's timerange
TEST_F(GetNextValidTimeNormalWeekdayTest, BetweenWeekdays) {
  default_data_set();
  time_t now(strtotimet("2016-10-26 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-27 12:00:00"));
}

// Given a timeperiod configured with normal weekdays
// And we are within a weekdday
// When get_next_valid_time() is called
// Then the next valid time is now
TEST_F(GetNextValidTimeNormalWeekdayTest, WithinWeekday) {
  default_data_set();
  time_t now(strtotimet("2016-10-28 18:36:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, now);
}

// Given a timeperiod configured with normal weekdays
// And we are just before the end of a weekday
// When get_next_valid_time() is called
// Then the next valid time is now
TEST_F(GetNextValidTimeNormalWeekdayTest, JustBeforeEndWeekday) {
  default_data_set();
  time_t now(strtotimet("2016-10-27 12:59:59"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, now);
}

// Given a timeperiod configured with normal weekdays
// And we are just before the end of the weekdays
// When get_next_valid_time() is called
// Then the next valid time is now
TEST_F(GetNextValidTimeNormalWeekdayTest, JustBeforeEndWeekdays) {
  default_data_set();
  time_t now(strtotimet("2016-10-28 19:44:59"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, now);
}

// Given a timeperiod configured with normal weekdays
// And we are after the weekdays but in the same week
// When get_next_valid_time() is called
// Then the next valid time is the next week's first week day
TEST_F(GetNextValidTimeNormalWeekdayTest, AfterWeekdays) {
  default_data_set();
  time_t now(strtotimet("2016-10-29 13:37:42"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-01 10:30:00"));
}
