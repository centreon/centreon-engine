/*
** Copyright 1999-2010      Ethan Galstad
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

#ifndef CCE_MACROS_HH
#  define CCE_MACROS_HH

#  include "com/centreon/engine/macros/clear_host.hh"
#  include "com/centreon/engine/macros/clear_service.hh"
#  include "com/centreon/engine/macros/defines.hh"
#  include "com/centreon/engine/macros/grab_host.hh"
#  include "com/centreon/engine/macros/grab_service.hh"
#  include "com/centreon/engine/macros/grab_value.hh"
#  include "com/centreon/engine/macros/misc.hh"
#  include "com/centreon/engine/macros/process.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Macro generation functions.
int grab_custom_macro_value_r(
      nagios_macros* mac,
      char* macro_name,
      char const* host_name,
      char const* service_description,
      char** output);
int grab_datetime_macro_r(
      nagios_macros* mac,
      int macro_type,
      char const* arg1,
      char const* arg2,
      char** output);
int grab_custom_object_macro_r(
      nagios_macros* mac,
      char* macro_name,
      customvariablesmember* vars,
      char** output);

// cleans macros characters before insertion into output string
char const* clean_macro_chars(char* macro,int options);

// URL encode a string
char* get_url_encoded_string(char* input);

int init_macros();
int init_macrox_names();
int free_macrox_names();

void copy_constant_macros(char** dest);

// Clear macros.
int clear_argv_macros_r(nagios_macros* mac);
int clear_volatile_macros_r(nagios_macros* mac);
int clear_summary_macros_r(nagios_macros* mac);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_MACROS_HH
