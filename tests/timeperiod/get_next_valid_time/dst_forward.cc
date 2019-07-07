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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "tests/timeperiod/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class        GetNextValidTimeForwardDST : public ::testing::Test {
 public:
  void       SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    configuration::applier::state::load();
    _creator.new_timeperiod();
    _computed = (time_t)-1;
  }

  void TearDown() override {
    configuration::applier::state::unload();
    com::centreon::logging::engine::unload();
    clib::unload();
  }

  //
  // DATE RANGES
  //

  daterange* dst_calendar_date() {
    return (_creator.new_calendar_date(2017, 2, 26, 2017, 2, 26));
  }

  daterange* dst_specific_month_date() {
    return (_creator.new_specific_month_date(2, 26, 2, 26));
  }

  daterange* dst_generic_month_date() {
    return (_creator.new_generic_month_date(26, 26));
  }

  daterange* dst_offset_weekday_of_specific_month() {
    return (_creator.new_offset_weekday_of_specific_month(
                       2,
                       0,
                       4,
                       2,
                       0,
                       4));
  }

  daterange* dst_offset_weekday_of_generic_month() {
    return (_creator.new_offset_weekday_of_generic_month(0, 4, 0, 4));
  }

  //
  // TIME RANGES
  //

  void       timerange_includes_dst(daterange* dr) {
    _creator.new_timerange(1, 0, 4, 0, dr);
  }

  void       timerange_excludes_dst(daterange* dr) {
    _creator.new_timerange(1, 0, 2, 0, dr);
    _creator.new_timerange(3, 0, 4, 0, dr);
  }

  //
  // CURRENT TIME
  //

  void       at_dst() {
    _now = strtotimet("2017-03-26 01:59:59") + 1;
    set_time(_now);
  }

 protected:
  time_t             _computed;
  timeperiod_creator _creator;
  time_t             _now;
};

//
// RESULTS WILL BE MOSTLY THE SAME FOR ALL DATE RANGES
//
// So instead of writing all Given/When/Then, here are the common cases.
// Please note that no test with timeranges starting in, stopping in or
// within the DST forward gap exist. This is because no assumption can
// be made on such ranges.
//
// Given a timeperiod covering a forward DST change
// And the timeranges includes the DST range
// And we are at DST
// When get_next_valid_time() is called
// Then the next valid time is now
//
// Given a timeperiod covering a forward DST change
// And the timeranges excludes the DST range
// And we are at DST
// When get_next_valid_time() is callled
// Then the next valid time is the beginning of the timerange in the new timezone


//
// CALENDAR DATE TESTS
//

TEST_F(GetNextValidTimeForwardDST, CalendarDateIncludeDSTNowAtDST) {
  daterange* dr(dst_calendar_date());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

TEST_F(GetNextValidTimeForwardDST, CalendarDateExcludeDSTNowAtDST) {
  daterange* dr(dst_calendar_date());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

//
// SPECIFIC MONTH DATE TESTS
//

TEST_F(GetNextValidTimeForwardDST, SpecificMonthDateIncludeDSTNowAtDST) {
  daterange* dr(dst_specific_month_date());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

TEST_F(GetNextValidTimeForwardDST, SpecificMonthDateExcludeDSTNowAtDST) {
  daterange* dr(dst_specific_month_date());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

//
// GENERIC MONTH DATE TESTS
//

TEST_F(GetNextValidTimeForwardDST, GenericMonthDateIncludeDSTNowAtDST) {
  daterange* dr(dst_generic_month_date());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

TEST_F(GetNextValidTimeForwardDST, GenericMonthDateExcludeDSTNowAtDST) {
  daterange* dr(dst_generic_month_date());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

//
// OFFSET WEEKDAY OF SPECIFIC MONTH DATE TESTS
//

TEST_F(GetNextValidTimeForwardDST, OffsetWeekdayOfSpecificMonthIncludeDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

TEST_F(GetNextValidTimeForwardDST, OffsetWeekdayOfSpecificMonthExcludeDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

//
// OFFSET WEEKDAY OF GENERIC MONTH DATE TESTS
//

TEST_F(GetNextValidTimeForwardDST, OffsetWeekdayOfGenericMonthIncludeDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

TEST_F(GetNextValidTimeForwardDST, OffsetWeekdayOfGenericMonthExcludeDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

//
// NORMAL WEEKDAY TESTS
//

TEST_F(GetNextValidTimeForwardDST, NormalWeekdayIncludeDSTNowAtDST) {
  _creator.new_timerange(1, 0, 4, 0, 0);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}

TEST_F(GetNextValidTimeForwardDST, NormalWeekdayExcludeDSTNowAtDST) {
  _creator.new_timerange(1, 0, 2, 0, 0);
  _creator.new_timerange(3, 0, 4, 0, 0);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-03-26 03:00:00"));
}
