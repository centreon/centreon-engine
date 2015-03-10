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

#include <exception>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run disable_hostgroup_svc_notifications test.
 */
static int check_disable_hostgroup_svc_notifications(int argc, char** argv) {
  (void)argc;
  (void)argv;

  host* hst = unittest::add_generic_host();
  if (!hst)
    throw (engine_error() << "create host failed.");

  service* svc = unittest::add_generic_service();
  if (!svc)
    throw (engine_error() << "create service failed.");

  hostgroup* group = add_hostgroup("group", NULL);
  if (!group)
    throw (engine_error() << "create hostgroup failed.");

  hostsmember* hmember = add_host_to_hostgroup(group, "name");
  if (!hmember)
    throw (engine_error() << "host link to hostgroup.");
  hmember->host_ptr = hst;

  servicesmember* smember = add_service_link_to_host(hst, svc);
  if (!smember)
    throw (engine_error() << "service link to servicegroup.");
  smember->service_ptr = svc;

  svc->notifications_enabled = true;
  char const* cmd("[1317196300] DISABLE_HOSTGROUP_SVC_NOTIFICATIONS;group");
  process_external_command(cmd);

  if (svc->notifications_enabled)
    throw (engine_error() << "disable_hostgroup_svc_notifications failed.");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_disable_hostgroup_svc_notifications);
  return (utest.run());
}
