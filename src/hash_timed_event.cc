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

#include <cstddef>
#include "com/centreon/engine/hash_timed_event.hh"

using namespace com::centreon::engine;

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
  _hevent[service_check][p].clear();
  _hevent[host_check][p].clear();
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
    return;
  if (event->event_type == EVENT_SERVICE_CHECK)
    _hevent[service_check][p].erase(event->event_data);
  else if (event->event_type == EVENT_HOST_CHECK)
    _hevent[host_check][p].erase(event->event_data);
  return;
}

/**
 *  Find an event with it's data.
 *
 *  @param[in] p    The hash list priority.
 *  @param[in] t    The hash list priority.
 *  @param[in] ptr  The event data.
 */
timed_event* hash_timed_event::find(priority p, type t, void* ptr) {
  umap<void*, timed_event*>::iterator it(_hevent[t][p].find(ptr));
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
  if (event->event_type == EVENT_SERVICE_CHECK)
    _hevent[service_check][p][event->event_data] = event;
  else if (event->event_type == EVENT_HOST_CHECK)
    _hevent[host_check][p][event->event_data] = event;
  return;
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
hash_timed_event& hash_timed_event::_internal_copy(hash_timed_event const& right) {
  if (this != &right) {
    _hevent[service_check][low] = right._hevent[service_check][low];
    _hevent[service_check][high] = right._hevent[service_check][high];
    _hevent[host_check][low] = right._hevent[host_check][low];
    _hevent[host_check][high] = right._hevent[host_check][high];
  }
  return (*this);
}
