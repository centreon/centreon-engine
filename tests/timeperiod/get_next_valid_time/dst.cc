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
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "tests/timeperiod/utils.hh"

using namespace com::centreon::engine;

class        GetNextValidTimeDST : public ::testing::Test {
 public:
  void       SetUp() {
    _creator.new_timeperiod();
    _computed = (time_t)-1;
  }

  daterange* dst_calendar_date() {
    return (_creator.new_calendar_date(2016, 9, 30, 2016, 9, 30));
  }

  void       timerange_includes_dst(daterange* dr) {
    _creator.new_timerange(1, 0, 4, 0, dr);
  }

  void       timerange_excludes_dst(daterange* dr) {
    _creator.new_timerange(1, 0, 2, 0, dr);
    _creator.new_timerange(3, 0, 4, 0, dr);
  }

  void       timerange_within_dst(daterange* dr) {
    _creator.new_timerange(2, 15, 2, 45, dr);
  }

  void       beginning_of_overlapped_hour() {
    _now = strtotimet("2016-10-30 01:59:59") + 1;
    set_time(_now);
  }

 protected:
  time_t             _computed;
  timeperiod_creator _creator;
  time_t             _now;
};

// Given a timeperiod configured with calendar dates
// And the dates cover a backward DST change
// And the timeranges include the DST change
// And we are at the beginning of the hour that will be overlapped in the original timezone
// When get_next_valid_time is called
// Then the next valid time is now
TEST_F(GetNextValidTimeDST, BackwardDSTAtBeginningOfOverlappedHourCalendarDateIncludeDST) {
  daterange* dr(dst_calendar_date());
  timerange_includes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1);
}

// Given a timeperiod configured with calendar dates
// And the dates cover a backward DST change
// And the timeranges exclude the DST change
// And we are at the beginning of the hour that will be overlapped in the original timezone
// When get_next_valid_time is called
// Then the next valid time is just after the DST
TEST_F(GetNextValidTimeDST, BackwardDSTAtBeginningOfOverlappedHourCalendarDateExcludeDST) {
  daterange* dr(dst_calendar_date());
  timerange_excludes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

// Given a timeperiod configured with calendar dates
// And the dates cover a backward DST change
// And the timeranges is within the DST change
// And we are at the beginning of the hour that will be overlapped in the original timezone
// When get_next_valid_time is called
// Then the next valid time is the beginning of the timerange in the original timezone
TEST_F(GetNextValidTimeDST, BackwardDSTAtBeginningOfOverlappedHourCalendarDateWithinDST) {
  daterange* dr(dst_calendar_date());
  timerange_within_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1 + 15 * 60);
}
