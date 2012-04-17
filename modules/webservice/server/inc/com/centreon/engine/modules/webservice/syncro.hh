/*
** Copyright 2011 Merethis
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

#ifndef CCE_MOD_WS_SYNCRO_HH
#  define CCE_MOD_WS_SYNCRO_HH

#  include <QWaitCondition>
#  include <QMutex>
#  include "com/centreon/engine/modules/webservice/namespace.hh"

CCE_MOD_WS_BEGIN()

/**
 *  @class syncro syncro.hh
 *  @brief Synchronize thread and callback.
 *
 *  Syncro class provide system to synchronize thread
 *  and callback processing.
 */
class            syncro {
public:
  static syncro& instance();
  void           waiting_callback();
  void           wakeup_worker();
  void           worker_finish();

private:
                 syncro();
                 syncro(syncro const& right);
                 ~syncro();
  syncro&        operator=(syncro const& right);
  bool           _thread_count_is_null() const;

  bool           _can_run;
  QWaitCondition _cnd_main;
  QWaitCondition _cnd_worker;
  QMutex         _mtx_main;
  mutable QMutex _mtx_worker;
  unsigned int   _thread_count;
};

CCE_MOD_WS_END()

#endif // !CCE_MOD_WS_SYNCRO_HH
