/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2011-2014 Merethis
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

#ifndef CCE_TIMEPERIOD_HH
#  define CCE_TIMEPERIOD_HH

#  ifdef __cplusplus
extern "C" {
#  endif // C++

int  check_time_against_period(
       time_t test_time,
       timeperiod* tperiod,
       char const* tz);
void get_next_valid_time(
       time_t pref_time,
       time_t* valid_time,
       timeperiod* tperiod,
       char const* tz);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_TIMEPERIOD_HH
