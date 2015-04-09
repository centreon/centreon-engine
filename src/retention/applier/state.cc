/*
** Copyright 2011-2013,2015 Merethis
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

#include <ctime>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/retention/applier/host.hh"
#include "com/centreon/engine/retention/applier/program.hh"
#include "com/centreon/engine/retention/applier/service.hh"
#include "com/centreon/engine/retention/applier/state.hh"

using namespace com::centreon::engine::retention;

/**
 *  Restore retention state.
 *
 *  @param[in, out] config The global configuration to update.
 *  @param[in]      state  The retention informations.
 */
void applier::state::apply(
                       configuration::state& config,
                       retention::state const& state) {
  // Send data to event broker.
  broker_retention_data(
    NEBTYPE_RETENTIONDATA_STARTLOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);

  try {
    applier::program app_program;
    app_program.apply(config, state.globals());

    applier::host app_hosts;
    app_hosts.apply(config, state.hosts(), true);

    applier::service app_services;
    app_services.apply(config, state.services(), true);
  }
  catch (...) {
    // Send data to event broker.
    broker_retention_data(
      NEBTYPE_RETENTIONDATA_ENDLOAD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      NULL);
    throw ;
  }

  // Send data to event broker.
  broker_retention_data(
    NEBTYPE_RETENTIONDATA_ENDLOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);

  return ;
}
