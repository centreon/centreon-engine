/*
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

#include "com/centreon/engine/configuration/applier/state.hh"
#include <ctime>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/retention/applier/comment.hh"
#include "com/centreon/engine/retention/applier/contact.hh"
#include "com/centreon/engine/retention/applier/downtime.hh"
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
void applier::state::apply(configuration::state& config,
                           retention::state const& state) {
  if (!config.retain_state_information())
    return;

  // send data to event broker.
  broker_retention_data(NEBTYPE_RETENTIONDATA_STARTLOAD, NEBFLAG_NONE,
                        NEBATTR_NONE, NULL);

  try {
    time_t current_time(time(NULL));
    bool scheduling_info_is_ok(false);
    if ((current_time - state.informations().created()) <
        static_cast<time_t>(config.retention_scheduling_horizon()))
      scheduling_info_is_ok = true;

    applier::program app_program;
    app_program.apply(config, state.globals());

    applier::comment app_comments;
    app_comments.apply(state.comments());

    applier::downtime app_downtimes;
    app_downtimes.apply(state.downtimes());

    applier::contact app_contacts;
    app_contacts.apply(config, state.contacts());

    applier::host app_hosts;
    app_hosts.apply(config, state.hosts(), scheduling_info_is_ok);

    applier::service app_services;
    app_services.apply(config, state.services(), scheduling_info_is_ok);
  } catch (...) {
    // send data to event broker.
    broker_retention_data(NEBTYPE_RETENTIONDATA_ENDLOAD, NEBFLAG_NONE,
                          NEBATTR_NONE, NULL);
    throw;
  }

  // send data to event broker.
  broker_retention_data(NEBTYPE_RETENTIONDATA_ENDLOAD, NEBFLAG_NONE,
                        NEBATTR_NONE, NULL);
}
