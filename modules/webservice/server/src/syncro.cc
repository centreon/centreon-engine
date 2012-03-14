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

#include "com/centreon/engine/webservice/server/syncro.hh"

using namespace com::centreon::engine::modules;

syncro& syncro::instance() {
  static syncro instance;
  return (instance);
}

syncro::syncro()
  : _thread_count(0), _can_run(false) {

}

syncro::~syncro() {

}

void syncro::wakeup_worker() {
  _mutex.lock();
  _can_run = true;
  _condition.wakeAll();
  while (_thread_count > 0) {
    _condition.wait(&_mutex);
  }
  _can_run = false;
  _mutex.unlock();
}

void syncro::waiting_callback() {
  _mutex.lock();
  ++_thread_count;
  while (_can_run == false) {
    _condition.wait(&_mutex);
  }
  _mutex.unlock();
}

void syncro::worker_finish() {
  _mutex.lock();
  --_thread_count;
  _mutex.unlock();

  if (_thread_count == 0) {
    _condition.wakeAll();
  }
}
