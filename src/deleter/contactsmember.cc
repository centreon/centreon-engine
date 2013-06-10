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

#include "com/centreon/engine/deleter/contactsmember.hh"
#include "com/centreon/engine/objects.hh"

using namespace com::centreon::engine;

/**
 *  Delete contactsmember.
 *
 *  @param[in] ptr The contactsmember to delete.
 */
void deleter::contactsmember(void* ptr) throw () {
  if (!ptr)
    return;

  contactsmember_struct* obj(static_cast<contactsmember_struct*>(ptr));

  delete[] obj->contact_name;
  obj->contact_name = NULL;

  delete obj;
}
