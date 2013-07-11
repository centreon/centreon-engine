/*
** Copyright 1999-2006 Ethan Galstad
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

#include <fstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/retention/parser.hh"
#include "com/centreon/engine/sretention.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::retention;

/******************************************************************/
/************* TOP-LEVEL STATE INFORMATION FUNCTIONS **************/
/******************************************************************/

/* save all host and service state information */
int save_state_information(int autosave) {
  if (!config->retain_state_information())
    return (OK);

  /* send data to event broker */
  broker_retention_data(
    NEBTYPE_RETENTIONDATA_STARTSAVE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);

  int result(ERROR);
  try {
    std::ofstream stream(
                    config->state_retention_file().c_str(),
                    std::ios::out | std::ios::trunc);
    if (!stream.is_open())
      throw (engine_error() << "retention: can't open retention file: "
             "open " << config->state_retention_file() << " failed");
    dump::header(stream);
    dump::info(stream);
    dump::program(stream);
    dump::hosts(stream);
    dump::services(stream);
    dump::contacts(stream);
    dump::comments(stream);
    dump::downtimes(stream);

    result = OK;
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << e.what();
  }

  /* send data to event broker */
  broker_retention_data(
    NEBTYPE_RETENTIONDATA_ENDSAVE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);

  if (result != ERROR && autosave == TRUE)
    logger(log_process_info, basic)
      << "Auto-save of retention data completed successfully.";

  return (result);
}

/* reads in initial host and state information */
int read_initial_state_information() {
  if (!config->retain_state_information())
    return (OK);

  // send data to event broker.
  broker_retention_data(
    NEBTYPE_RETENTIONDATA_STARTLOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);

  int result(OK);
  try {
    // XXX:
    // parser p;
    // p.parse(config->state_retention_file());
  }
  catch (...) {
    result = ERROR;
  }

  // send data to event broker.
  broker_retention_data(
    NEBTYPE_RETENTIONDATA_ENDLOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);

  return (result);
}
