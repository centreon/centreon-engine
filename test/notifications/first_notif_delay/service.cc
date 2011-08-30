/*
** Copyright 2011 Merethis
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

#include <QCoreApplication>
#include <QFile>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "checks.hh"
#include "common.hh"
#include "globals.hh"

using namespace com::centreon::engine;

/**
 *  Check routine.
 *
 *  @param[in] cr Check result.
 *
 *  @return 0 on success.
 */
static int check(check_result& cr) {
  // Remove flag file.
  QFile::remove(FLAG_FILE);

  // Return value.
  int retval(0);

  // Process critical status until FND is reached.
  time_t now(time(NULL));
  time_t target(now + FIRST_NOTIF_DELAY);
  while (now < target) {
    cr.start_time.tv_sec = now;
    cr.finish_time.tv_sec = now;
    retval |= handle_async_service_check_result(service_list, &cr);
    retval |= QFile::exists(FLAG_FILE);
    sleep(1);
    now = time(NULL);
  }

  // FND is reached, process check result to send notification.
  sleep(2);
  cr.start_time.tv_sec = now;
  cr.finish_time.tv_sec = now;
  retval |= handle_async_service_check_result(service_list, &cr);

  // Check that file flag exists.
  retval |= !QFile::exists(FLAG_FILE);

  return (retval);
}

/**
 *  Check that the first_notification_delay parameter works properly.
 *
 *  @return 0 on success.
 */
int main(int argc, char* argv[]) {
  // Qt Core object.
  QCoreApplication app(argc, argv);

  // Return value.
  int retval(0);

  // Setup default configuration.
  retval |= first_notif_delay_default_setup();

  if (!retval) {
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
    cr.return_code = 2;
    cr.output = output;

    // First check.
    retval |= check(cr);

    // Recovery.
    sleep(1);
    cr.return_code = 0;
    cr.start_time.tv_sec = time(NULL);
    cr.finish_time.tv_sec = cr.start_time.tv_sec;
    retval |= handle_async_service_check_result(service_list, &cr);

    // Check that FND was reset properly.
    sleep(1);
    cr.return_code = 2;
    retval |= check(cr);
  }

  // Remove flag file.
  QFile::remove(FLAG_FILE);

  return (retval);
}
