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
 *  Run enable_svc_event_handler test.
 */
static void check_enable_svc_event_handler() {
  init_object_skiplists();

  service* svc = add_service("name", "description", NULL,
                             NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL,
                             0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, "command", 0, 0,
                             0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,
                             0, 0, NULL, NULL, NULL, NULL, NULL,
                             0, 0, 0);
  if (!svc)
    throw (engine_error() << "create service failed.");

  svc->event_handler_enabled = false;
  char const* cmd("[1317196300] ENABLE_SVC_EVENT_HANDLER;name;description");
  process_external_command(cmd);

  if (!svc->event_handler_enabled)
    throw (engine_error() << "enable_svc_event_handler failed.");

  delete[] svc->host_name;
  delete[] svc->description;
  delete[] svc->service_check_command;
  delete[] svc->display_name;
  delete svc;

  free_object_skiplists();
}

/**
 *  Check processing of enable_svc_event_handler works.
 */
int main_test() {
  logging::engine& engine = logging::engine::instance();
  check_enable_svc_event_handler();
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
