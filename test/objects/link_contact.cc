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
#include <iostream>
#include <sstream>
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/utils.hh"
#include "test/objects/create_object.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::objects;
using namespace test::objects;

static bool create_and_link(bool has_host_notification_period,
                            bool has_service_notification_period,
                            bool has_contactgroups,
                            bool has_host_notification_commands,
                            bool has_service_notification_commands,
                            bool has_custom_variables) {
  init_object_skiplists();
  contact* obj = create_contact(1);
  timeperiod* hst_notif_period = NULL;
  timeperiod* svc_notif_period = NULL;
  std::vector<contactgroup*> contactgroups;
  std::vector<command*> hst_notif_cmd;
  std::vector<command*> svc_notif_cmd;
  std::vector<std::string> customvar;
  bool ret = true;

  if (has_host_notification_period == true)
    hst_notif_period = create_timeperiod(1);
  if (has_service_notification_period == true)
    svc_notif_period = create_timeperiod(2);
  for (unsigned int i = 0; i < 10; ++i) {
    if (has_contactgroups == true)
      contactgroups.push_back(create_contactgroup(i + 1));
    if (has_host_notification_commands == true)
      hst_notif_cmd.push_back(create_command(i + 1));
    if (has_service_notification_commands == true)
      svc_notif_cmd.push_back(create_command(i + 20));
    if (has_custom_variables == true) {
      std::ostringstream oss;
      oss << "_VAR" << i + 1 << "=" << i + 1;
      customvar.push_back(oss.str());
    }
  }

  try {
    link(obj,
         hst_notif_period,
         svc_notif_period,
         contactgroups,
         hst_notif_cmd,
         svc_notif_cmd,
         customvar);
    if ((has_host_notification_period && !obj->host_notification_period_ptr)
        || (has_service_notification_period && !obj->service_notification_period_ptr)
        || (has_contactgroups && !obj->contactgroups_ptr)
        || (has_host_notification_commands && !obj->host_notification_commands)
        || (has_service_notification_commands && !obj->service_notification_commands)
        || (has_custom_variables && !obj->custom_variables))
      ret = false;
  }
  catch (std::exception const& e) {
    (void)e;
    ret = false;
  }
  release(obj);
  release(hst_notif_period);
  release(svc_notif_period);
  for (unsigned int i = 0; i < 10; ++i) {
    if (has_contactgroups == true)
      release(contactgroups[i]);
    if (has_host_notification_commands == true)
      release(hst_notif_cmd[i]);
    if (has_service_notification_commands == true)
      release(svc_notif_cmd[i]);
  }
  free_object_skiplists();
  return (ret);
}

static void link_null_pointer() {
  try {
    std::vector<contactgroup*> contactgroups;
    std::vector<command*> hst_notif_cmd;
    std::vector<command*> svc_notif_cmd;
    std::vector<std::string> customvar;
    link(static_cast<contact*>(NULL),
         NULL,
         NULL,
         contactgroups,
         hst_notif_cmd,
         svc_notif_cmd,
         customvar);
  }
  catch (std::exception const& e) {
    (void)e;
  }
}

static void link_null_name() {
  init_object_skiplists();
  contact* obj = NULL;
  try {
    obj = create_contact(1);
    std::vector<contactgroup*> contactgroups;
    std::vector<command*> hst_notif_cmd;
    std::vector<command*> svc_notif_cmd;
    std::vector<std::string> customvar;

    delete[] obj->name;
    obj->name = NULL;

    link(obj,
         NULL,
         NULL,
         contactgroups,
         hst_notif_cmd,
         svc_notif_cmd,
         customvar);
  }
  catch (std::exception const& e) {
    (void)e;
  }
  release(obj);
  free_object_skiplists();
}

static void link_without_host_notification_period() {
  if (create_and_link(false, true, true, true, true, true) == true)
    throw (engine_error() << __func__ << " invalid return");
}

static void link_without_service_notification_period() {
  if (create_and_link(true, false, true, true, true, true) == true)
    throw (engine_error() << __func__ << " invalid return");
}

static void link_without_contactgroups() {
  if (create_and_link(true, true, false, true, true, true) == false)
    throw (engine_error() << __func__ << " invalid return");
}

static void link_without_host_notification_commands() {
  if (create_and_link(true, true, true, false, true, true) == false)
    throw (engine_error() << __func__ << " invalid return");
}

static void link_without_service_notification_commands() {
  if (create_and_link(true, true, true, true, false, true) == false)
    throw (engine_error() << __func__ << " invalid return");
}

static void link_without_custom_variables() {
  if (create_and_link(true, true, true, true, true, false) == false)
    throw (engine_error() << __func__ << " invalid return");
}

static void link_with_valid_objects() {
  if (create_and_link(true, true, true, true, true, true) == false)
    throw (engine_error() << __func__ << " invalid return");
}

/**
 *  Check linkage of contact.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    // Tests.
    link_null_pointer();
    link_null_name();
    link_without_host_notification_period();
    link_without_service_notification_period();
    link_without_contactgroups();
    link_without_host_notification_commands();
    link_without_service_notification_commands();
    link_without_custom_variables();
    link_with_valid_objects();
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
