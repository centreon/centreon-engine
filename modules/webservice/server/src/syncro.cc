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
#include "com/centreon/engine/modules/webservice/syncro.hh"

using namespace com::centreon::engine::modules::webservice;

syncro& syncro::instance() {
  static syncro instance;
  return (instance);
}

void syncro::waiting_callback() {
  _mtx_worker.lock();
  ++_thread_count;
  _cnd_worker.wait(&_mtx_worker);
}

void syncro::wakeup_worker() {
  QMutexLocker lock(&_mtx_main);
  while (!_thread_count_is_null()) {
    {
      QMutexLocker tmp_lock(&_mtx_worker);
      _cnd_worker.wakeOne();
    }
    _cnd_main.wait(&_mtx_main);
  }
}

void syncro::worker_finish() {
  --_thread_count;
  _mtx_worker.unlock();
  QMutexLocker lock(&_mtx_main);
  _cnd_main.wakeOne();
}

syncro::syncro()
  : _thread_count(0) {

}

syncro::~syncro() {

}

bool syncro::_thread_count_is_null() const {
  QMutexLocker lock(&_mtx_worker);
  return (!_thread_count);
}
