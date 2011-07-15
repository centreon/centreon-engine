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

#include <QDebug>
#include <exception>
#include "error.hh"
#include "objects.hh"
#include "utils.hh"
#include "macros.hh"
#include "globals.hh"

using namespace com::centreon::engine;

static void remove_all_host() {
  init_object_skiplists();

  add_host("host_name_1", "host_display_name", "host_alias",
	   "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
	   0.0, 0.0, "notification_period", 0, "check_command", 0,
	   0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
	   0, "failure_prediction_options", 0, 0, "notes", "notes_url",
	   "action_url", "icon_image", "icon_image_alt", "vrml_image",
	   "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  add_host("host_name_2", "host_display_name", "host_alias",
	   "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
	   0.0, 0.0, "notification_period", 0, "check_command", 0,
	   0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
	   0, "failure_prediction_options", 0, 0, "notes", "notes_url",
	   "action_url", "icon_image", "icon_image_alt", "vrml_image",
	   "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  add_host("host_name_3", "host_display_name", "host_alias",
	   "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
	   0.0, 0.0, "notification_period", 0, "check_command", 0,
	   0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
	   0, "failure_prediction_options", 0, 0, "notes", "notes_url",
	   "action_url", "icon_image", "icon_image_alt", "vrml_image",
	   "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);

  if (remove_host_by_id("host_name_1") != 1
      || remove_host_by_id("host_name_2") != 1
      || remove_host_by_id("host_name_3") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw ("remove all host failed.");

  free_object_skiplists();
}

static void remove_host_failed() {
  init_object_skiplists();

  if (remove_host_by_id("") == 1)
    throw (engine_error() << "host remove but dosen't exist.");
  if (remove_host_by_id(NULL) == 1)
    throw (engine_error() << "host remove but pointer is NULL.");

  free_object_skiplists();
}

static void remove_host_with_() {
  init_object_skiplists();

  

  free_object_skiplists();
}

int main(void) {
  try {
    remove_all_host();
    remove_host_failed();
    remove_host_with_();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}
