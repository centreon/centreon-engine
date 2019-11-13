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
#include <cstring>
#include "com/centreon/clib.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "tests/timeperiod/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class GetNextValidTimeOffsetWeekdayOfGenericMonthTest : public ::testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    configuration::applier::state::load();
  }

  void TearDown() override {
    configuration::applier::state::unload();
    com::centreon::logging::engine::unload();
    clib::unload();
  }

  void default_data_set() {
    _creator.new_timeperiod();
    daterange* dr(NULL);
    // tuesday 4 october 10:45-14:25
    dr = _creator.new_offset_weekday_of_generic_month(2, 4, 2, 4);
    _creator.new_timerange(10, 45, 14, 25, dr);
    // thursday 4 october - friday 4 october 08:30-12:30,18:30-21:15
    dr = _creator.new_offset_weekday_of_generic_month(4, 4, 5, 4);
    _creator.new_timerange(8, 30, 12, 30, dr);
    _creator.new_timerange(18, 30, 21, 15, dr);
  }

  void negative_offset_data_set() {
    _creator.new_timeperiod();
    daterange* dr(NULL);
    // tuesday -4 10:45-14:25
    dr = _creator.new_offset_weekday_of_generic_month(2, -4, 2, -4);
    _creator.new_timerange(10, 45, 14, 25, dr);
    // thursday -3 - friday -3 08:30-12:30,18:30,21:15
    dr = _creator.new_offset_weekday_of_generic_month(4, -3, 5, -3);
    _creator.new_timerange(8, 30, 12, 30, dr);
    _creator.new_timerange(18, 30, 21, 15, dr);
  }

 protected:
  timeperiod_creator _creator;
};

// Given a timeperiod configured with offset weekdays of generic month
// And we are earlier than these dates
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next date's timerange
TEST_F(GetNextValidTimeOffsetWeekdayOfGenericMonthTest, BeforeDateRanges) {
  default_data_set();
  time_t now(strtotimet("2016-10-24 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-25 10:45:00"));
}

// Given a timeperiod configured with offset weekdays of generic month
// And we are between two offset weekdays ranges
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next daterange's timerange
TEST_F(GetNextValidTimeOffsetWeekdayOfGenericMonthTest, BetweenDateRanges) {
  default_data_set();
  time_t now(strtotimet("2016-10-26 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-27 08:30:00"));
}

// Given a timeperiod configured with offset weekdays of generic month
// And we are within an offset weekday daterange
// When get_next_valid_time() is called
// Then the next valid time is now
TEST_F(GetNextValidTimeOffsetWeekdayOfGenericMonthTest, WithinRange) {
  default_data_set();
  time_t now(strtotimet("2016-10-28 20:59:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, now);
}

// Given a timeperiod configured with offset weekdays of generic month
// And we are after these dates in the year
// When get_next_valid_time() is called
// Then the next valid time is the first month with offset weekdays in next
// month
TEST_F(GetNextValidTimeOffsetWeekdayOfGenericMonthTest, AfterRanges) {
  default_data_set();
  time_t now(strtotimet("2016-10-30 13:37:42"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-22 10:45:00"));
}

// Given a timeperiod configured with negative offset weekdays of generic month
// And we are earlier than these dates
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next date's timerange
TEST_F(GetNextValidTimeOffsetWeekdayOfGenericMonthTest,
       BeforeNegativeDateRanges) {
  negative_offset_data_set();
  time_t now(strtotimet("2016-10-03 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-04 10:45:00"));
}

// Given a timeperiod configured with negative offset weekdays of generic month
// And we are between two offset weekdays ranges
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next daterange's timerange
TEST_F(GetNextValidTimeOffsetWeekdayOfGenericMonthTest,
       BetweenNegativeDateRanges) {
  negative_offset_data_set();
  time_t now(strtotimet("2016-10-08 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-13 08:30:00"));
}

// Given a timeperiod configured with negative offset weekdays of generic month
// And we are within an offset weekday daterange
// When get_next_valid_time() is called
// Then the next valid time is now
TEST_F(GetNextValidTimeOffsetWeekdayOfGenericMonthTest, WithinNegativeRange) {
  negative_offset_data_set();
  time_t now(strtotimet("2016-10-13 20:59:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, now);
}

// Given a timeperiod configured with negative offset weekdays of generic month
// And we are after these dates in the year
// When get_next_valid_time() is called
// Then the next valid time is the first month with offset weekdays in next
// month
TEST_F(GetNextValidTimeOffsetWeekdayOfGenericMonthTest, AfterNegativeRanges) {
  negative_offset_data_set();
  time_t now(strtotimet("2016-10-30 13:37:42"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-08 10:45:00"));
}
