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

int    hmtos(int h, int m);
void   set_time(time_t now);
time_t strtotimet(std::string const& str);

#endif // !TESTS_TIMEPERIOD_UTILS_HH
