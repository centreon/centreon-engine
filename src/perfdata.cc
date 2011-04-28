/*
** Copyright 2000-2004 Ethan Galstad
** Copyright 2011      Merethis
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

/*********** COMMON HEADER FILES ***********/

#include "configuration.hh"
#include "globals.hh"
#include "xpddefault.hh"
#include "perfdata.hh"

using namespace com::centreon::engine;

/******************************************************************/
/************** INITIALIZATION & CLEANUP FUNCTIONS ****************/
/******************************************************************/

/* initializes performance data */
int initialize_performance_data(char* config_file) {
  xpddefault_initialize_performance_data(config_file);
  return (OK);
}

/* cleans up performance data */
int cleanup_performance_data(char* config_file) {
  xpddefault_cleanup_performance_data(config_file);
  return (OK);
}

/******************************************************************/
/****************** PERFORMANCE DATA FUNCTIONS ********************/
/******************************************************************/

/* updates service performance data */
int update_service_performance_data(service* svc) {
  /* should we be processing performance data for anything? */
  if (config.get_process_performance_data() == false)
    return (OK);

  /* should we process performance data for this service? */
  if (svc->process_performance_data == FALSE)
    return (OK);

  /* process the performance data! */
  xpddefault_update_service_performance_data(svc);
  return (OK);
}

/* updates host performance data */
int update_host_performance_data(host* hst) {
  /* should we be processing performance data for anything? */
  if (config.get_process_performance_data() == false)
    return (OK);

  /* should we process performance data for this host? */
  if (hst->process_performance_data == FALSE)
    return (OK);

  /* process the performance data! */
  xpddefault_update_host_performance_data(hst);
  return (OK);
}
