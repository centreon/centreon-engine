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

#include <cstdio>
#include <cstring>
#include <ctime>
#include <thread>
#include "com/centreon/engine/checks.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/io/file_stream.hh"
#include "test/notifications/first_notif_delay/common.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**
 *  Check routine.
 *
 *  @param[in] cr Check result.
 *
 *  @return 0 on success.
 */
static int check(std::string const& tmpfile, check_result& cr) {
  // Remove flag file.
  io::file_stream::remove(tmpfile);

  // Return value.
  int retval(0);

  // Process critical status until FND is reached.
  time_t now(time(NULL));
  time_t target(now + FIRST_NOTIF_DELAY);
  while (now < target) {
    cr.start_time.tv_sec = now;
    cr.finish_time.tv_sec = now;
    retval |= handle_async_host_check_result_3x(host_list, &cr);
    retval |= io::file_stream::exists(tmpfile);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    now = time(NULL);
  }

  // FND is reached, process check result to send notification.
  std::this_thread::sleep_for(std::chrono::seconds(2));
  cr.start_time.tv_sec = now;
  cr.finish_time.tv_sec = now;
  retval |= handle_async_host_check_result_3x(host_list, &cr);

  // Check that file flag exists.
  retval |= !io::file_stream::exists(tmpfile);

  return (retval);
}

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

  // tmpfile.
  std::string tmpfile(io::file_stream::temp_path());

  // Setup default configuration.
  retval |= first_notif_delay_default_setup(tmpfile);

  if (!retval) {
    // Initialize fake check result.
    check_result cr;
    char output[] = "output";
    memset(&cr, 0, sizeof(cr));
    cr.object_check_type = HOST_CHECK;
    cr.host_name = host_list->name;
    cr.check_type = HOST_CHECK_ACTIVE;
    cr.scheduled_check = false;
    cr.reschedule_check = false;
    cr.latency = 0.0;
    cr.exited_ok = 1;
    cr.return_code = 2;
    cr.output = output;

    // First check.
    retval |= check(tmpfile, cr);

    // Recovery.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cr.return_code = 0;
    cr.start_time.tv_sec = time(NULL);
    cr.finish_time.tv_sec = cr.start_time.tv_sec;
    retval |= handle_async_host_check_result_3x(host_list, &cr);

    // Check that FND was reset properly.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cr.return_code = 2;
    retval |= check(tmpfile, cr);
  }

  // Remove flag file.
  io::file_stream::remove(tmpfile);

  // Cleanup.
  cleanup();

  return (retval);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
