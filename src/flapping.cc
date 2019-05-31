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

#include <iomanip>
#include <sstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/******************************************************************/
/***************** FLAP DETECTION STATUS FUNCTIONS ****************/
/******************************************************************/

/* enables flap detection on a program wide basis */
void enable_flap_detection_routines() {
  com::centreon::engine::host* temp_host = NULL;
  com::centreon::engine::service* temp_service = NULL;
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "enable_flap_detection_routines()";

  /* bail out if we're already set */
  if (config->enable_flap_detection())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* set flap detection flag */
  config->enable_flap_detection(true);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE, NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  /* update program status */
  update_program_status(false);

  /* check for flapping */
  for (host_map::iterator
         it(com::centreon::engine::host::hosts.begin()),
         end(com::centreon::engine::host::hosts.end());
       it != end;
       ++it)
    it->second->check_for_flapping(false, false, true);
  for (temp_service = service_list;
       temp_service != NULL;
       temp_service = temp_service->next)
    temp_service->check_for_flapping(false, true);
}

/* disables flap detection on a program wide basis */
void disable_flap_detection_routines() {
  com::centreon::engine::host* temp_host = NULL;
  com::centreon::engine::service* temp_service = NULL;
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "disable_flap_detection_routines()";

  /* bail out if we're already set */
  if (!config->enable_flap_detection())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* set flap detection flag */
  config->enable_flap_detection(false);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE, NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  /* update program status */
  update_program_status(false);

  /* handle the details... */
  for (host_map::iterator
         it(com::centreon::engine::host::hosts.begin()),
         end(com::centreon::engine::host::hosts.end());
       it != end;
       ++it)
    handle_host_flap_detection_disabled(it->second.get());
  for (temp_service = service_list;
       temp_service != NULL;
       temp_service = temp_service->next)
    handle_service_flap_detection_disabled(temp_service);
  return;
}

/* handles the details for a host when flap detection is disabled (globally or per-host) */
void handle_host_flap_detection_disabled(com::centreon::engine::host* hst) {
  logger(dbg_functions, basic)
    << "handle_host_flap_detection_disabled()";

  if (hst == NULL)
    return;

  /* if the host was flapping, remove the flapping indicator */
  if (hst->get_is_flapping()) {
    hst->set_is_flapping(false);

    /* delete the original comment we added earlier */
    if (hst->get_flapping_comment_id() != 0)
      comment::delete_comment(hst->get_flapping_comment_id());
    hst->set_flapping_comment_id(0);

    /* log a notice - this one is parsed by the history CGI */
    logger(log_info_message, basic)
      << "HOST FLAPPING ALERT: " << hst->get_name()
      << ";DISABLED; Flap detection has been disabled";

    /* send data to event broker */
    broker_flapping_data(
      NEBTYPE_FLAPPING_STOP,
      NEBFLAG_NONE,
      NEBATTR_FLAPPING_STOP_DISABLED,
      HOST_FLAPPING,
      hst,
      hst->get_percent_state_change(),
      0.0,
      0.0,
      NULL);

    /* send a notification */
    hst->notify(
      NOTIFICATION_FLAPPINGDISABLED,
      NULL,
      NULL,
      NOTIFICATION_OPTION_NONE);

    /* should we send a recovery notification? */
    if (hst->get_check_flapping_recovery_notification()
        && hst->get_current_state() == HOST_UP)
      hst->notify(
        NOTIFICATION_NORMAL,
        NULL,
        NULL,
        NOTIFICATION_OPTION_NONE);

    /* clear the recovery notification flag */
    hst->set_check_flapping_recovery_notification(false);
  }

  /* update host status */
  hst->update_status(false);
}

/* handles the details for a service when flap detection is disabled (globally or per-service) */
void handle_service_flap_detection_disabled(com::centreon::engine::service* svc) {
  logger(dbg_functions, basic)
    << "handle_service_flap_detection_disabled()";

  if (svc == NULL)
    return;

  /* if the service was flapping, remove the flapping indicator */
  if (svc->is_flapping) {
    svc->is_flapping = false;

    /* delete the original comment we added earlier */
    if (svc->flapping_comment_id != 0)
      comment::delete_comment(svc->flapping_comment_id);
    svc->flapping_comment_id = 0;

    /* log a notice - this one is parsed by the history CGI */
    logger(log_info_message, basic)
      << "SERVICE FLAPPING ALERT: " << svc->get_hostname()
      << ";" << svc->get_description()
      << ";DISABLED; Flap detection has been disabled";

    /* send data to event broker */
    broker_flapping_data(
      NEBTYPE_FLAPPING_STOP,
      NEBFLAG_NONE,
      NEBATTR_FLAPPING_STOP_DISABLED,
      SERVICE_FLAPPING,
      svc,
      svc->percent_state_change,
      0.0,
      0.0,
      NULL);

    /* send a notification */
    svc->notify(
      NOTIFICATION_FLAPPINGDISABLED,
      NULL,
      NULL,
      NOTIFICATION_OPTION_NONE);

    /* should we send a recovery notification? */
    if (svc->check_flapping_recovery_notification
        && svc->current_state == STATE_OK)
      svc->notify(
        NOTIFICATION_NORMAL,
        NULL,
        NULL,
        NOTIFICATION_OPTION_NONE);

    /* clear the recovery notification flag */
    svc->check_flapping_recovery_notification = false;
  }

  /* update service status */
  svc->update_status(false);
}
