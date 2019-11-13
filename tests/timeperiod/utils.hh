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

#ifndef TESTS_TIMEPERIOD_UTILS_HH
#define TESTS_TIMEPERIOD_UTILS_HH

#include <ctime>
#include <string>
#include "com/centreon/engine/daterange.hh"
#include "com/centreon/engine/timeperiod.hh"

class timeperiod_creator {
 public:
  timeperiod_creator();
  ~timeperiod_creator();
  com::centreon::engine::timeperiod* get_timeperiods();
  std::shared_ptr<com::centreon::engine::timeperiod> get_timeperiods_shared();
  com::centreon::engine::timeperiod* new_timeperiod();
  void new_exclusion(
      std::shared_ptr<com::centreon::engine::timeperiod> excluded,
      com::centreon::engine::timeperiod* target = NULL);
  com::centreon::engine::daterange* new_calendar_date(
      int start_year,
      int start_month,
      int start_day,
      int end_year,
      int end_month,
      int end_day,
      com::centreon::engine::timeperiod* target = NULL);
  com::centreon::engine::daterange* new_specific_month_date(
      int start_month,
      int start_day,
      int end_month,
      int end_day,
      com::centreon::engine::timeperiod* target = NULL);
  com::centreon::engine::daterange* new_generic_month_date(
      int start_day,
      int end_day,
      com::centreon::engine::timeperiod* target = NULL);
  com::centreon::engine::daterange* new_offset_weekday_of_specific_month(
      int start_month,
      int start_wday,
      int start_offset,
      int end_month,
      int end_wday,
      int end_offset,
      com::centreon::engine::timeperiod* target = NULL);
  com::centreon::engine::daterange* new_offset_weekday_of_generic_month(
      int start_wday,
      int start_offset,
      int end_wday,
      int end_offset,
      com::centreon::engine::timeperiod* target = NULL);
  void new_timerange(int start_hour,
                     int start_minute,
                     int end_hour,
                     int end_minute,
                     com::centreon::engine::daterange* target);
  void new_timerange(int start_hour,
                     int start_minute,
                     int end_hour,
                     int end_minute,
                     int day,
                     com::centreon::engine::timeperiod* target = NULL);

 private:
  std::list<std::shared_ptr<com::centreon::engine::timeperiod>> _timeperiods;
};

int hmtos(int h, int m);
void set_time(time_t now);
time_t strtotimet(std::string const& str);

#endif  // !TESTS_TIMEPERIOD_UTILS_HH
