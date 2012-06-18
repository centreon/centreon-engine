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

#include <exception>
#include <QDebug>
#include <sstream>
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/objects/servicegroup.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/utils.hh"
#include "test/objects/create_object.hh"

using namespace com::centreon::engine::objects;
using namespace test::objects;

static bool create_and_link(bool has_contacts,
                            bool has_contactgroups,
                            bool has_servicegroups,
                            bool has_custom_variables,
                            bool has_check_period,
                            bool has_notification_period,
                            bool has_cmd_event_handler,
                            bool has_cmd_check_command) {
  init_object_skiplists();
  host* hst = create_host(1);
  service* obj = create_service(1);
  std::vector<contact*> contacts;
  std::vector<contactgroup*> contactgroups;
  std::vector<servicegroup*> servicegroups;
  std::vector<std::string> customvar;
  timeperiod* check_period = NULL;
  timeperiod* notification_period = NULL;
  command* cmd_event_handler = NULL;
  command* cmd_check_command = NULL;
  bool ret = true;

  for (unsigned int i = 0; i < 10; ++i) {
    if (has_contacts == true)
      contacts.push_back(create_contact(i + 1));
    if (has_contactgroups == true)
      contactgroups.push_back(create_contactgroup(i + 1));
    if (has_servicegroups == true)
      servicegroups.push_back(create_servicegroup(i + 1));
    if (has_custom_variables == true) {
      std::ostringstream oss;
      oss << "_VAR" << i + 1 << "=" << i + 1;
      customvar.push_back(oss.str());
    }
  }
  if (has_check_period == true)
    check_period = create_timeperiod(1);
  if (has_notification_period == true)
    notification_period = create_timeperiod(2);
  if (has_cmd_event_handler == true)
    cmd_event_handler = create_command(1);
  if (has_cmd_check_command == true)
    cmd_check_command = create_command(2);

  try {
    link(obj,
         contacts,
         contactgroups,
         servicegroups,
         customvar,
         42,
         check_period,
         notification_period,
         cmd_event_handler,
         cmd_check_command);

    if (obj->initial_state != 42
        || !obj->check_period_ptr
        || obj->check_period_ptr != check_period
        || !obj->notification_period_ptr
        || obj->notification_period_ptr != notification_period
        || obj->event_handler_ptr != cmd_event_handler
        || obj->check_command_ptr != cmd_check_command
        || (has_contacts && !obj->contacts)
        || (has_contactgroups && !obj->contact_groups)
        || (has_custom_variables && !obj->custom_variables))
      ret = false;
  }
  catch (std::exception const& e) {
    (void)e;
    ret = false;
  }

  release(hst);
  release(obj);
  for (unsigned int i = 0; i < 10; ++i) {
    if (has_contacts == true)
      release(contacts[i]);
    if (has_contactgroups == true)
      release(contactgroups[i]);
    if (has_servicegroups == true)
      release(servicegroups[i]);
  }
  release(check_period);
  release(notification_period);
  release(cmd_event_handler);
  release(cmd_check_command);
  free_object_skiplists();
  return (ret);
}

static void link_null_pointer() {
  try {
    std::vector<contact*> contacts;
    std::vector<contactgroup*> contactgroups;
    std::vector<servicegroup*> servicegroups;
    std::vector<std::string> customvar;
    link(static_cast<service*>(NULL),
         contacts,
         contactgroups,
         servicegroups,
         customvar,
         0,
         NULL,
         NULL,
         NULL,
         NULL);
  }
  catch (std::exception const& e) {
    (void)e;
  }
}

static void link_null_name() {
  init_object_skiplists();
  service* obj = NULL;
  try {
    obj = create_service(1);
    std::vector<contact*> contacts;
    std::vector<contactgroup*> contactgroups;
    std::vector<servicegroup*> servicegroups;
    std::vector<std::string> customvar;

    delete [] obj->host_name;
    obj->host_name = NULL;

    link(
      static_cast<service*>(NULL),
      contacts,
      contactgroups,
      servicegroups,
      customvar,
      0,
      NULL,
      NULL,
      NULL,
      NULL);
  }
  catch (std::exception const& e) {
    (void)e;
  }
  release(obj);
  free_object_skiplists();
}

static void link_null_description() {
  init_object_skiplists();
  service* obj = NULL;
  try {
    obj = create_service(1);
    std::vector<contact*> contacts;
    std::vector<contactgroup*> contactgroups;
    std::vector<servicegroup*> servicegroups;
    std::vector<std::string> customvar;

    delete [] obj->description;
    obj->description = NULL;

    link(
      static_cast<service*>(NULL),
      contacts,
      contactgroups,
      servicegroups,
      customvar,
      0,
      NULL,
      NULL,
      NULL,
      NULL);
  }
  catch (std::exception const& e) {
    (void)e;
  }
  release(obj);
  free_object_skiplists();
}

static void link_without_contacts() {
  if (create_and_link(false, true, true, true, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_contactgroups() {
  if (create_and_link(true, false, true, true, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_servicegroups() {
  if (create_and_link(true, true, false, true, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_custom_variables() {
  if (create_and_link(true, true, true, false, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_check_period() {
  if (create_and_link(true, true, true, true, false, true, true, true) == true)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_notification_period() {
  if (create_and_link(true, true, true, true, true, false, true, true) == true)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_cmd_event_handler() {
  if (create_and_link(true, true, true, true, true, true, false, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_cmd_check_command() {
  if (create_and_link(true, true, true, true, true, true, true, false) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_with_valid_objects() {
  if (create_and_link(true, true, true, true, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

int main() {
  com::centreon::engine::logging::engine::load();
  com::centreon::engine::commands::set::load();
  try {
    link_null_pointer();
    link_null_name();
    link_null_description();
    link_without_contacts();
    link_without_contactgroups();
    link_without_servicegroups();
    link_without_custom_variables();
    link_without_check_period();
    link_without_notification_period();
    link_without_cmd_event_handler();
    link_without_cmd_check_command();
    link_with_valid_objects();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}
