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
#  define CCE_FLAPPING_HH

#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/service.hh"

// Flapping Types
#  define HOST_FLAPPING    0
#  define SERVICE_FLAPPING 1

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Flap Detection Functions

// determines whether or not a service is "flapping" between states
void check_for_service_flapping(
       service* svc,
       int update,
       int allow_flapstart_notification);
// determines whether or not a host is "flapping" between states
void check_for_host_flapping(
       host* hst,
       int update,
       int actual_check,
       int allow_flapstart_notification);
// handles a service that is flapping
void set_service_flap(
       service* svc,
       double percent_change,
       double high_threshold,
       double low_threshold,
       int allow_flapstart_notification);
// handles a service that has stopped flapping
void clear_service_flap(
       service* svc,
       double percent_change,
       double high_threshold,
       double low_threshold);
// handles a host that is flapping
void set_host_flap(
       host* hst,
       double percent_change,
       double high_threshold,
       double low_threshold,
       int allow_flapstart_notification);
// handles a host that has stopped flapping
void clear_host_flap(
       host* hst,
       double percent_change,
       double high_threshold,
       double low_threshold);
// enables flap detection on a program-wide basis
void enable_flap_detection_routines();
// disables flap detection on a program-wide basis
void disable_flap_detection_routines();
// enables flap detection for a particular host
void enable_host_flap_detection(host* hst);
// disables flap detection for a particular host
void disable_host_flap_detection(host* hst);
// handles the details when flap detection is disabled globally or on a per-host basis
void handle_host_flap_detection_disabled(host* hst);
// enables flap detection for a particular service
void enable_service_flap_detection(service* svc);
// disables flap detection for a particular service
void disable_service_flap_detection(service* svc);
// handles the details when flap detection is disabled globally or on a per-service basis
void handle_service_flap_detection_disabled(service* svc);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_FLAPPING_HH
