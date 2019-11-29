/*
** Copyright 2000-2008      Ethan Galstad
** Copyright 2011-2019      Centreon
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
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/downtimes/host_downtime.hh"
#include "com/centreon/engine/downtimes/service_downtime.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

downtime::downtime(int type,
                   std::string const& host_name,
                   time_t entry_time,
                   std::string const& author,
                   std::string const& comment,
                   time_t start_time,
                   time_t end_time,
                   bool fixed,
                   uint64_t triggered_by,
                   int32_t duration,
                   uint64_t downtime_id)
    : _type{type},
      _hostname{host_name},
      _entry_time{entry_time},
      _author{author},
      _comment{comment},
      _start_time{start_time},
      _end_time{end_time},
      _fixed{fixed},
      _triggered_by{triggered_by},
      _duration{duration},
      _downtime_id{downtime_id},
      _in_effect{false},
      _comment_id{0},
      _start_flex_downtime{0},
      _incremented_pending_downtime{false} {
  /* don't add triggered downtimes that don't have a valid parent */
  if (triggered_by > 0 &&
      !downtime_manager::instance().find_downtime(ANY_DOWNTIME, triggered_by))
    throw engine_error()
        << "can not add triggered host downtime without a valid parent";

  /* we don't have enough info */
  if (host_name.empty())
    throw engine_error()
        << "can not create a host downtime on host with empty name";
}

downtime::~downtime() {}

/* handles scheduled downtime (id passed from timed event queue) */
int handle_scheduled_downtime_by_id(uint64_t downtime_id) {
  std::shared_ptr<downtime> temp_downtime{
      downtime_manager::instance().find_downtime(ANY_DOWNTIME, downtime_id)};
  /* find the downtime entry */
  if (!temp_downtime)
    return ERROR;

  /* handle the downtime */
  return temp_downtime->handle();
}

/******************************************************************/
/************************ SEARCH FUNCTIONS ************************/
/******************************************************************/

/**
 * @brief  Get the downtime type
 *
 * @return an integer that can be HOST_DOWNTIME = 2 or SERVICE_DOWNTIME = 1
 */
int downtime::get_type() const {
  return _type;
}

/**
 * @brief Get the hostname of the host associated with this downtime.
 *
 * @return A string reference to the host name.
 */
std::string const& downtime::get_hostname() const {
  return _hostname;
}

/**
 * @brief stream operator to output a downtime.
 *
 * @param os The output stream
 * @param dt The downtime to export to this stream.
 *
 * @return The stream.
 */
std::ostream& operator<<(std::ostream& os, downtime const& dt) {
  dt.print(os);
  return os;
}

/**
 * @brief  Get the downtime comment.
 *
 * @return A reference to the downtime comment.
 */
std::string const& downtime::get_comment() const {
  return _comment;
}

/**
 * @brief Get the downtime author.
 *
 * @return A reference to the downtime author.
 */
std::string const& downtime::get_author() const {
  return _author;
}

/**
 * @brief Get the downtime id.
 *
 * @return A 64bits integer.
 */
uint64_t downtime::get_downtime_id() const {
  return _downtime_id;
}

/**
 * @brief Get the id of the parent downtime or 0 if triggered by nothing.
 *
 * @return A 64bits integer.
 */
uint64_t downtime::get_triggered_by() const {
  return _triggered_by;
}

/**
 * @brief Tells if the downtime is fixed (in term of duration) or not.
 *
 * @return boolean.
 */
bool downtime::is_fixed() const {
  return _fixed;
}

/**
 * @brief Get the downtime's entry time.
 *
 * @return A time_t representing the entry time of this downtime.
 */
time_t downtime::get_entry_time() const {
  return _entry_time;
}

/**
 * @brief Get the downtime's start time.
 *
 * @return A time_t representing the start time of this downtime.
 */
time_t downtime::get_start_time() const {
  return _start_time;
}

/**
 * @brief Get the downtime's end time.
 *
 * @return A time_t representing the end time of this downtime.
 */
time_t downtime::get_end_time() const {
  return _end_time;
}

/**
 * @brief Get the downtime's duration.
 *
 * @return The duration is an uint64_t.
 */
int32_t downtime::get_duration() const {
  return _duration;
}

bool downtime::is_in_effect() const {
  return _in_effect;
}

void downtime::_set_in_effect(bool in_effect) {
  _in_effect = in_effect;
}

uint64_t downtime::_get_comment_id() const {
  return _comment_id;
}

void downtime::start_flex_downtime() {
  _start_flex_downtime = true;
}
