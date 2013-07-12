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

#ifndef CCE_CONFIG_HH
#  define CCE_CONFIG_HH

#  include "com/centreon/engine/objects.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Try and verify the configuration data.
int pre_flight_check();
// Verify object relationships and settings.
int pre_flight_object_check(int* w, int* e);
// Detects circular dependencies and paths.
int pre_flight_circular_check(int* w, int* e);

int check_service(service* svc, int* w, int* e);
int check_host(host* hst, int* w, int* e);
int check_contact(contact* cntct, int* w, int* e);
int check_servicegroup(servicegroup* sg, int* w, int* e);
int check_hostgroup(hostgroup* hg, int* w, int* e);
int check_contactgroup(contactgroup* cg, int* w, int* e);
int check_servicedependency(servicedependency* sd, int* w, int* e);
int check_hostdependency(hostdependency* hd, int* w, int* e);
int check_serviceescalation(serviceescalation* se, int* w, int* e);
int check_hostescalation(hostescalation* he, int* w, int* e);
int check_timeperiod(timeperiod* tp, int* w, int* e);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_CONFIG_HH
