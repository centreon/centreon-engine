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
#include "utils.hh"
#include "macros.hh"
#include "objects/host.hh"
#include "objects/contact.hh"
#include "objects/contactgroup.hh"
#include "objects/hostgroup.hh"
#include "objects/timeperiod.hh"
#include "objects/command.hh"
#include "create_object.hh"

using namespace com::centreon::engine::objects;
using namespace test::objects;

static bool create_and_link(bool has_parents,
                            bool has_contacts,
                            bool has_contactgroups,
                            bool has_hostgroups,
                            bool has_custom_variables,
                            bool has_check_period,
                            bool has_notification_period,
                            bool has_cmd_event_handler,
                            bool has_cmd_check_command) {
  init_object_skiplists();
  host* obj = create_host(1);
  QVector<host*> parents;
  QVector<contact*> contacts;
  QVector<contactgroup*> contactgroups;
  QVector<hostgroup*> hostgroups;
  QVector<QString> customvar;
  timeperiod* check_period = NULL;
  timeperiod* notification_period = NULL;
  command* cmd_event_handler = NULL;
  command* cmd_check_command = NULL;
  bool ret = true;

  for (unsigned int i = 0; i < 10; ++i) {
    if (has_parents == true)
      parents.push_back(create_host(i + 2));
    if (has_contacts == true)
      contacts.push_back(create_contact(i + 1));
    if (has_contactgroups == true)
      contactgroups.push_back(create_contactgroup(i + 1));
    if (has_hostgroups == true)
      hostgroups.push_back(create_hostgroup(i + 1));
    if (has_custom_variables == true)
      customvar.push_back(QString("_VAR%1=%1").arg(i + 1));
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
         parents,
         contacts,
         contactgroups,
         hostgroups,
         customvar,
         0,
         check_period,
         notification_period,
         cmd_event_handler,
         cmd_check_command);
  }
  catch (std::exception const& e) {
    (void)e;
    ret = false;
  }

  release(obj);
  for (unsigned int i = 0; i < 10; ++i) {
    if (has_parents == true)
      release(parents[i]);
    if (has_contacts == true)
      release(contacts[i]);
    if (has_contactgroups == true)
      release(contactgroups[i]);
    if (has_hostgroups == true)
      release(hostgroups[i]);
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
    QVector<host*> parents;
    QVector<contact*> contacts;
    QVector<contactgroup*> contactgroups;
    QVector<hostgroup*> hostgroups;
    QVector<QString> customvar;
    link(static_cast<host*>(NULL),
         parents,
         contacts,
         contactgroups,
         hostgroups,
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
  host* obj = NULL;
  try {
    obj = create_host(1);
    QVector<host*> parents;
    QVector<contact*> contacts;
    QVector<contactgroup*> contactgroups;
    QVector<hostgroup*> hostgroups;
    QVector<QString> customvar;

    delete[] obj->name;
    obj->name = NULL;

    link(static_cast<host*>(NULL),
         parents,
         contacts,
         contactgroups,
         hostgroups,
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

static void link_without_parents() {
  if (create_and_link(false, true, true, true, true, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_without_contacts() {
  if (create_and_link(true, false, true, true, true, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_without_contactgroups() {
  if (create_and_link(true, true, false, true, true, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_without_hostgroups() {
  if (create_and_link(true, true, true, false, true, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_without_custom_variables() {
  if (create_and_link(true, true, true, true, false, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_without_check_period() {
  if (create_and_link(true, true, true, true, true, false, true, true, true) == true)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_without_notification_period() {
  if (create_and_link(true, true, true, true, true, true, false, true, true) == true)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_without_cmd_event_handler() {
  if (create_and_link(true, true, true, true, true, true, true, false, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_without_cmd_check_command() {
  if (create_and_link(true, true, true, true, true, true, true, true, false) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_with_valid_objects() {
  if (create_and_link(true, true, true, true, true, true, true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

int main() {
  try {
    link_null_pointer();
    link_null_name();
    link_without_parents();
    link_without_contacts();
    link_without_contactgroups();
    link_without_hostgroups();
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
