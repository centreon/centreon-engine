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
#include "com/centreon/engine/objects/commandsmember.hh"

using namespace com::centreon::engine;

/**
 *  Delete commandsmember.
 *
 *  @param[in] ptr The commandsmember to delete.
 */
void deleter::commandsmember(void* ptr) throw () {
  if (!ptr)
    return;

  commandsmember_struct* obj(static_cast<commandsmember_struct*>(ptr));

  delete[] obj->cmd;
  obj->cmd = NULL;

  delete obj;
}
