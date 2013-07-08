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

#ifndef CCE_EVENTS_HASH_TIMED_EVENT_HH
#  define CCE_EVENTS_HASH_TIMED_EVENT_HH

#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/unordered_hash.hh"

// Forward declaration.
struct timed_event_struct;

CCE_BEGIN()

namespace               events {
  /**
   *  @class hash_timed_event hash_timed_event.hh
   *  @brief Allow to find quickly timed event.
   *
   *  This class allow to find timed event with
   *  it's data very quickly.
   */
  class                 hash_timed_event {
  public:
    enum                priority {
      low = 0,
      high = 1
    };
    enum                type {
      service_check = 0,
      host_check = 1
    };

                        hash_timed_event();
                        hash_timed_event(hash_timed_event const& right);
                        ~hash_timed_event();
    hash_timed_event&   operator=(hash_timed_event const& right);
    void                clear(priority p);
    void                clear(priority p, type t);
    void                erase(priority p, timed_event_struct* event);
    timed_event_struct* find(priority p, type t, void* ptr);
    void                insert(priority p, timed_event_struct* event);

  private:
    hash_timed_event&   _internal_copy(hash_timed_event const& right);

    umap<void*, timed_event_struct*>
                        _hevent[2][2];
  };
}

CCE_END()

#endif // !CCE_EVENTS_HASH_TIMED_EVENT_HH
