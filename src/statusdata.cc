/*
** Copyright 2000-2006 Ethan Galstad
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

#include "com/centreon/engine/statusdata.hh"

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/xsddefault.hh"

/******************************************************************/
/****************** TOP-LEVEL OUTPUT FUNCTIONS ********************/
/******************************************************************/

/* initializes status data at program start */
int initialize_status_data() {
  return xsddefault_initialize_status_data();
}

/* update all status data (aggregated dump) */
int update_all_status_data() {
  int result = OK;

  /* send data to event broker */
  broker_aggregated_status_data(NEBTYPE_AGGREGATEDSTATUS_STARTDUMP,
                                NEBFLAG_NONE, NEBATTR_NONE, NULL);

  result = xsddefault_save_status_data();

  /* send data to event broker */
  broker_aggregated_status_data(NEBTYPE_AGGREGATEDSTATUS_ENDDUMP, NEBFLAG_NONE,
                                NEBATTR_NONE, NULL);

  if (result != OK)
    return ERROR;
  return OK;
}

/* cleans up status data before program termination */
int cleanup_status_data(int delete_status_data) {
  return xsddefault_cleanup_status_data(delete_status_data);
}

/* updates program status info */
int update_program_status(int aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  static time_t next_program_status =
      time(nullptr) + instance_heartbeat_interval;
  time_t now = time(nullptr);
  if (now >= next_program_status) {
    next_program_status += instance_heartbeat_interval;
    if (!aggregated_dump)
      broker_program_status(NEBTYPE_PROGRAMSTATUS_UPDATE, NEBFLAG_NONE,
                            NEBATTR_NONE, NULL);
  }
  return OK;
}
