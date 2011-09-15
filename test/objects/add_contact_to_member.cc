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
#include "macros.hh"
#include "add_object_to_member.hh"
#include "create_object.hh"

using namespace com::centreon::engine::objects;
using namespace test::objects;

int main() {
  try {
    add_with_null_member(&add_contacts_to_object);
    add_without_objects(&add_contacts_to_object);
    add_with_objects(&add_contacts_to_object,
                     &create_contact,
                     1);
    add_with_objects(&add_contacts_to_object,
                     &create_contact,
                     10);
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}
