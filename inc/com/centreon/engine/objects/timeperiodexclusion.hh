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

#ifndef CCE_OBJECTS_TIMEPERIODEXCLUSION_HH
#  define CCE_OBJECTS_TIMEPERIODEXCLUSION_HH

/* Forward declaration. */
struct timeperiod_struct;

typedef struct                       timeperiodexclusion_struct {
  char*                              timeperiod_name;
  struct timeperiod_struct*          timeperiod_ptr;
  struct timeperiodexclusion_struct* next;
}                                    timeperiodexclusion;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

timeperiodexclusion* add_exclusion_to_timeperiod(
                       timeperiod_struct* period,
                       char const* name);

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                timeperiodexclusion const& obj1,
                timeperiodexclusion const& obj2) throw ();
bool          operator!=(
                timeperiodexclusion const& obj1,
                timeperiodexclusion const& obj2) throw ();
std::ostream& operator<<(
                std::ostream& os,
                timeperiodexclusion const& obj);

#  endif /* C++ */

#endif // !CCE_OBJECTS_TIMEPERIODEXCLUSION_HH


