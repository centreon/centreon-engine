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

#include <cstring>
#include <gtest/gtest.h>
#include "com/centreon/engine/deleter/timeperiod.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/objects/timerange.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "tests/timeperiod/utils.hh"

using namespace com::centreon::engine;

class         GetNextValidTimeNormalWeekdayTest : public ::testing::Test {
 public:
  void        SetUp() {
    _timeperiod = new timeperiod();
    // tuesday 10:30-11:45
    add_timerange_to_timeperiod(
      _timeperiod,
      2,
      hmtos(10, 30),
      hmtos(11, 45));
    // wednesday 12:00-13:00,18:30-23:30
    add_timerange_to_timeperiod(
      _timeperiod,
      3,
      hmtos(12, 0),
      hmtos(13, 0));
    add_timerange_to_timeperiod(
      _timeperiod,
      3,
      hmtos(18, 30),
      hmtos(23, 30));
    // friday 08:30-09:15,12:15-12:45,18:30-19:45
    add_timerange_to_timeperiod(
      _timeperiod,
      5,
      hmtos(8, 30),
      hmtos(9, 15));
    add_timerange_to_timeperiod(
      _timeperiod,
      5,
      hmtos(12, 15),
      hmtos(12, 45));
    add_timerange_to_timeperiod(
      _timeperiod,
      5,
      hmtos(18, 30),
      hmtos(19, 45));
  }

  void        TearDown() {
    deleter::timeperiod(_timeperiod);
    _timeperiod = NULL;
    return ;
  }

 protected:
  timeperiod* _timeperiod;
};

// Given a timeperiod configured with normal weekdays
// And we are earlier in the week than this weekday
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the weekday's timerange
TEST_F(GetNextValidTimeNormalWeekdayTest, BeforeWeekday) {
  time_t now(strtotimet("2016-10-24 12:00:00"));
  set_time(now);
  time_t expected((time_t)-1);
  get_next_valid_time(now, &expected, _timeperiod);
  ASSERT_EQ(expected, strtotimet("2016-10-25 10:30:00"));
}

// Given a timeperiod configured with normal weekdays
// And we are between two weekdays
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the next weekday's timerange
TEST_F(GetNextValidTimeNormalWeekdayTest, BetweenWeekdays) {
  time_t now(strtotimet("2016-10-27 12:00:00"));
  set_time(now);
  time_t expected((time_t)-1);
  get_next_valid_time(now, &expected, _timeperiod);
  ASSERT_EQ(expected, strtotimet("2016-10-28 08:30:00"));
}

// Given a timeperiod configured with normal weekdays
// And we are within a weekdday
// When get_next_valid_time() is called
// Then the next valid time is now
TEST_F(GetNextValidTimeNormalWeekdayTest, WithinWeekday) {
  time_t now(strtotimet("2016-10-28 18:36:00"));
  set_time(now);
  time_t expected((time_t)-1);
  get_next_valid_time(now, &expected, _timeperiod);
  ASSERT_EQ(expected, now);
}
