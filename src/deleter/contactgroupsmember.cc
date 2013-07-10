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

#include "com/centreon/engine/deleter/contactgroupsmember.hh"
#include "com/centreon/engine/objects/contactgroupsmember.hh"

using namespace com::centreon::engine;

/**
 *  Delete contactgroupsmember.
 *
 *  @param[in] ptr The contactgroupsmember to delete.
 */
void deleter::contactgroupsmember(void* ptr) throw () {
  if (!ptr)
    return;

  contactgroupsmember_struct* obj(static_cast<contactgroupsmember_struct*>(ptr));

  delete[] obj->group_name;
  obj->group_name = NULL;

  delete obj;
}
