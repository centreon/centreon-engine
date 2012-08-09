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
 *  Cleanup host memory.
 */
static void free_host(host* hst) {
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
}

/**
 *  Check if remove service works with some hostgroup.
 */
static void remove_all_hostgroup() {
  init_object_skiplists();

  add_hostgroup("hostgroup_name_1",
		"hostgroup_alias",
		"hostgroup_notes",
		"hostgroup_notes_url",
		"hostgroup_action_url");

  add_hostgroup("hostgroup_name_2",
		"hostgroup_alias",
		"hostgroup_notes",
		"hostgroup_notes_url",
		"hostgroup_action_url");

  add_hostgroup("hostgroup_name_3",
		"hostgroup_alias",
		"hostgroup_notes",
		"hostgroup_notes_url",
		"hostgroup_action_url");

  if (remove_hostgroup_by_id("hostgroup_name_2") != 1
      || remove_hostgroup_by_id("hostgroup_name_1") != 1
      || remove_hostgroup_by_id("hostgroup_name_3") != 1
      || hostgroup_list != NULL
      || hostgroup_list_tail != NULL)
    throw (engine_error() << "remove all hostgroup failed.");

  free_object_skiplists();
}

/**
 *  Check if remove service works with invalid call.
 */
static void remove_hostgroup_failed() {
  init_object_skiplists();

  if (remove_hostgroup_by_id("") == 1)
    throw (engine_error() << "hostgroup remove but dosen't exist.");
  if (remove_hostgroup_by_id(NULL) == 1)
    throw (engine_error() << "hostgroup remove but pointer is NULL.");

  free_object_skiplists();
}

/**
 *  Check if remove service works with some hosts.
 */
static void remove_hostgroup_with_hosts() {
  init_object_skiplists();

  host* hst1 = add_host("host_name_1", "host_display_name", "host_alias",
			"localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
			0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
			0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
			NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0,
			0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  host* hst2 = add_host("host_name_2", "host_display_name", "host_alias",
			"localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
			0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
			0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
			NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0,
			0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  host* hst3 = add_host("host_name_3", "host_display_name", "host_alias",
			"localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
			0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
			0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
			NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0,
			0.0, 0.0, 0.0, 0, 0, 0, 0, 0);

  hostgroup* sg = add_hostgroup("hostgroup_name",
				"hostgroup_alias",
				"hostgroup_notes",
				"hostgroup_notes_url",
				"hostgroup_action_url");

  add_host_to_hostgroup(sg, "host_host_name_1");
  add_host_to_hostgroup(sg, "host_host_name_2");
  add_host_to_hostgroup(sg, "host_host_name_3");

  sg->members->host_ptr = hst1;
  sg->members->next->host_ptr = hst2;
  sg->members->next->next->host_ptr = hst3;

  if (remove_hostgroup_by_id("hostgroup_name") != 1
      || hostgroup_list != NULL
      || hostgroup_list_tail != NULL)
    throw (engine_error() << "remove hostgroup with hostsfailed.");

  free_host(hst1);
  free_host(hst2);
  free_host(hst3);
  free_object_skiplists();
}

/**
 *  Check if remove service works.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    // Tests.
    remove_all_hostgroup();
    remove_hostgroup_failed();
    remove_hostgroup_with_hosts();
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
