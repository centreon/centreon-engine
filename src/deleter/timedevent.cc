/*
** Copyright 2013 Merethis
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

#include "com/centreon/engine/deleter/timedevent.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/timed_event.hh"

using namespace com::centreon::engine;

/**
 *  Delete timed_event.
 *
 *  @param[in] ptr The object to delete.
 */
void deleter::timedevent(void* ptr) throw() {
  timed_event* obj(static_cast<timed_event*>(ptr));
  if (obj->event_type == EVENT_SCHEDULED_DOWNTIME ||
      obj->event_type == EVENT_EXPIRE_COMMENT)
    delete static_cast<unsigned long*>(obj->event_data);
  delete obj;
  return;
}
