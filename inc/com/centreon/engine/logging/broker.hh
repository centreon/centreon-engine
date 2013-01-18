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

#  include "com/centreon/concurrency/thread.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/logging/backend.hh"

CCE_BEGIN()

namespace                      logging {
  /**
   *  @class broker broker.hh
   *  @brief Call broker for all logging message.
   *
   *  Call broker for all logging message without debug.
   */
  class                        broker
    : public com::centreon::logging::backend {
  public:
                               broker();
                               broker(broker const& right);
                               ~broker() throw ();
    broker&                    operator=(broker const& right);
    void                       close() throw ();
    void                       log(
                                 unsigned long long types,
                                 unsigned int verbose,
                                 char const* msg,
                                 unsigned int size) throw ();
    void                        open();
    void                        reopen();
    void                        show_pid(bool enable);
    void                        show_timestamp(com::centreon::logging::time_precision val);
    void                        show_thread_id(bool enable);

  private:
    bool                       _enable;
    concurrency::thread_id     _thread;
  };
}

CCE_END()

#endif // !CCE_LOGGING_BROKER_HH
