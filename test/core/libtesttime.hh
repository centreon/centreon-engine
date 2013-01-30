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

#ifndef TEST_CORE_LIBTESTTIME_HH
#  define TEST_CORE_LIBTESTTIME_HH

#  include <string>
#  include <vector>
#  include "com/centreon/engine/objects.hh"

#  define HOUR(x) (x) * 60 * 60
#  define MIN(x) (x) * 60
#  define SEC(x) (x)

CCE_BEGIN()

namespace     core {
  timeperiod* build_timeperiod(
                std::string const& name,
                std::vector<std::string> const& range,
                std::vector<std::string> const& exclude);
  void        build_week_days(
                std::vector<std::string>& range,
                std::string const& range_str);
  bool        check_valid_time(
                timeperiod* p,
                time_t pref,
                time_t ref);
}

CCE_END()

#endif // !TEST_CORE_LIBTESTTIME_HH
