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
#include "com/centreon/clib.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "com/centreon/logging/engine.hh"
#include "tests/timeperiod/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class GetNextValidTimeSkipIntervalTest : public ::testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    _computed = (time_t)-1;
    _creator.new_timeperiod();
    _now = strtotimet("2016-11-24 6:00:00");
    set_time(_now);
  }

  void TearDown() override {
    com::centreon::logging::engine::unload();
    clib::unload();
  }

  void calendar_date_skip() {
    daterange* dr(_creator.new_calendar_date(2016, 10, 19, 2017, 10, 1));
    _creator.new_timerange(8, 0, 17, 0, dr);
    dr->set_skip_interval(5);
  }

  void specific_month_date_skip() {
    daterange* dr(_creator.new_specific_month_date(10, 19, 11, 24));
    _creator.new_timerange(8, 0, 17, 0, dr);
    dr->set_skip_interval(5);
  }

  void generic_month_date_skip() {
    daterange* dr(_creator.new_generic_month_date(19, 30));
    _creator.new_timerange(8, 0, 17, 0, dr);
    dr->set_skip_interval(5);
  }

  void offset_weekday_of_specific_month_skip() {
    daterange* dr(
        _creator.new_offset_weekday_of_specific_month(10, 6, -2, 10, 6, -1));
    _creator.new_timerange(8, 0, 17, 0, dr);
    dr->set_skip_interval(5);
  }

  void offset_weekday_of_generic_month_skip() {
    daterange* dr(_creator.new_offset_weekday_of_generic_month(6, -2, 6, -1));
    _creator.new_timerange(8, 0, 17, 0, dr);
    dr->set_skip_interval(5);
  }

 protected:
  time_t _computed;
  timeperiod_creator _creator;
  time_t _now;
};

// Given a timeperiod with exceptions with skip interval
// When get_next_valid_time() is called
// Then the next valid time is calculated by skipping days in each exception

TEST_F(GetNextValidTimeSkipIntervalTest, CalendarDate) {
  calendar_date_skip();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-11-24 08:00:00"));
}

TEST_F(GetNextValidTimeSkipIntervalTest, SpecificMonthDate) {
  specific_month_date_skip();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-11-24 08:00:00"));
}

TEST_F(GetNextValidTimeSkipIntervalTest, GenericMonthDate) {
  generic_month_date_skip();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-11-24 08:00:00"));
}

TEST_F(GetNextValidTimeSkipIntervalTest, OffsetWeekdayOfSpecificMonth) {
  offset_weekday_of_generic_month_skip();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-11-24 08:00:00"));
}

TEST_F(GetNextValidTimeSkipIntervalTest, OffsetWeekdayOfGenericMonth) {
  offset_weekday_of_generic_month_skip();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-11-24 08:00:00"));
}
