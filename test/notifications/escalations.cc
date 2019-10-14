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
#include "com/centreon/engine/checks.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/io/file_stream.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**
 *  Init host and service.
 */
void init_host_and_service(host*& hst, service*& svc) {
  hst = add_host("name", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42,
                       0, 0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0,
                       0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
                       NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0.0, 0.0,
                       0.0, 0, 0, 0, 0, 0);
  if (!hst)
    throw (engine_error() << "create host failed.");

  svc = add_service("name", "description", NULL,
                             NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL,
                             0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, "command", 0, 0,
                             0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,
                             0, 0, NULL, NULL, NULL, NULL, NULL,
                             0, 0, 0);
  if (!svc)
    throw (engine_error() << "create service failed.");
  svc->host_ptr = hst;

  timeperiod* tperiod = add_timeperiod("tperiod", "alias");
  if (!tperiod)
    throw (engine_error() << "create timeperiod failed.");

  for (int i = 0; i < 6; ++i)
    if (!add_timerange_to_timeperiod(tperiod, i, 0, 86400))
      throw (engine_error() << "create timerange failed.");
}

/**
 *  Check the host escalations.
 */
void check_host_escalation(host* hst) {
  hostescalation* host_escalation = add_host_escalation("name",
                                                        0,
                                                        1,
                                                        1,
                                                        "tperiod",
                                                        true,
                                                        true,
                                                        true);
  if (!host_escalation)
    throw (engine_error() << "cannot create host escalation.");
  if (host_notification(hst, NOTIFICATION_NORMAL, NULL, NULL,
                        NOTIFICATION_OPTION_FORCED) != OK)
    throw (engine_error() << "host notification failed." );
}

/**
 *  Check the service escalations.
 */
void check_service_escalation(service* svc) {
  serviceescalation* service_escalation = add_service_escalation("name",
                                                                 "description",
                                                                 0,
                                                                 1,
                                                                 2,
                                                                 "tperiod",
                                                                 true,
                                                                 true,
                                                                 true,
                                                                 true);

  if (!service_escalation)
    throw (engine_error() << "cannot create service escalation");
  if (service_notification(svc, NOTIFICATION_NORMAL, NULL, NULL,
                           NOTIFICATION_OPTION_FORCED) != OK)
    throw (engine_error() << "service notification failed.");
}

/**
 *  Check that host escalation works properly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  host* hst;
  service* svc;

  init_host_and_service(hst, svc);
  check_host_escalation(hst);
  check_service_escalation(svc);

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
