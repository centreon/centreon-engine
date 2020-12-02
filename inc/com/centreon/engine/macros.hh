/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2019 Centreon
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
#define CCE_MACROS_HH
#include <string>
#include "com/centreon/engine/macros/clear_host.hh"
#include "com/centreon/engine/macros/clear_hostgroup.hh"
#include "com/centreon/engine/macros/clear_service.hh"
#include "com/centreon/engine/macros/clear_servicegroup.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/macros/grab_value.hh"
#include "com/centreon/engine/macros/misc.hh"
#include "com/centreon/engine/macros/process.hh"

// cleans macros characters before insertion into output string
std::string clean_macro_chars(std::string const& macro, int options);

// URL encode a string
std::string url_encode(std::string const& value);

#ifdef __cplusplus
extern "C" {
#endif  // C++

int grab_contact_address_macro(unsigned int macro_num,
                               com::centreon::engine::contact* temp_contact,
                               std::string& output);
int grab_standard_contactgroup_macro(
    int macro_type,
    std::shared_ptr<com::centreon::engine::contactgroup> temp_contactgroup,
    std::string& output);

int grab_contact_macros_r(nagios_macros* mac,
                          com::centreon::engine::contact* cntct);

int grab_custom_macro_value_r(nagios_macros* mac,
                              std::string const& macro_name,
                              std::string const& arg1,
                              std::string const& arg2,
                              std::string& output);
int grab_datetime_macro_r(nagios_macros* mac,
                          int macro_type,
                          std::string const& arg1,
                          std::string const& arg2,
                          std::string& output);
int grab_standard_hostgroup_macro_r(
    nagios_macros* mac,
    int macro_type,
    com::centreon::engine::hostgroup* temp_hostgroup,
    std::string& output);
int grab_standard_servicegroup_macro_r(
    nagios_macros* mac,
    int macro_type,
    com::centreon::engine::servicegroup* temp_servicegroup,
    std::string& output);
int grab_standard_contact_macro_r(nagios_macros* mac,
                                  int macro_type,
                                  com::centreon::engine::contact* temp_contact,
                                  std::string& output);
int grab_custom_object_macro_r(nagios_macros* mac,
                               std::string const& macro_name,
                               com::centreon::engine::map_customvar const& vars,
                               std::string& output);

int init_macros();
int init_macrox_names();
int free_macrox_names();

/* thread-safe version of the above */
int clear_argv_macros_r(nagios_macros* mac);
int clear_volatile_macros_r(nagios_macros* mac);
int clear_contact_macros_r(nagios_macros* mac);
int clear_contactgroup_macros_r(nagios_macros* mac);
int clear_summary_macros_r(nagios_macros* mac);

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_MACROS_HH
