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

#ifndef CCE_CONFIGURATION_RELOAD_HH
#  define CCE_CONFIGURATION_RELOAD_HH

#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/concurrency/thread.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace   configuration {
  /**
   *  @class reload reload.hh "com/centreon/engine/configuration/reload.hh"
   *  @brief Reload a configuration state.
   *
   *  This class is used to reload a configuration state in a separate
   *  thread which reduce the time required to load the configuration on
   *  a multiprocessor machine.
   */
  class     reload : private concurrency::thread {
  public:
            reload();
            ~reload() throw ();
    bool    is_finished() const;
    void    start();
    void    try_lock();
    void    wait();

  private:
            reload(reload const&);
    reload& operator=(reload const&);
    void    _run();
    void    _set_is_finished(bool value);

    bool    _is_finished;
    mutable concurrency::mutex
            _lock;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_RELOAD_HH


