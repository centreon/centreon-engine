/*
** Copyright 2013 Merethis
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

#include <cstring>
#include <ctime>
#include <exception>
#include <iostream>
#include <fstream>
#include <libgen.h>
#include <sys/types.h>
#include <signal.h>
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/timeperiod.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros/misc.hh"
#include "com/centreon/engine/objects/downtime.hh"
#include "com/centreon/engine/objects/timerange.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/string.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class exception_success : public std::exception {};

void check_downtimes_and_terminate() {
  unsigned int num_downtimes(0);
  for (scheduled_downtime* temp_downtime = scheduled_downtime_list;
       temp_downtime != NULL;
       temp_downtime = temp_downtime->next, ++num_downtimes) {
    if (strcmp("name", temp_downtime->host_name) != 0 ||
        temp_downtime->recurring_interval != 2 ||
        temp_downtime->recurring_period == NULL)
      throw (engine_error() << "downtime not recurring: invalid downtime "
                            << "parameters.");
  }
  if (num_downtimes != 3)
    throw (engine_error() << "downtime not recurring: invalid number of "
                          << "downtimes (got " << num_downtimes << ").");
  throw (exception_success());
}

/**
 *  Check that the recurrent downtime works correctly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  (void) argc;
  (void) argv;

  try {
    host* hst = add_host("name", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42,
                         0, 0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0,
                         0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
                         NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0.0, 0.0,
                         0.0, 0, 0, 0, 0, 0);
    if (!hst)
      throw (engine_error() << "create host failed.");

    timeperiod* tperiod = add_timeperiod("tperiod", "alias");
    if (!tperiod)
      throw (engine_error() << "create timeperiod failed.");

    for (int i = 0; i < 7; ++i)
      if (!add_timerange_to_timeperiod(tperiod, i, 0, 86400))
        throw (engine_error() << "create timerange failed.");

    next_downtime_id = 1;
    unsigned long downtime_id;
    scheduled_downtime_list = NULL;
    if (schedule_downtime(HOST_DOWNTIME,
                          "name",
                          NULL,
                          time(NULL),
                          NULL,
                          NULL,
                          time(NULL),
                          time(NULL) + 2,
                          0,
                          0,
                          0,
                          2,
                          tperiod,
                          &downtime_id) != OK)
      throw (engine_error() << "create downtime failed.");

    schedule_new_event(
      EVENT_USER_FUNCTION,
      true,
      time(NULL) + 5,
      false,
      0,
      NULL,
      false,
      (void*) check_downtimes_and_terminate,
      NULL,
      0);

    com::centreon::engine::events::loop::instance().run();

  }
  catch (exception_success) {
    return (0);
  }
  catch (...) {
    free_memory(get_global_macros());
    throw;
  }
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
