/*
** Copyright 2011-2012 Merethis
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
#include <iostream>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/utils.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check if remove command works with some commands.
 */
static void remove_all_commands() {
  init_object_skiplists();

  add_command("command_name_1", "command_value");
  add_command("command_name_2", "command_value");
  add_command("command_name_3", "command_value");

  if (remove_command_by_id("command_name_2") != 1
      || remove_command_by_id("command_name_3") != 1
      || remove_command_by_id("command_name_1") != 1
      || command_list != NULL
      || command_list_tail != NULL)
    throw (engine_error() << "remove command by id failed.");

  free_object_skiplists();
}

/**
 *  Check if remove command works with invalid call.
 */
static void remove_command_failed() {
  init_object_skiplists();

  if (remove_command_by_id("") == 1)
    throw (engine_error() << "command remove but dosen't exist.");
  if (remove_command_by_id(NULL) == 1)
    throw (engine_error() << "command remove but pointer is NULL.");

  free_object_skiplists();
}

/**
 *  Check if remove command works with host dependency.
 */
static void try_to_remove_command() {
  init_object_skiplists();

  command* cmd = add_command("command_name_1", "command_value");
  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
		       "localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
		       0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
		       0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
		       NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0,
		       0.0, 0.0, 0.0, 0, 0, 0, 0, 0);

  hst->event_handler_ptr = cmd;

  if (remove_command_by_id("command_name_1") == 0
      || command_list == NULL
      || command_list_tail == NULL)
    throw (engine_error() << "remove command who are currently used.");

  hst->event_handler_ptr = NULL;
  hst->check_command_ptr = cmd;

  if (remove_command_by_id("command_name_1") == 0
      || command_list == NULL
      || command_list_tail == NULL)
    throw (engine_error() << "remove command who are currently used.");

  delete[] hst->plugin_output;
  delete[] hst->long_plugin_output;
  delete[] hst->perf_data;
  delete[] hst->statusmap_image;
  delete[] hst->vrml_image;
  delete[] hst->icon_image_alt;
  delete[] hst->icon_image;
  delete[] hst->action_url;
  delete[] hst->notes_url;
  delete[] hst->notes;
  delete[] hst->failure_prediction_options;
  delete[] hst->event_handler;
  delete[] hst->host_check_command;
  delete[] hst->notification_period;
  delete[] hst->check_period;
  delete[] hst->address;
  delete[] hst->alias;
  delete[] hst->display_name;
  delete[] hst->name;
  delete hst;

  host_list = NULL;
  host_list_tail = NULL;

  if (remove_command_by_id("command_name_1") != 1
      || command_list != NULL
      || command_list_tail != NULL)
    throw (engine_error() << "remove command by id failed.");

  free_object_skiplists();
}

/**
 *  Check if remove command works.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    // Tests
    remove_all_commands();
    remove_command_failed();
    try_to_remove_command();
  }
  catch (std::exception const& e) {
    // Exception handling.
    std::cerr << "error: " << e.what() << std::endl;
    free_memory(get_global_macros());
    return (EXIT_FAILURE);
  }

  return (EXIT_SUCCESS);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  com::centreon::engine::unittest utest(argc, argv, &main_test);
  return (utest.run());
}
