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

#ifndef CCE_MACROS_HH
#  define CCE_MACROS_HH

#  include "com/centreon/engine/macros/clear_host.hh"
#  include "com/centreon/engine/macros/clear_hostgroup.hh"
#  include "com/centreon/engine/macros/clear_service.hh"
#  include "com/centreon/engine/macros/clear_servicegroup.hh"
#  include "com/centreon/engine/macros/defines.hh"
#  include "com/centreon/engine/macros/grab_host.hh"
#  include "com/centreon/engine/macros/grab_service.hh"
#  include "com/centreon/engine/macros/grab_value.hh"
#  include "com/centreon/engine/macros/misc.hh"
#  include "com/centreon/engine/macros/process.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

int grab_hostgroup_macros(hostgroup* hg);
int grab_servicegroup_macros(servicegroup* sg);
int grab_contact_macros(contact* cntct);

int grab_custom_macro_value(
      char* macro_name,
      char const* arg1,
      char const* arg2,
      char** output);
int grab_datetime_macro(
      int macro_type,
      char const* arg1,
      char const* arg2,
      char** output);
int grab_standard_hostgroup_macro(
      int macro_type,
      hostgroup* temp_hostgroup,
      char** output);
int grab_standard_service_macro(
      int macro_type,
      service* temp_service,
      char** output,
      int* free_macro);
int grab_standard_servicegroup_macro(
      int macro_type,
      servicegroup* temp_servicegroup,
      char** output);
int grab_standard_contact_macro(
      int macro_type,
      contact* temp_contact,
      char** output);
int grab_contact_address_macro(
      unsigned int macro_num,
      contact* temp_contact,
      char** output);
int grab_standard_contactgroup_macro(
      int macro_type,
      contactgroup* temp_contactgroup,
      char** output);
int grab_custom_object_macro(
      char* macro_name,
      customvariablesmember* vars,
      char** output);

// Thread-safe version of the above.
int grab_hostgroup_macros_r(
      nagios_macros* mac,
      hostgroup* hg);
int grab_servicegroup_macros_r(
      nagios_macros* mac,
      servicegroup* sg);
int grab_contact_macros_r(
      nagios_macros* mac,
      contact* cntct);

int grab_custom_macro_value_r(
      nagios_macros* mac,
      char* macro_name,
      char const* arg1,
      char const* arg2,
      char** output);
int grab_datetime_macro_r(
      nagios_macros* mac,
      int macro_type,
      char const* arg1,
      char const* arg2,
      char** output);
int grab_standard_hostgroup_macro_r(
      nagios_macros* mac,
      int macro_type,
      hostgroup* temp_hostgroup,
      char** output);
int grab_standard_servicegroup_macro_r(
      nagios_macros* mac,
      int macro_type,
      servicegroup* temp_servicegroup,
      char** output);
int grab_standard_contact_macro_r(
      nagios_macros* mac,
      int macro_type,
      contact* temp_contact,
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

/* clear macros */
int clear_argv_macros();
int clear_volatile_macros();
int clear_contact_macros();
int clear_contactgroup_macros();
int clear_summary_macros();

/* thread-safe version of the above */
int clear_argv_macros_r(nagios_macros* mac);
int clear_volatile_macros_r(nagios_macros* mac);
int clear_contact_macros_r(nagios_macros* mac);
int clear_contactgroup_macros_r(nagios_macros* mac);
int clear_summary_macros_r(nagios_macros* mac);

int set_all_macro_environment_vars(int set);
int set_macrox_environment_vars(int set);
int set_argv_macro_environment_vars(int set);
int set_custom_macro_environment_vars(int set);
int set_contact_address_environment_vars(int set);
int set_macro_environment_var(
      char const* name,
      char const* value,
      int set);

/* thread-safe version of the above */
int set_all_macro_environment_vars_r(nagios_macros* mac, int set);
int set_macrox_environment_vars_r(nagios_macros* mac, int set);
int set_argv_macro_environment_vars_r(nagios_macros* mac, int set);
int set_custom_macro_environment_vars_r(nagios_macros* mac, int set);
int set_contact_address_environment_vars_r(nagios_macros* mac, int set);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_MACROS_HH
