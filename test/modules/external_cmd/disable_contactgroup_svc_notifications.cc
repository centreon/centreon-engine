/*
** Copyright 2011 Merethis
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

#include <QCoreApplication>
#include <QDebug>
#include <exception>
#include "test/unittest.hh"
#include "logging/engine.hh"
#include "error.hh"
#include "commands.hh"
#include "globals.hh"

using namespace com::centreon::engine;

/**
 *  Run disable_contactgroup_svc_notifications test.
 */
static void check_disable_contactgroup_svc_notifications() {
  init_object_skiplists();

  contact* cntct = add_contact("name", NULL, NULL, NULL, NULL, NULL, NULL, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  if (!cntct)
    throw (engine_error() << "create contact failed.");

  contactgroup* group = add_contactgroup("group", NULL);
  if (!group)
    throw (engine_error() << "create contactgroup failed.");

  contactsmember* member = add_contact_to_contactgroup(group, "name");
  if (!member)
    throw (engine_error() << "create contactsmember failed.");

  member->contact_ptr = cntct;
  cntct->service_notifications_enabled = true;
  char const* cmd("[1317196300] DISABLE_CONTACTGROUP_SVC_NOTIFICATIONS;group");
  process_external_command(cmd);

  if (cntct->service_notifications_enabled)
    throw (engine_error() << "disable_contactgroup_svc_notifications failed.");

  delete[] member->contact_name;
  delete member;

  delete[] group->group_name;
  delete[] group->alias;
  delete group;

  delete[] cntct->name;
  delete[] cntct->alias;
  delete cntct;

  free_object_skiplists();
}

/**
 *  Check processing of disable_contactgroup_svc_notifications works.
 */
int main_test() {
  logging::engine& engine = logging::engine::instance();
  check_disable_contactgroup_svc_notifications();
  engine.cleanup();
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&main_test);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}
