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

class GetNextValidTimeExclusionTest : public ::testing::Test {
 public:
  void SetUp() override {
    configuration::applier::state::load();
    _computed = (time_t)-1;
    _tp = _creator.new_timeperiod();
    for (int i(0); i < 7; ++i)
      _creator.new_timerange(0, 0, 24, 0, i);
    _now = strtotimet("2016-11-24 08:00:00");
    set_time(_now);
  }

  void TearDown() override {
    configuration::applier::state::unload();
  }

  void calendar_date_full_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(_creator.new_calendar_date(2016, 10, 24, 2016, 10, 24));

    _creator.new_timerange(0, 0, 24, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void calendar_date_partial_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(_creator.new_calendar_date(2016, 10, 24, 2016, 10, 30));
    _creator.new_timerange(8, 0, 9, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void specific_month_date_full_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(_creator.new_specific_month_date(10, 24, 10, 25));
    _creator.new_timerange(0, 0, 24, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void specific_month_date_partial_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(_creator.new_specific_month_date(10, 24, 10, 28));
    _creator.new_timerange(8, 0, 10, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void generic_month_date_full_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(_creator.new_generic_month_date(24, 26));
    _creator.new_timerange(0, 0, 24, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void generic_month_date_partial_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(_creator.new_generic_month_date(24, 29));
    _creator.new_timerange(8, 0, 11, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void offset_weekday_of_specific_month_full_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(
        _creator.new_offset_weekday_of_specific_month(10, 4, -1, 10, 0, -1));
    _creator.new_timerange(0, 0, 24, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void offset_weekday_of_specific_month_partial_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(
        _creator.new_offset_weekday_of_specific_month(10, 4, -1, 10, 0, -1));
    _creator.new_timerange(8, 0, 12, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void offset_weekday_of_generic_month_full_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(_creator.new_offset_weekday_of_generic_month(4, -1, 1, -1));
    _creator.new_timerange(0, 0, 24, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void offset_weekday_of_generic_month_partial_days_exclusion() {
    _creator.new_timeperiod();
    daterange* dr(_creator.new_offset_weekday_of_generic_month(4, -1, 1, -1));
    _creator.new_timerange(8, 0, 13, 0, dr);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
    std::cout << _creator.get_timeperiods_shared();
  }

  void normal_weekday_full_days_exclusion() {
    _creator.new_timeperiod();
    _creator.new_timerange(0, 0, 24, 0, 0);
    _creator.new_timerange(0, 0, 24, 0, 1);
    _creator.new_timerange(0, 0, 24, 0, 2);
    _creator.new_timerange(0, 0, 24, 0, 4);
    _creator.new_timerange(0, 0, 24, 0, 5);
    _creator.new_timerange(0, 0, 24, 0, 6);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

  void normal_weekday_partial_days_exclusion() {
    _creator.new_timeperiod();
    _creator.new_timerange(8, 0, 14, 0, 0);
    _creator.new_timerange(8, 0, 14, 0, 1);
    _creator.new_timerange(8, 0, 14, 0, 2);
    _creator.new_timerange(8, 0, 14, 0, 4);
    _creator.new_timerange(8, 0, 14, 0, 5);
    _creator.new_timerange(8, 0, 14, 0, 6);
    _creator.new_exclusion(_creator.get_timeperiods_shared(), _tp);
  }

 protected:
  time_t _computed;
  timeperiod_creator _creator;
  time_t _now;
  timeperiod* _tp;
};

// Given a timeperiod with exclusions
// And we are within an exclusion
// When get_next_valid_time() is called
// Then the next valid time is outside the exclusions

//
// FULL DAYS (DATERANGE EXCLUSION)
//

TEST_F(GetNextValidTimeExclusionTest, FullDaysCalendarDate) {
  calendar_date_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-25 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, FullDaysCalendarDateAndBelow) {
  calendar_date_full_days_exclusion();
  specific_month_date_full_days_exclusion();
  generic_month_date_full_days_exclusion();
  offset_weekday_of_specific_month_full_days_exclusion();
  offset_weekday_of_generic_month_full_days_exclusion();
  normal_weekday_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-30 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, FullDaysSpecificMonthDate) {
  specific_month_date_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-26 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, FullDaysSpecificMonthDateAndBelow) {
  specific_month_date_full_days_exclusion();
  generic_month_date_full_days_exclusion();
  offset_weekday_of_specific_month_full_days_exclusion();
  offset_weekday_of_generic_month_full_days_exclusion();
  normal_weekday_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-30 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, FullDaysGenericMonthDate) {
  generic_month_date_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-27 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, FullDaysGenericMonthDateAndBelow) {
  generic_month_date_full_days_exclusion();
  offset_weekday_of_specific_month_full_days_exclusion();
  offset_weekday_of_generic_month_full_days_exclusion();
  normal_weekday_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-30 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, FullDaysOffsetWeekdayOfSpecificMonth) {
  offset_weekday_of_specific_month_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-28 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest,
       FullDaysOffsetWeekdayOfSpecificMonthAndBelow) {
  offset_weekday_of_specific_month_full_days_exclusion();
  offset_weekday_of_generic_month_full_days_exclusion();
  normal_weekday_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-30 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, FullDaysOffsetWeekdayOfGenericMonth) {
  offset_weekday_of_generic_month_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-29 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest,
       FullDaysOffsetWeekdayOfGenericMonthAndBelow) {
  offset_weekday_of_generic_month_full_days_exclusion();
  normal_weekday_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-30 00:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, FullDaysNormalWeekday) {
  normal_weekday_full_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-30 00:00:00"));
}

//
// PARTIAL DAYS (TIMERANGE EXCLUSIONS)
//

TEST_F(GetNextValidTimeExclusionTest, PartialDaysCalendarDate) {
  calendar_date_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 09:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, PartialDaysCalendarDateAndBelow) {
  calendar_date_partial_days_exclusion();
  specific_month_date_partial_days_exclusion();
  generic_month_date_partial_days_exclusion();
  offset_weekday_of_specific_month_partial_days_exclusion();
  offset_weekday_of_generic_month_partial_days_exclusion();
  normal_weekday_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 14:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, PartialDaysSpecificMonthDate) {
  specific_month_date_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 10:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, PartialDaysSpecificMonthDateAndBelow) {
  specific_month_date_partial_days_exclusion();
  generic_month_date_partial_days_exclusion();
  offset_weekday_of_specific_month_partial_days_exclusion();
  offset_weekday_of_generic_month_partial_days_exclusion();
  normal_weekday_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 14:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, PartialDaysGenericMonthDate) {
  generic_month_date_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 11:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, PartialDaysGenericMonthDateAndBelow) {
  generic_month_date_partial_days_exclusion();
  offset_weekday_of_specific_month_partial_days_exclusion();
  offset_weekday_of_generic_month_partial_days_exclusion();
  normal_weekday_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 14:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, PartialDaysOffsetWeekdayOfSpecificMonth) {
  offset_weekday_of_specific_month_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 12:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest,
       PartialDaysOffsetWeekdayOfSpecificMonthAndBelow) {
  offset_weekday_of_specific_month_partial_days_exclusion();
  offset_weekday_of_generic_month_partial_days_exclusion();
  normal_weekday_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 14:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, PartialDaysOffsetWeekdayOfGenericMonth) {
  offset_weekday_of_generic_month_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 13:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest,
       PartialDaysOffsetWeekdayOfGenericMonthAndBelow) {
  offset_weekday_of_generic_month_partial_days_exclusion();
  normal_weekday_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 14:00:00"));
}

TEST_F(GetNextValidTimeExclusionTest, PartialDaysNormalWeekday) {
  normal_weekday_partial_days_exclusion();
  get_next_valid_time(_now, &_computed, _tp);
  ASSERT_EQ(_computed, strtotimet("2016-11-24 14:00:00"));
}
