/*
** Copyright 1999-2006 Ethan Galstad
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

#include "broker.hh"
#include "logging.hh"
#include "configuration/states.hh"
#include "xrddefault.hh"      /* default routines */
#include "sretention.hh"

using namespace com::centreon::engine;

extern configuration::states config;

/******************************************************************/
/************* TOP-LEVEL STATE INFORMATION FUNCTIONS **************/
/******************************************************************/

/* initializes retention data at program start */
int initialize_retention_data(char* config_file) {
  return (xrddefault_initialize_retention_data(config_file));
}

/* cleans up retention data before program termination */
int cleanup_retention_data(char* config_file) {
  return (xrddefault_cleanup_retention_data(config_file));
}

/* save all host and service state information */
int save_state_information(int autosave) {
  int result = OK;

  if (config.get_retain_state_information() == false)
    return (OK);

  /* send data to event broker */
  broker_retention_data(NEBTYPE_RETENTIONDATA_STARTSAVE,
			NEBFLAG_NONE,
                        NEBATTR_NONE,
			NULL);

  result = xrddefault_save_state_information();

  /* send data to event broker */
  broker_retention_data(NEBTYPE_RETENTIONDATA_ENDSAVE,
			NEBFLAG_NONE,
                        NEBATTR_NONE,
			NULL);

  if (result == ERROR)
    return (ERROR);

  if (autosave == TRUE)
    logit(NSLOG_PROCESS_INFO, FALSE, "Auto-save of retention data completed successfully.\n");

  return (OK);
}

/* reads in initial host and state information */
int read_initial_state_information(void) {
  int result = OK;

  if (config.get_retain_state_information() == false)
    return (OK);

  /* send data to event broker */
  broker_retention_data(NEBTYPE_RETENTIONDATA_STARTLOAD,
			NEBFLAG_NONE,
                        NEBATTR_NONE,
			NULL);

  result = xrddefault_read_state_information();

  /* send data to event broker */
  broker_retention_data(NEBTYPE_RETENTIONDATA_ENDLOAD,
			NEBFLAG_NONE,
                        NEBATTR_NONE,
			NULL);

  if (result == ERROR)
    return (ERROR);

  return (OK);
}
