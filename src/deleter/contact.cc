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

#include "com/centreon/engine/deleter/commandsmember.hh"
#include "com/centreon/engine/deleter/contactsmember.hh"
#include "com/centreon/engine/deleter/contact.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/objects.hh"

/**
 *  Delete contact.
 *
 *  @param[in] ptr The contact to delete.
 */
void deleter::contact(void* ptr) throw () {
  if (!ptr)
    return;

  contact_struct* obj(static_cast<contact_struct*>(ptr));

  listmember(obj->host_notification_commands, &commandsmember);
  listmember(obj->service_notification_commands, &commandsmember);
  listmember(obj->custom_variables, &contactsmember);
  listmember(obj->contactgroups_ptr, &objectlist);

  // host_notification_period_ptr not free.
  // service_notification_period_ptr not free.

  delete[] obj->name;
  obj->name = NULL;
  delete[] obj->alias;
  obj->alias = NULL;
  delete[] obj->email;
  obj->email = NULL;
  delete[] obj->pager;
  obj->pager = NULL;
  for (unsigned int i(0); i < MAX_CONTACT_ADDRESSES; ++i) {
    delete[] obj->address[i];
    obj->address[i] = NULL;
  }
  delete[] obj->host_notification_period;
  obj->host_notification_period = NULL;
  delete[] obj->service_notification_period;
  obj->service_notification_period = NULL;

  delete obj;
}
