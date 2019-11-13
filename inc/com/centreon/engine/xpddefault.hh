/*
** Copyright 2001-2006 Ethan Galstad
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

#ifndef CCE_XPDDEFAULT_HH
#define CCE_XPDDEFAULT_HH

#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/service.hh"

#ifdef __cplusplus
extern "C" {
#endif  // C++

int xpddefault_initialize_performance_data();
int xpddefault_cleanup_performance_data();

int xpddefault_update_service_performance_data(
    com::centreon::engine::service* svc);
int xpddefault_update_host_performance_data(com::centreon::engine::host* hst);

int xpddefault_run_service_performance_data_command(
    nagios_macros* mac,
    com::centreon::engine::service* svc);
int xpddefault_run_host_performance_data_command(
    nagios_macros* mac,
    com::centreon::engine::host* hst);

int xpddefault_update_service_performance_data_file(
    nagios_macros* mac,
    com::centreon::engine::service* svc);
int xpddefault_update_host_performance_data_file(
    nagios_macros* mac,
    com::centreon::engine::host* hst);

void xpddefault_preprocess_file_templates(char* tmpl);

int xpddefault_open_host_perfdata_file();
int xpddefault_open_service_perfdata_file();
int xpddefault_close_host_perfdata_file();
int xpddefault_close_service_perfdata_file();

int xpddefault_process_host_perfdata_file();
int xpddefault_process_service_perfdata_file();

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_XPDDEFAULT_HH
