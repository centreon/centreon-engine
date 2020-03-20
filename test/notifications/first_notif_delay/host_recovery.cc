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
#include "com/centreon/engine/check_result.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/io/file_stream.hh"
#include "test/notifications/first_notif_delay/common.hh"
#include "test/unittest.hh"

using namespace com::centreon;
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

  // tmpfile.
  std::string tmpfile(io::file_stream::temp_path());

  // Setup default configuration.
  retval |= first_notif_delay_default_setup(tmpfile);

  if (!retval) {
    // Adjust default setup.
    host_list->current_state = 1;
    host_list->last_hard_state = 1;
    host_list->state_type = HARD_STATE;
    host_list->notified_on_down = 1;
    host_list->notify_on_recovery = 1;
    contact_list->notify_on_host_recovery = 1;

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
    cr.return_code = 0;
    cr.output = output;
    cr.start_time.tv_sec = time(NULL);
    cr.finish_time.tv_sec = cr.start_time.tv_sec;

    // Recovery.
    retval |= handle_async_host_check_result_3x(host_list, &cr);

    // Check that FND was not respected.
    retval |= !io::file_stream::remove(tmpfile);
  }

  // Remove flag file.
  io::file_stream::remove(tmpfile);

  return (retval);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
