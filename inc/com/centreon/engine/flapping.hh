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

#ifndef CCE_FLAPPING_HH
#define CCE_FLAPPING_HH

#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/service.hh"

// Flapping Types
#define HOST_FLAPPING 0
#define SERVICE_FLAPPING 1

#ifdef __cplusplus
extern "C" {
#endif  // C++

// handles a service that is flapping
// handles a host that is flapping
// enables flap detection on a program-wide basis
void enable_flap_detection_routines();
// disables flap detection on a program-wide basis
void disable_flap_detection_routines();
// enables flap detection for a particular host
void enable_host_flap_detection(com::centreon::engine::host* hst);
// disables flap detection for a particular host
void disable_host_flap_detection(com::centreon::engine::host* hst);
// enables flap detection for a particular service
void enable_service_flap_detection(com::centreon::engine::service* svc);
// disables flap detection for a particular service
void disable_service_flap_detection(com::centreon::engine::service* svc);

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_FLAPPING_HH
