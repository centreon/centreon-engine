/*
** Copyright 2013 Merethis
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

#include <time.h>
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/utils.hh"
#include "test/core/libtesttime.hh"

#ifndef __THROW
#  define __THROW
#endif // !__THROW

// overload of libc time function.
// always return "Mon Jan 28 13:00:00 CET 2013"
extern "C" time_t time(time_t *t) __THROW {
  if (t)
    *t = 1359370800;
  return (1359370800);
}

using namespace com::centreon::engine;

/**
 *  Check valid time.
 *
 *  @param[in] p     Time period to check.
 *  @param[in] pref  Preferred time.
 *  @param[in] ref   Reference time.
 *
 *  @return True on success, false otherwise.
 */
bool core::check_valid_time(timeperiod* p, time_t pref, time_t ref) {
  time_t valid(0);
  get_next_valid_time(pref, &valid, p);
  return (valid == ref);
}

/**
 *  Build week days with specific range.
 *
 *  @param[out] range      Tab to fill.
 *  @param[in]  range_str  Define range.
 */
void core::build_week_days(
       std::vector<std::string>& range,
       std::string const& range_str) {
  static char const* tab_days[] = {
    "sunday ",
    "monday ",
    "tuesday ",
    "wednesday ",
    "thursday ",
    "friday ",
    "saturday ",
    NULL
  };

  for (unsigned int i(0); tab_days[i]; ++i)
    range.push_back(tab_days[i] + range_str);
}

/**
 *  Build timeperiod and fill it with specific range and exlusion.
 *
 *  @param[in] name     The timeperiod name.
 *  @param[in] range    Week days and exception.
 *  @param[in] exclude  Timeperiod exclusion.
 *
 *  @return New timeperiod.
 */
timeperiod* core::build_timeperiod(
              std::string const& name,
              std::vector<std::string> const& range,
              std::vector<std::string> const& exclude) {
  objects::add_timeperiod(name, name, range, exclude);
  timeperiod* p(find_timeperiod(name.c_str()));
  if (p)
    for (timeperiodexclusion* e(p->exclusions); e; e = e->next)
      e->timeperiod_ptr = find_timeperiod(e->timeperiod_name);
  return (p);
}
