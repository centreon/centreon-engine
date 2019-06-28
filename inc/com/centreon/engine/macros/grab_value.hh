/*
** Copyright 1999-2010 Ethan Galstad
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

#ifndef CCE_MACROS_GRAB_VALUE_HH
#  define CCE_MACROS_GRAB_VALUE_HH

#  include "com/centreon/engine/macros/defines.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

int grab_macro_value_r(
      nagios_macros* mac,
      char const* macro_buffer,
      std::string& output,
      int* clean_options,
      int* free_macro);
int grab_macrox_value_r(
      nagios_macros* mac,
      int macro_type,
      char const* arg1,
      char const* arg2,
      std::string& output,
      int* free_macro);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_MACROS_GRAB_VALUE_HH
