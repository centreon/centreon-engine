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

#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/retention/downtime.hh"

using namespace com::centreon::engine;

/**
 *  Constructor.
 *
 *  @param[in] type This is a host or service downtime.
 */
retention::downtime::downtime(type_id downtime_type)
  : object(object::downtime),
    _downtime_id(0),
    _downtime_type(downtime_type),
    _duration(0),
    _end_time(0),
    _entry_time(0),
    _fixed(false),
    _start_time(0),
    _triggered_by(0) {

}

/**
 *  Destructor.
 */
retention::downtime::~downtime() throw () {
  _finished();
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::downtime::set(
       std::string const& key,
       std::string const& value) {
  if (key == "host_name")
    _host_name = value;
  else if (key == "service_description")
    _service_description = value;
  else if (key == "downtime_id")
    misc::to(value, _downtime_id);
  else if (key == "entry_time")
    misc::to(value, _entry_time);
  else if (key == "start_time")
    misc::to(value, _start_time);
  else if (key == "end_time")
    misc::to(value, _end_time);
  else if (key == "fixed")
    misc::to(value, _fixed);
  else if (key == "triggered_by")
    misc::to(value, _triggered_by);
  else if (key == "duration")
    misc::to(value, _duration);
  else if (key == "author")
    _author = value;
  else if (key == "comment")
    _comment_data = value;
  else
    return (false);
  return (true);
}

/**
 *  Add host downtime.
 */
void retention::downtime::_add_host_downtime() throw () {
  add_host_downtime(
    _host_name.c_str(),
    _entry_time,
    _author.c_str(),
    _comment_data.c_str(),
    _start_time,
    _end_time,
    _fixed,
    _triggered_by,
    _duration,
    _downtime_id);
  register_downtime(HOST_DOWNTIME, _downtime_id);
}

/**
 *  Add serivce downtime.
 */
void retention::downtime::_add_service_downtime() throw () {
  add_service_downtime(
    _host_name.c_str(),
    _service_description.c_str(),
    _entry_time,
    _author.c_str(),
    _comment_data.c_str(),
    _start_time,
    _end_time,
    _fixed,
    _triggered_by,
    _duration,
    _downtime_id);
  register_downtime(SERVICE_DOWNTIME, _downtime_id);
}

/**
 *  Finish all downtime update.
 */
void retention::downtime::_finished() throw () {
  if (_downtime_type == host)
    _add_host_downtime();
  else
    _add_service_downtime();
}

