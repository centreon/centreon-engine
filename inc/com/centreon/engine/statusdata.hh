/*
** Copyright 2000-2007 Ethan Galstad
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

#ifndef CCE_STATUSDATA_HH
#define CCE_STATUSDATA_HH

#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/service.hh"

#ifdef __cplusplus
extern "C" {
#endif  // C++

// initializes status data at program start
int initialize_status_data();
// updates all status data
int update_all_status_data();
// cleans up status data at program termination
int cleanup_status_data(int delete_status_data);
// updates program status data
int update_program_status(int aggregated_dump);

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_STATUSDATA_HH
