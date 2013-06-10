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

#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/timeperiodexclusion.hh"
#include "com/centreon/engine/deleter/timeperiod.hh"
#include "com/centreon/engine/deleter/timerange.hh"
#include "com/centreon/engine/objects.hh"

using namespace com::centreon::engine;

/**
 *  Delete timeperiod.
 *
 *  @param[in] ptr The timeperiod to delete.
 */
void deleter::timeperiod(void* ptr) throw () {
  if (!ptr)
    return;

  timeperiod_struct* obj(static_cast<timeperiod_struct*>(ptr));

  for (unsigned int i(0);
       i < sizeof(obj->days) / sizeof(obj->days[0]);
       ++i)
    listmember(obj->days[i], &timerange);
  for (unsigned int i(0);
       i < sizeof(obj->exceptions) / sizeof(obj->exceptions[0]);
       ++i)
    listmember(obj->exceptions[i], &timeperiodexclusion);

  delete[] obj->name;
  obj->name = NULL;
  delete[] obj->alias;
  obj->alias = NULL;

  delete obj;
}
