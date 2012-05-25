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

#include <QMutexLocker>
#include "com/centreon/engine/modules/webservice/sync.hh"

using namespace com::centreon::engine::modules::webservice;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Get class instance.
 *
 *  @return Class instance.
 */
sync& sync::instance() {
  static sync instance;
  return (instance);
}

/**
 *  Wait for global thread safeness.
 */
void sync::wait_thread_safeness() {
  QMutexLocker lock(&_mtx_worker);
  ++_thread_count;
  _cnd_worker.wait(&_mtx_worker);
  return ;
}

/**
 *  @brief Wake up a single worker thread.
 *
 *  This method will not return before all workers have finished
 *  executing.
 */
void sync::wakeup_workers() {
  QMutexLocker main_lock(&_mtx_main);
  QMutexLocker worker_lock(&_mtx_worker);
  while (_thread_count) {
    _cnd_worker.wakeOne();
    worker_lock.unlock();
    _cnd_main.wait(&_mtx_main);
    worker_lock.relock();
  }
  return ;
}

/**
 *  This method needs to be called by worker thread when they stop their
 *  processing to release other waiting threads.
 *
 *  @see sync_locker
 */
void sync::worker_finish() {
  --_thread_count;
  QMutexLocker lock(&_mtx_main);
  _cnd_main.wakeAll();
  return ;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
sync::sync() : _thread_count(0) {}

/**
 *  Destructor,
 */
sync::~sync() {}
