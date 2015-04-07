/*
** Copyright 1999-2007 Ethan Galstad
** Copyright 2011-2015 Merethis
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

#ifndef CCE_LOGGING_HH
#  define CCE_LOGGING_HH

#  include <time.h>
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/service.hh"

// State Logging Types
#  define INITIAL_STATES             1
#  define CURRENT_STATES             2

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// logs a service event
int log_service_event(service const* svc);
// logs a host event
int log_host_event(host const* hst);
// logs initial/current host states
int log_host_states(unsigned int type, time_t* timestamp);
// logs initial/current service states
int log_service_states(unsigned int type, time_t* timestamp);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_LOGGING_HH
