/*
** Copyright 2000-2008      Ethan Galstad
** Copyright 2011-2013,2016 Centreon
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

#include <cstdlib>
#include <cstring>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/downtimes/host_downtime.hh"
#include "com/centreon/engine/downtimes/service_downtime.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/objects/comment.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

downtime::downtime(int type, std::string const& host_name)
: _type{type}, _hostname{host_name} {}

downtime::downtime(downtime const& other) {}

downtime::downtime(downtime&& other) {}

downtime::~downtime() {}

/* registers scheduled downtime (schedules it, adds comments, etc.) */
int register_downtime(int type, unsigned long downtime_id) {
  /* find the downtime entry in memory */
  std::shared_ptr<downtime> temp_downtime{downtime_manager::instance().find_downtime(type, downtime_id)};
  if (!temp_downtime)
    return ERROR;

  if (temp_downtime->subscribe() == ERROR)
    return ERROR;

  return OK;
}

/* handles scheduled downtime (id passed from timed event queue) */
int handle_scheduled_downtime_by_id(unsigned long downtime_id) {
  std::shared_ptr<downtime> temp_downtime{downtime_manager::instance().find_downtime(ANY_DOWNTIME, downtime_id)};
  /* find the downtime entry */
  if (!temp_downtime)
    return ERROR;

  /* handle the downtime */
  return temp_downtime->handle();
}

/******************************************************************/
/************************ SEARCH FUNCTIONS ************************/
/******************************************************************/

int downtime::get_type() const {
  return _type;
}

std::string const& downtime::get_hostname() const {
  return _hostname;
}

std::ostream& operator<<(std::ostream& os, downtime const& dt) {
  dt.print(os);
  return os;
}
