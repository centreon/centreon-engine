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

#include "com/centreon/engine/deleter/servicedependency.hh"
#include "com/centreon/engine/objects/servicedependency.hh"

using namespace com::centreon::engine;

/**
 *  Delete servicedependency.
 *
 *  @param[in] ptr The servicedependency to delete.
 */
void deleter::servicedependency(void* ptr) throw () {
  if (!ptr)
    return;

  servicedependency_struct* obj(static_cast<servicedependency_struct*>(ptr));

  delete[] obj->dependent_host_name;
  obj->dependent_host_name = NULL;
  delete[] obj->dependent_service_description;
  obj->dependent_service_description = NULL;
  delete[] obj->host_name;
  obj->host_name = NULL;
  delete[] obj->service_description;
  obj->service_description = NULL;
  delete[] obj->dependency_period;
  obj->dependency_period = NULL;

  delete obj;
}
