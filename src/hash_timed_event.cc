/*
** Copyright 2011      Merethis
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

#include <stddef.h>
#include "hash_timed_event.hh"

using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
hash_timed_event::hash_timed_event() {

}

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
hash_timed_event::~hash_timed_event() {

}

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
 *  Remove timed event.
 *
 *  @param[in] event  The event to remove.
 */
void hash_timed_event::erase(priority p, timed_event* event) {
  if (!event)
    return;
  _hevent[p].erase(event->event_data);
}

/**
 *  Find an event with it's data.
 *
 *  @param[in] ptr  The event data.
 */
timed_event* hash_timed_event::find(priority p, void* ptr) {
  htable::iterator it(_hevent[p].find(ptr));
  if (it == _hevent[p].end())
    return (NULL);
  return (it->second);
}

/**
 *  Add timed event.
 *
 *  @param[in] event  The event to add.
 */
void hash_timed_event::insert(priority p, timed_event* event) {
  if (!event || !event->event_data)
    return;
  _hevent[p][event->event_data] = event;
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
    _hevent[0] = right._hevent[0];
    _hevent[1] = right._hevent[1];
  }
  return (*this);
}
