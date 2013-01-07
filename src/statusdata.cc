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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/xsddefault.hh"

/******************************************************************/
/****************** TOP-LEVEL OUTPUT FUNCTIONS ********************/
/******************************************************************/

/* initializes status data at program start */
int initialize_status_data(char* config_file) {
  return (xsddefault_initialize_status_data(config_file));
}

/* update all status data (aggregated dump) */
int update_all_status_data() {
  int result = OK;

  /* send data to event broker */
  broker_aggregated_status_data(
    NEBTYPE_AGGREGATEDSTATUS_STARTDUMP,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);

  result = xsddefault_save_status_data();

  /* send data to event broker */
  broker_aggregated_status_data(
    NEBTYPE_AGGREGATEDSTATUS_ENDDUMP,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);

  if (result != OK)
    return (ERROR);
  return (OK);
}

/* cleans up status data before program termination */
int cleanup_status_data(char* config_file, int delete_status_data) {
  return (xsddefault_cleanup_status_data(
            config_file,
            delete_status_data));
}

/* updates program status info */
int update_program_status(int aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (aggregated_dump == FALSE)
    broker_program_status(
      NEBTYPE_PROGRAMSTATUS_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      NULL);
  return (OK);
}

/* updates host status info */
int update_host_status(host* hst, int aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (aggregated_dump == FALSE)
    broker_host_status(
      NEBTYPE_HOSTSTATUS_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      hst,
      NULL);
  return (OK);
}

/* updates service status info */
int update_service_status(service* svc, int aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (aggregated_dump == FALSE)
    broker_service_status(
      NEBTYPE_SERVICESTATUS_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      svc,
      NULL);
  return (OK);
}

/* updates contact status info */
int update_contact_status(contact* cntct, int aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (aggregated_dump == FALSE)
    broker_contact_status(
      NEBTYPE_CONTACTSTATUS_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cntct,
      NULL);
  return (OK);
}
