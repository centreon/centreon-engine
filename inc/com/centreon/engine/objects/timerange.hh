/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_OBJECTS_TIMERANGE_HH
#  define CCE_OBJECTS_TIMERANGE_HH

/* Forward declarations. */
struct daterange_struct;
struct timeperiod_struct;

typedef struct             timerange_struct {
  unsigned long            range_start;
  unsigned long            range_end;
  struct timerange_struct* next;
}                          timerange;

#  ifdef __cplusplus
extern "C" {
#  endif

timerange* add_timerange_to_daterange(
             daterange_struct* drange,
             unsigned long start_time,
             unsigned long end_time);
timerange* add_timerange_to_timeperiod(
             timeperiod_struct* period,
             int day,
             unsigned long start_time,
             unsigned long end_time);

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                timerange const& obj1,
                timerange const& obj2) throw ();
bool          operator!=(
                timerange const& obj1,
                timerange const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, timerange const& obj);

#  endif /* C++ */

#endif // !CCE_OBJECTS_TIMERANGE_HH


