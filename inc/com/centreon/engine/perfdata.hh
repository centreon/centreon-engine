/*
** Copyright 2000-2004 Ethan Galstad
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

#ifndef CCE_PERFDATA_HH
#  define CCE_PERFDATA_HH

#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/service.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// initializes performance data
int initialize_performance_data();
// cleans up performance data
int cleanup_performance_data();
// updates service performance data
int update_service_performance_data(service* svc);
// updates host performance data
int update_host_performance_data(host* hst);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_PERFDATA_HH
