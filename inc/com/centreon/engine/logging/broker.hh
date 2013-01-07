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

#ifndef CCE_LOGGING_BROKER_HH
#  define CCE_LOGGING_BROKER_HH

#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/concurrency/thread.hh"
#  include "com/centreon/engine/logging/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace   logging {
  /**
   *  @class broker broker.hh
   *  @brief Call broker for all logging message.
   *
   *  Call broker for all logging message without debug.
   */
  class      broker : public object {
  public:
             broker();
             broker(broker const& right);
             ~broker() throw ();
    broker&  operator=(broker const& right);
    void     log(
               char const* message,
               unsigned long long type,
               unsigned int verbosity) throw ();

  private:
    mutable com::centreon::concurrency::mutex
             _mutex;
    com::centreon::concurrency::thread_id
             _thread;
  };
}

CCE_END()

#endif // !CCE_LOGGING_BROKER_HH
