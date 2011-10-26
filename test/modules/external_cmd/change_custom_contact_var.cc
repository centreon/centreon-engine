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
 *  Run change_custom_contact_var test.
 */
static void check_change_custom_contact_var() {
  init_object_skiplists();

  contact* cntct = add_contact("name", NULL, NULL, NULL, NULL, NULL, NULL, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  if (!cntct)
    throw (engine_error() << "create contact failed.");

  customvariablesmember* member = add_custom_variable_to_contact(cntct, "_VAR", "TEST");
  if (!member)
    throw (engine_error() << "create customvariablesmember failed.");

  char const* cmd("[1317196300] CHANGE_CUSTOM_CONTACT_VAR;name;_VAR;VALUE");
  process_external_command(cmd);

  if (strcmp(member->variable_value, "VALUE"))
    throw (engine_error() << "change_custom_contact_var failed.");

  delete[] cntct->name;
  delete[] cntct->alias;
  delete cntct;

  delete[] member->variable_name;
  delete[] member->variable_value;
  delete member;

  free_object_skiplists();
}

/**
 *  Check processing of change_custom_contact_var works.
 */
int main_test() {
  logging::engine& engine = logging::engine::instance();
  check_change_custom_contact_var();
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
