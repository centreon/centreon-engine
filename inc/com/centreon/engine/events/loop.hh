/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
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

#ifndef CCE_EVENTS_LOOP_HH
#  define CCE_EVENTS_LOOP_HH

#  include <ctime>
#  include "com/centreon/engine/events/timed_event.hh"
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
    static loop&      instance();
    static void       load();
    void              run();
    static void       unload();

  private:
                      loop();
                      loop(loop const&);
                      ~loop() throw ();
    loop&             operator=(loop const&);
    void              _dispatching();

    time_t            _last_status_update;
    time_t            _last_time;
    unsigned int      _need_reload;

    bool              _reload_running;
    timed_event       _sleep_event;
  };
}

CCE_END()

#endif // !CCE_EVENTS_LOOP_HH
