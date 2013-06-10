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

#include "com/centreon/engine/deleter/servicesmember.hh"
#include "com/centreon/engine/objects.hh"

using namespace com::centreon::engine;

/**
 *  Delete servicesmember.
 *
 *  @param[in] ptr The servicesmember to delete.
 */
void deleter::servicesmember(void* ptr) throw () {
  if (!ptr)
    return;

  servicesmember_struct* obj(static_cast<servicesmember_struct*>(ptr));

  delete[] obj->host_name;
  obj->host_name = NULL;
  delete[] obj->service_description;
  obj->service_description = NULL;

  delete obj;
}
