/*
** Copyright 2015 Merethis
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

#include <cstdlib>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/nebcallbacks.hh"
#include "com/centreon/engine/nebmodules.hh"
#include "com/centreon/engine/nebstructs.hh"

// Module support.
NEB_API_VERSION(CURRENT_NEB_API_VERSION)

// Number of expected passive service checks.
int expected_passive_checks(0);
int received_passive_checks(0);

/**
 *  Callback that check number of received passive checks.
 *
 *  @param[in] type  Callback type.
 *  @param[in] arg   Callback argument.
 *
 *  @return 0.
 */
int service_check_callback(int type, void* arg) {
  (void)type;
  nebstruct_service_check_data* scd(
      static_cast<nebstruct_service_check_data*>(arg));
  if (scd && (scd->type == NEBTYPE_SERVICECHECK_PROCESSED) &&
      (scd->check_type == 1)) {
    ++received_passive_checks;
    if (received_passive_checks == expected_passive_checks)
      exit(0);
  }
  return (0);
}

extern "C" {
/**
 *  Module entry point.
 *
 *  @param[in] flags   Unused.
 *  @param[in] args    This should be the number of expected passive
 *                     service checks.
 *  @param[in] handle  Module handle.
 *
 *  @return 0.
 */
int nebmodule_init(int flags, char const* args, void* handle) {
  (void)flags;
  expected_passive_checks = strtol(args, NULL, 0);
  neb_register_callback(NEBCALLBACK_SERVICE_CHECK_DATA, handle, 0,
                        &service_check_callback);
  return (0);
}

/**
 *  Module exit point.
 *
 *  @param[in] flags   Unused.
 *  @param[in] reason  Unused.
 *
 *  @return 0.
 */
int nebmodule_deinit(int flags, int reason) {
  (void)flags;
  (void)reason;
  return (0);
}
}
