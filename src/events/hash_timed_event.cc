/*
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

#include <cstddef>
#include <unordered_map>
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/hash_timed_event.hh"
#include "com/centreon/engine/events/sched_info.hh"
#include "com/centreon/engine/events/timed_event.hh"

using namespace com::centreon::engine::events;

/**
 *  Default constructor.
 */
hash_timed_event::hash_timed_event() {}

/**
 *  Copy constructor
 *
 *  @param[in] right  The object to copy.
 */
hash_timed_event::hash_timed_event(hash_timed_event const& right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
hash_timed_event::~hash_timed_event() {}

/**
 *  Copy operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
hash_timed_event& hash_timed_event::operator=(hash_timed_event const& right) {
  return (_internal_copy(right));
}

/**
 *  Clear hash list.
 *
 *  @param[in] p  The hash list priority to clear.
 */
void hash_timed_event::clear(priority p) {
  for (int i(0); i < type_num; ++i)
    _hevent[i][p].clear();
  return;
}

/**
 *  Clear hash list.
 *
 *  @param[in] p  The hash list priority to clear.
 *  @param[in] t  The hash list type to clear.
 */
void hash_timed_event::clear(priority p, type t) {
  _hevent[t][p].clear();
  return;
}

/**
 *  Remove timed event.
 *
 *  @param[in] p      The hash list priority.
 *  @param[in] event  The event to remove.
 */
void hash_timed_event::erase(priority p, timed_event* event) {
  if (!event)
    return ;
  switch(event->event_type) {
  case EVENT_SERVICE_CHECK:
    _hevent[service_check][p].erase(event->event_data);
    break ;
  case EVENT_HOST_CHECK:
    _hevent[host_check][p].erase(event->event_data);
    break ;
  case EVENT_EXPIRE_SERVICE_ACK:
    _hevent[expire_service_ack][p].erase(event->event_data);
    break ;
  case EVENT_EXPIRE_HOST_ACK:
    _hevent[expire_host_ack][p].erase(event->event_data);
    break ;
  };
  return ;
}

/**
 *  Find an event with it's data.
 *
 *  @param[in] p    The hash list priority.
 *  @param[in] t    The hash list priority.
 *  @param[in] ptr  The event data.
 */
timed_event* hash_timed_event::find(priority p, type t, void* ptr) {
  std::unordered_map<void*, timed_event*>::iterator it(_hevent[t][p].find(ptr));
  if (it == _hevent[t][p].end())
    return (NULL);
  return (it->second);
}

/**
 *  Add timed event.
 *
 *  @param[in] p      The hash list priority.
 *  @param[in] event  The event to add.
 */
void hash_timed_event::insert(priority p, timed_event* event) {
  if (!event || !event->event_data)
    return;
  switch (event->event_type) {
  case EVENT_SERVICE_CHECK:
    _hevent[service_check][p][event->event_data] = event;
    break ;
  case EVENT_HOST_CHECK:
    _hevent[host_check][p][event->event_data] = event;
    break ;
  case EVENT_EXPIRE_SERVICE_ACK:
    _hevent[expire_service_ack][p][event->event_data] = event;
    break ;
  case EVENT_EXPIRE_HOST_ACK:
    _hevent[expire_host_ack][p][event->event_data] = event;
    break ;
  };
  return;
}

/**
 *  Internal copy.
 *
 *  @param[in] other  The object to copy.
 *
 *  @return This object.
 */
hash_timed_event& hash_timed_event::_internal_copy(hash_timed_event const& other) {
  if (this != &other)
    for (int i(0); i < type_num; ++i)
      for (int j(0); j < priority_num; ++j)
        _hevent[i][j] = other._hevent[i][j];
  return (*this);
}
