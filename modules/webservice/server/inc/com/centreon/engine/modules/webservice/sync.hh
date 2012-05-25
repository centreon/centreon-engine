/*
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

#ifndef CCE_MOD_WS_SYNC_HH
#  define CCE_MOD_WS_SYNC_HH

#  include <QMutex>
#  include <QWaitCondition>
#  include "com/centreon/engine/modules/webservice/namespace.hh"

CCE_MOD_WS_BEGIN()

/**
 *  @class sync sync.hh "com/centreon/engine/modules/webservice/sync.hh"
 *  @brief Synchronize thread and callback.
 *
 *  The sync class provide system to synchronize thread and callback
 *  processing.
 */
class             sync {
public:
  static sync&    instance();
  void            wait_thread_safeness();
  void            wakeup_workers();
  void            worker_finish();

private:
                  sync();
                  sync(sync const& right);
                  ~sync();
  sync&           operator=(sync const& right);

  QWaitCondition  _cnd_main;
  QWaitCondition  _cnd_worker;
  QMutex          _mtx_main;
  QMutex          _mtx_worker;
  unsigned int    _thread_count;
};

CCE_MOD_WS_END()

#endif // !CCE_MOD_WS_SYNC_HH
