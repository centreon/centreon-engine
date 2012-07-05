/*
** Copyright 2011-2012 Merethis
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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/checks.hh"
#include "com/centreon/engine/globals.hh"
#include "test/notifications/first_notif_delay/common.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check that the first_notification_delay parameter works properly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Return value.
  int retval(0);

  // Setup default configuration.
  retval |= first_notif_delay_default_setup();

  if (!retval) {
    // Adjust default setup.
    service_list->current_state = 2;
    service_list->last_hard_state = 2;
    service_list->state_type = HARD_STATE;
    service_list->notified_on_critical = 1;
    service_list->notify_on_recovery = 1;
    contact_list->notify_on_service_recovery = 1;

    // Initialize fake check result.
    check_result cr;
    char output[] = "output";
    memset(&cr, 0, sizeof(cr));
    cr.object_check_type = SERVICE_CHECK;
    cr.host_name = host_list->name;
    cr.service_description = service_list->description;
    cr.check_type = SERVICE_CHECK_ACTIVE;
    cr.scheduled_check = false;
    cr.reschedule_check = false;
    cr.latency = 0.0;
    cr.exited_ok = 1;
    cr.return_code = 0;
    cr.output = output;
    cr.start_time.tv_sec = time(NULL);
    cr.finish_time.tv_sec = cr.start_time.tv_sec;

    // Recovery.
    retval |= handle_async_service_check_result(service_list, &cr);

    // Check that FND was not respected.
    retval |= !file_exists(FLAG_FILE);

    first_notif_delay_default_cleanup();
  }

  // Remove flag file.
  remove(FLAG_FILE);

  return (retval);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  // rewrite basic process to remove QEventLoop.
  return (1);

  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
