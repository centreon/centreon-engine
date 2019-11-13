/*
** Copyright 1999-2007      Ethan Galstad
** Copyright 2011-2013,2017 Centreon
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
#define CCE_LOGGING_HH

#include <time.h>
#include "com/centreon/engine/namespace.hh"

// State Logging Types
#define INITIAL_STATES 1

#define NSLOG_HOST_UP 1024
#define NSLOG_HOST_DOWN 2048
#define NSLOG_HOST_UNREACHABLE 4096

#define NSLOG_SERVICE_OK 8192
#define NSLOG_SERVICE_UNKNOWN 16384
#define NSLOG_SERVICE_WARNING 32768
#define NSLOG_SERVICE_CRITICAL 65536

CCE_BEGIN()
class host;
class service;
CCE_END()

#ifdef __cplusplus
extern "C" {
#endif  // C++

void log_host_state(unsigned int type, com::centreon::engine::host* hst);
// logs initial/current service states
void log_service_state(unsigned int type, com::centreon::engine::service* svc);

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_LOGGING_HH
