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

#include "com/centreon/engine/deleter/downtime.hh"
#include "com/centreon/engine/objects/downtime.hh"

using namespace com::centreon::engine;

/**
 *  Delete downtime.
 *
 *  @param[in] ptr The downtime to delete.
 */
void deleter::downtime(void* ptr) throw () {
  if (!ptr)
    return;

  scheduled_downtime* obj(static_cast<scheduled_downtime*>(ptr));

  delete[] obj->host_name;
  delete[] obj->service_description;
  delete[] obj->author;
  delete[] obj->comment;
  delete obj;
}
