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

#include "com/centreon/engine/deleter/hostdependency.hh"
#include "com/centreon/engine/objects/hostdependency.hh"

using namespace com::centreon::engine;

/**
 *  Delete hostdependency.
 *
 *  @param[in] ptr The hostdependency to delete.
 */
void deleter::hostdependency(void* ptr) throw () {
  if (!ptr)
    return;

  hostdependency_struct* obj(static_cast<hostdependency_struct*>(ptr));

  delete[] obj->dependent_host_name;
  obj->dependent_host_name = NULL;
  delete[] obj->host_name;
  obj->host_name = NULL;
  delete[] obj->dependency_period;
  obj->dependency_period = NULL;

  delete obj;
}
