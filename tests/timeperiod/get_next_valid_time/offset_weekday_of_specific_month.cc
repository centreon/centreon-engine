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

class GetNextValidTimeOffsetWeekdayOfSpecificMonthTest
    : public ::testing::Test {
 public:

  void default_data_set() {
    _creator.new_timeperiod();
    daterange* dr(NULL);
    // tuesday 4 october 10:45-14:25
    dr = _creator.new_offset_weekday_of_specific_month(9, 2, 4, 9, 2, 4);
    _creator.new_timerange(10, 45, 14, 25, dr);
    // thursday 4 october - friday 4 october 08:30-12:30,18:30-21:15
    dr = _creator.new_offset_weekday_of_specific_month(9, 4, 4, 9, 5, 4);
    _creator.new_timerange(8, 30, 12, 30, dr);
    _creator.new_timerange(18, 30, 21, 15, dr);
  }

  void negative_offset_data_set() {
    _creator.new_timeperiod();
    daterange* dr(NULL);
    // tuesday -4 october 10:45-14:25
    dr = _creator.new_offset_weekday_of_specific_month(9, 2, -4, 9, 2, -4);
    _creator.new_timerange(10, 45, 14, 25, dr);
    // thursday -3 october - friday -3 october 08:30-12:30,18:30,21:15
    dr = _creator.new_offset_weekday_of_specific_month(9, 4, -3, 9, 5, -3);
    _creator.new_timerange(8, 30, 12, 30, dr);
    _creator.new_timerange(18, 30, 21, 15, dr);
  }

 protected:
  timeperiod_creator _creator;
};

// Given a timeperiod configured with offset weekdays of specific month
// And we are earlier than these dates
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next date's timerange
TEST_F(GetNextValidTimeOffsetWeekdayOfSpecificMonthTest, BeforeDateRanges) {
  default_data_set();
  time_t now(strtotimet("2016-10-24 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-25 10:45:00"));
}

// Given a timeperiod configured with offset weekdays of specific month
// And we are between two offset weekdays ranges
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next daterange's timerange
TEST_F(GetNextValidTimeOffsetWeekdayOfSpecificMonthTest, BetweenDateRanges) {
  default_data_set();
  time_t now(strtotimet("2016-10-26 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-27 08:30:00"));
}

// Given a timeperiod configured with offset weekdays of specific month
// And we are within an offset weekday daterange
// When get_next_valid_time() is called
// Then the next valid time is now
TEST_F(GetNextValidTimeOffsetWeekdayOfSpecificMonthTest, WithinRange) {
  default_data_set();
  time_t now(strtotimet("2016-10-28 20:59:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, now);
}

// Given a timeperiod configured with offset weekdays of specific month
// And we are after these dates in the year
// When get_next_valid_time() is called
// Then the next valid time is the first month with offset weekdays in next year
TEST_F(GetNextValidTimeOffsetWeekdayOfSpecificMonthTest, AfterRanges) {
  default_data_set();
  time_t now(strtotimet("2016-10-30 13:37:42"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2017-10-24 10:45:00"));
}

// Given a timeperiod configured with negative offset weekdays of specific month
// And we are earlier than these dates
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next date's timerange
TEST_F(GetNextValidTimeOffsetWeekdayOfSpecificMonthTest,
       BeforeNegativeDateRanges) {
  negative_offset_data_set();
  time_t now(strtotimet("2016-10-03 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-04 10:45:00"));
}

// Given a timeperiod configured with negative offset weekdays of specific month
// And we are between two offset weekdays ranges
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next daterange's timerange
TEST_F(GetNextValidTimeOffsetWeekdayOfSpecificMonthTest,
       BetweenNegativeDateRanges) {
  negative_offset_data_set();
  time_t now(strtotimet("2016-10-08 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-13 08:30:00"));
}

// Given a timeperiod configured with negative offset weekdays of specific month
// And we are within an offset weekday daterange
// When get_next_valid_time() is called
// Then the next valid time is now
TEST_F(GetNextValidTimeOffsetWeekdayOfSpecificMonthTest, WithinNegativeRange) {
  negative_offset_data_set();
  time_t now(strtotimet("2016-10-13 20:59:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, now);
}

// Given a timeperiod configured with negative offset weekdays of specific month
// And we are after these dates in the year
// When get_next_valid_time() is called
// Then the next valid time is the first month with offset weekdays in next year
TEST_F(GetNextValidTimeOffsetWeekdayOfSpecificMonthTest, AfterNegativeRanges) {
  negative_offset_data_set();
  time_t now(strtotimet("2016-10-30 13:37:42"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2017-10-10 10:45:00"));
}
