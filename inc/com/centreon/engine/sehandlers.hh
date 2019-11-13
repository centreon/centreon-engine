/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_SEHANDLERS_HH
#define CCE_SEHANDLERS_HH

#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/service.hh"

// Event Handler Types
#define HOST_EVENTHANDLER 0
#define SERVICE_EVENTHANDLER 1
#define GLOBAL_HOST_EVENTHANDLER 2
#define GLOBAL_SERVICE_EVENTHANDLER 3

#ifdef __cplusplus
extern "C" {
#endif  // C++

// Event Handler Functions

// distributed monitoring craziness...
int obsessive_compulsive_service_check_processor(
    com::centreon::engine::service* svc);
// distributed monitoring craziness...
int obsessive_compulsive_host_check_processor(com::centreon::engine::host* hst);
// top level service event logic
int handle_service_event(com::centreon::engine::service* svc);
// runs the global service event handler
int run_global_service_event_handler(nagios_macros* mac,
                                     com::centreon::engine::service* svc);
// runs the event handler for a specific service
int run_service_event_handler(nagios_macros* mac,
                              com::centreon::engine::service* svc);
// top level host event logic
int handle_host_event(com::centreon::engine::host* hst);
// runs the global host event handler
int run_global_host_event_handler(nagios_macros* mac,
                                  com::centreon::engine::host* hst);
// runs the event handler for a specific host
int run_host_event_handler(nagios_macros* mac,
                           com::centreon::engine::host* hst);
// top level host state handler
int handle_host_state(com::centreon::engine::host* hst);

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_SEHANDLERS_HH
