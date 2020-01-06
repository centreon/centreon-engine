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

class GetNextValidTimeGenericMonthDateTest : public ::testing::Test {
 public:

  void default_data_set() {
    _creator.new_timeperiod();
    daterange* dr(NULL);
    // day 25 10:45-14:25
    dr = _creator.new_generic_month_date(25, 25);
    _creator.new_timerange(10, 45, 14, 25, dr);

    // day 27-day 28 08:30-12:30,18:30-21:15
    dr = _creator.new_generic_month_date(27, 28);
    _creator.new_timerange(8, 30, 12, 30, dr);
    _creator.new_timerange(18, 30, 21, 15, dr);
  }

 protected:
  timeperiod_creator _creator;
};

// Given a timeperiod configured with generic month dates
// And we are earlier than these dates
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next date's timerange
TEST_F(GetNextValidTimeGenericMonthDateTest, BeforeGenericMonthDates) {
  default_data_set();
  time_t now(strtotimet("2016-10-24 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-25 10:45:00"));
}

// Given a timeperiod configured with generic month dates
// And we are between two generic month dates
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next date's timerange
TEST_F(GetNextValidTimeGenericMonthDateTest, BetweenGenericMonthDates) {
  default_data_set();
  time_t now(strtotimet("2016-10-26 12:00:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-10-27 08:30:00"));
}

// Given a timeperiod configured with generic month dates
// And we are within a generic month date
// When get_next_valid_time() is called
// Then the next valid time is now
TEST_F(GetNextValidTimeGenericMonthDateTest, WithinGenericMonthDate) {
  default_data_set();
  time_t now(strtotimet("2016-10-28 20:59:00"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, now);
}

// Given a timeperiod configured with generic month dates
// And we are after these dates in the month
// When get_next_valid_time() is called
// Then the next valid time is the first generic month date in the next month
TEST_F(GetNextValidTimeGenericMonthDateTest, AfterGenericMonthDates) {
  default_data_set();
  time_t now(strtotimet("2016-10-30 13:37:42"));
  set_time(now);
  time_t computed((time_t)-1);
  get_next_valid_time(now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-25 10:45:00"));
}
