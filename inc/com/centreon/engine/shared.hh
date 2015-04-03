/*
** Copyright 1999-2011      Ethan Galstad
** Copyright 2011-2013,2015 Merethis
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

#ifndef CCE_SHARED_HH
#  define CCE_SHARED_HH

#  include <sys/time.h>
#  include <sys/types.h>

// only usable on compile-time initialized arrays, for obvious reasons
#  define ARRAY_SIZE(ary) (sizeof(ary) / sizeof(ary[0]))

#  ifdef __cplusplus
extern "C" {
#  endif // C++

char* my_strtok(char const* buffer, char const* tokens);
char* my_strsep(char** stringp, char const* delim);
void strip(char* buffer);
void get_time_breakdown(
       unsigned long raw_time,
       int* days,
       int* hours,
       int* minutes,
       int* seconds);
char* resize_string(char* str, size_t size);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_SHARED_HH
