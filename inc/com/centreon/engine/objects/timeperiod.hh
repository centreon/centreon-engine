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

#ifndef CCE_OBJECTS_TIMEPERIOD_HH
#  define CCE_OBJECTS_TIMEPERIOD_HH

#  include "com/centreon/engine/common.hh"

/* Forward declaration. */
struct daterange_struct;
struct timeperiodexclusion_struct;
struct timerange_struct;

typedef struct                timeperiod_struct {
  char*                       name;
  char*                       alias;
  timerange_struct*           days[7];
  daterange_struct*           exceptions[DATERANGE_TYPES];
  timeperiodexclusion_struct* exclusions;
  struct timeperiod_struct*   next;
  struct timeperiod_struct*   nexthash;
}                             timeperiod;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

timeperiod* add_timeperiod(char const* name, char const* alias);

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                timeperiod const& obj1,
                timeperiod const& obj2) throw ();
bool          operator!=(
                timeperiod const& obj1,
                timeperiod const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, timeperiod const& obj);

CCE_BEGIN()

timeperiod&   find_timperiod(std::string const& name);
bool          is_timeperiod_exist(std::string const& name) throw ();

CCE_END()

#  endif /* C++ */

#endif // !CCE_OBJECTS_TIMEPERIOD_HH


