/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
** Copyright 2011-2012 Merethis
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

#ifndef CCE_EVENTS_LOOP_HH
#  define CCE_EVENTS_LOOP_HH

#  include <memory>
#  include <time.h>
#  include "com/centreon/engine/events.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace             events {
  /**
   *  @class loop loop.hh
   *  @brief Create Centreon Engine event loop on a new thread.
   *
   *  Events loop is a singleton to create a new thread
   *  and dispatch the Centreon Engine events.
   */
  class               loop {
  public:
                      ~loop() throw ();
    static loop&      instance();
    static void       load();
    void              run();
    static void       unload();

  private:
                      loop();
                      loop(loop const& right);
    loop&             operator=(loop const& right);
    void              _dispatching();
    void              _internal_copy(loop const& right);

    static std::auto_ptr<loop>
    _instance;
    time_t            _last_status_update;
    time_t            _last_time;
    timed_event       _sleep_event;
  };
}

CCE_END()

#endif // !CCE_EVENTS_LOOP_HH
