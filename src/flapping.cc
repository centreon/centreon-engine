/*
** Copyright 2001-2009 Ethan Galstad
** Copyright 2011-2019 Centreon
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

#include "com/centreon/engine/flapping.hh"
#include <iomanip>
#include <sstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/******************************************************************/
/***************** FLAP DETECTION STATUS FUNCTIONS ****************/
/******************************************************************/

/* enables flap detection on a program wide basis */
void enable_flap_detection_routines() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic) << "enable_flap_detection_routines()";

  /* bail out if we're already set */
  if (config->enable_flap_detection())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* set flap detection flag */
  config->enable_flap_detection(true);

  /* send data to event broker */
  broker_adaptive_program_data(NEBTYPE_ADAPTIVEPROGRAM_UPDATE, NEBFLAG_NONE,
                               NEBATTR_NONE, CMD_NONE, attr,
                               modified_host_process_attributes, attr,
                               modified_service_process_attributes, NULL);

  /* update program status */
  update_program_status(false);

  /* check for flapping */
  for (host_map::iterator it(com::centreon::engine::host::hosts.begin()),
       end(com::centreon::engine::host::hosts.end());
       it != end; ++it)
    it->second->check_for_flapping(false, false, true);
  for (service_map::iterator it(service::services.begin()),
       end(service::services.end());
       it != end; ++it)
    it->second->check_for_flapping(false, true);
}

/* disables flap detection on a program wide basis */
void disable_flap_detection_routines() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic) << "disable_flap_detection_routines()";

  /* bail out if we're already set */
  if (!config->enable_flap_detection())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* set flap detection flag */
  config->enable_flap_detection(false);

  /* send data to event broker */
  broker_adaptive_program_data(NEBTYPE_ADAPTIVEPROGRAM_UPDATE, NEBFLAG_NONE,
                               NEBATTR_NONE, CMD_NONE, attr,
                               modified_host_process_attributes, attr,
                               modified_service_process_attributes, NULL);

  /* update program status */
  update_program_status(false);

  /* handle the details... */
  for (host_map::iterator it(com::centreon::engine::host::hosts.begin()),
       end(com::centreon::engine::host::hosts.end());
       it != end; ++it)
    it->second->handle_flap_detection_disabled();
  for (service_map::iterator it(service::services.begin()),
       end(service::services.end());
       it != end; ++it)
    it->second->handle_flap_detection_disabled();
}

// disables flap detection for a particular host
void disable_host_flap_detection(host* hst) {
  hst->disable_flap_detection();
}

// enables flap detection for a particular host
void enable_host_flap_detection(host* hst) {
  hst->enable_flap_detection();
}

// enables flap detection for a particular service
void enable_service_flap_detection(com::centreon::engine::service* svc) {
  svc->enable_flap_detection();
}

// disables flap detection for a particular service
void disable_service_flap_detection(service* svc) {
  svc->disable_flap_detection();
}
