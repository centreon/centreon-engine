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

#ifndef CCE_OBJECTS_DATERANGE_HH
#  define CCE_OBJECTS_DATERANGE_HH

/* Forward declaration. */
struct timeperiod_struct;
struct timerange_struct;

typedef struct             daterange_struct {
  int                      type;
  int                      syear;        // Start year.
  int                      smon;         // Start month.
  int                      smday;        // Start day of month (may 3rd, last day in feb).
  int                      swday;        // Start day of week (thursday).
  int                      swday_offset; // Start weekday offset (3rd thursday, last monday in jan).
  int                      eyear;
  int                      emon;
  int                      emday;
  int                      ewday;
  int                      ewday_offset;
  int                      skip_interval;
  timerange_struct*        times;
  struct daterange_struct* next;
}                          daterange;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

daterange* add_exception_to_timeperiod(
             timeperiod_struct* period,
             int type,
             int syear,
             int smon,
             int smday,
             int swday,
             int swday_offset,
             int eyear,
             int emon,
             int emday,
             int ewday,
             int ewday_offset,
             int skip_interval);

#  ifdef __cplusplus
}

#    include <ostream>

bool               operator==(
                     daterange const& obj1,
                     daterange const& obj2) throw ();
bool               operator!=(
                     daterange const& obj1,
                     daterange const& obj2) throw ();
std::ostream&      operator<<(std::ostream& os, daterange const& obj);
std::string const& get_month_name(unsigned int index);
std::string const& get_weekday_name(unsigned int index);

#  endif /* C++ */

#endif // !CCE_OBJECTS_DATERANGE_HH


