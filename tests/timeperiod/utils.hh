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
#  define TESTS_TIMEPERIOD_UTILS_HH

#  include <ctime>
#  include <string>
#  include "com/centreon/engine/objects/daterange.hh"
#  include "com/centreon/engine/objects/timeperiod.hh"

class         timeperiod_creator
{
 public:
              timeperiod_creator();
              ~timeperiod_creator();
  timeperiod* get_timeperiods();
  timeperiod* new_timeperiod();
  daterange*  new_calendar_date(
                int start_year,
                int start_month,
                int start_day,
                int end_year,
                int end_month,
                int end_day,
                timeperiod* target = NULL);
  daterange*  new_specific_month_date(
                int start_month,
                int start_day,
                int end_month,
                int end_day,
                timeperiod* target = NULL);
  daterange*  new_generic_month_date(
                int start_day,
                int end_day,
                timeperiod* target = NULL);
  daterange*  new_offset_weekday_of_specific_month(
                int start_month,
                int start_day,
                int start_offset,
                int end_month,
                int end_day,
                int end_offset,
                timeperiod* target = NULL);
  void        new_timerange(
                int start_hour,
                int start_minute,
                int end_hour,
                int end_minute,
                daterange* target);
  void        new_timerange(
                int start_hour,
                int start_minute,
                int end_hour,
                int end_minute,
                int day,
                timeperiod* target = NULL);

 private:
  timeperiod* _timeperiods;
};

int    hmtos(int h, int m);
void   set_time(time_t now);
time_t strtotimet(std::string const& str);

#endif // !TESTS_TIMEPERIOD_UTILS_HH
