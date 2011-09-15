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
#include "objects/contactgroup.hh"
#include "objects/contact.hh"
#include "create_object.hh"

using namespace com::centreon::engine::objects;
using namespace test::objects;

static bool create_and_link(bool has_contacts, bool has_contactgroups) {
  init_object_skiplists();
  contactgroup* obj = create_contactgroup(1);
  QVector<contact*> contacts;
  QVector<contactgroup*> contactgroups;
  bool ret = true;

  for (unsigned int i = 0; i < 10; ++i) {
    if (has_contacts == true)
      contacts.push_back(create_contact(i + 1));
    if (has_contactgroups == true) {
      contactgroups.push_back(create_contactgroup(i + 2));
    }
  }

  try {
    link(obj,
         contacts,
         contactgroups);
  }
  catch (std::exception const& e) {
    (void)e;
    ret = false;
  }
  release(obj);
  for (unsigned int i = 0; i < 10; ++i) {
    if (has_contacts == true)
      release(contacts[i]);
    if (has_contactgroups == true)
      release(contactgroups[i]);
  }
  free_object_skiplists();
  return (ret);
}

static void link_null_pointer() {
  try {
    QVector<contactgroup*> contactgroups;
    QVector<contact*> contacts;
    link(static_cast<contactgroup*>(NULL),
         contacts,
         contactgroups);
  }
  catch (std::exception const& e) {
    (void)e;
  }
}

static void link_null_name() {
  init_object_skiplists();
  contactgroup* obj = NULL;
  try {
    obj = create_contactgroup(1);
    QVector<contactgroup*> contactgroups;
    QVector<contact*> contacts;

    delete[] obj->group_name;
    obj->group_name = NULL;

    link(obj,
         contacts,
         contactgroups);
  }
  catch (std::exception const& e) {
    (void)e;
  }
  release(obj);
  free_object_skiplists();
}

static void link_without_contacts() {
  if (create_and_link(false, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_without_contactgroups() {
  if (create_and_link(true, false) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

static void link_with_valid_objects() {
  if (create_and_link(true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return.");
}

int main() {
  try {
    link_null_pointer();
    link_null_name();
    link_without_contacts();
    link_without_contactgroups();
    link_with_valid_objects();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}
