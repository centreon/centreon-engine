/*
** Copyright 2011-2015 Merethis
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

#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/retention/applier/program.hh"
#include "com/centreon/engine/retention/applier/utils.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::retention;

/**
 *  Restore programe informations.
 *
 *  @param[in, out] config The global configuration to update.
 *  @param[in]      obj    The global informations.
 */
void applier::program::apply(
       configuration::state& config,
       retention::program const& obj) {
  (void)config;

  // XXX: don't use globals, replace it by config!

  if (obj.modified_host_attributes().is_set())
    modified_host_process_attributes = *obj.modified_host_attributes();

  if (obj.modified_service_attributes().is_set())
    modified_service_process_attributes = *obj.modified_service_attributes();

  if (obj.enable_event_handlers().is_set()
      && (modified_host_process_attributes & MODATTR_EVENT_HANDLER_ENABLED))
    enable_event_handlers = *obj.enable_event_handlers();

  if (obj.obsess_over_services().is_set()
      && (modified_service_process_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED))
    obsess_over_services = *obj.obsess_over_services();

  if (obj.obsess_over_hosts().is_set()
      && (modified_host_process_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED))
    obsess_over_hosts = *obj.obsess_over_hosts();

  if (obj.check_service_freshness().is_set()
      && (modified_service_process_attributes & MODATTR_FRESHNESS_CHECKS_ENABLED))
    check_service_freshness = *obj.check_service_freshness();

  if (obj.check_host_freshness().is_set()
      && (modified_host_process_attributes & MODATTR_FRESHNESS_CHECKS_ENABLED))
    check_host_freshness = *obj.check_host_freshness();

  if (obj.enable_flap_detection().is_set()
      && (modified_host_process_attributes & MODATTR_FLAP_DETECTION_ENABLED))
    enable_flap_detection = *obj.enable_flap_detection();

  if (obj.global_host_event_handler().is_set()
      && (modified_host_process_attributes & MODATTR_EVENT_HANDLER_COMMAND)
      && utils::is_command_exist(*obj.global_host_event_handler()))
    string::setstr(global_host_event_handler, *obj.global_host_event_handler());

  if (obj.global_service_event_handler().is_set()
      && (modified_service_process_attributes & MODATTR_EVENT_HANDLER_COMMAND)
      && utils::is_command_exist(*obj.global_service_event_handler()))
    string::setstr(global_service_event_handler, *obj.global_service_event_handler());

  if (obj.next_event_id().is_set())
    next_event_id = *obj.next_event_id();

  if (obj.next_problem_id().is_set())
    next_problem_id = *obj.next_problem_id();

  return ;
}
