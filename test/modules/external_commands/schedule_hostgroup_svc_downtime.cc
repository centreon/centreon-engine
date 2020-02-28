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

#include <cstdlib>
#include <exception>
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run schedule_hostgroup_svc_downtime test.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
static int check_schedule_hostgroup_svc_downtime(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Create target host.
  host* hst(add_host("name", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42, 0,
                     0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
                     0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL,
                     NULL, NULL, NULL, NULL, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0,
                     0));
  if (!hst)
    throw(engine_error() << "host creation failed");

  // Create target service.
  service* svc(add_service(
      "name", "description", NULL, NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL,
      0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, "command", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0, 0));
  if (!svc)
    throw(engine_error() << "service creation failed");

  // Create target host group.
  hostgroup* group(add_hostgroup("group", NULL, NULL, NULL, NULL));
  if (!group)
    throw(engine_error() << "host group creation failed");

  // Create memberships.
  hostsmember* hmember(add_host_to_hostgroup(group, "name"));
  if (!hmember)
    throw(engine_error() << "could not link host to host group");
  hmember->host_ptr = hst;
  servicesmember* smember(add_service_link_to_host(hst, svc));
  if (!smember)
    throw(engine_error() << "could not link service to service group");
  smember->service_ptr = svc;

  // Send external command.
  char const* cmd(
      "[1317196300] "
      "SCHEDULE_HOSTGROUP_SVC_DOWNTIME;group;1317196300;2000000000;0;0;7200;"
      "user;comment");
  process_external_command(cmd);

  // Check.
  if (!scheduled_downtime_list)
    throw(engine_error() << "schedule_hostgroup_svc_downtime failed");

  // Cleanup.
  cleanup();

  return (EXIT_SUCCESS);
}

/**
 *  Init unit test.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_schedule_hostgroup_svc_downtime);
  return (utest.run());
}
