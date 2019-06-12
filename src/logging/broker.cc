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

#include <cstring>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/logging/broker.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/exceptions/basic.hh"
#include "com/centreon/unique_array_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine::logging;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
broker::broker()
  : backend(false, false, com::centreon::logging::none, false),
    _enable(false) {
  memset(&_thread, 0, sizeof(_thread));
  open();
}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
broker::broker(broker const& right)
  : backend(right),
    _enable(false) {
  operator=(right);
}

/**
 *  Destructor.
 */
broker::~broker() throw () {
  close();
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
broker& broker::operator=(broker const& right) {
  if (this != &right) {
    backend::operator=(right);
    concurrency::locker lock1(&_lock);
    concurrency::locker lock2(&right._lock);
    _thread = right._thread;
    _enable = right._enable;
  }
  return (*this);
}

/**
 *  Close broker log.
 */
void broker::close() throw () {
  concurrency::locker lock(&_lock);
  _enable = false;
}

/**
 *  Send message to broker.
 *
 *  @param[in] type     Logging types.
 *  @param[in] verbose  Verbosity level.
 *  @param[in] message  Message to log.
 *  @param[in] size     Message length.
 */
void broker::log(
               unsigned long long types,
               unsigned int verbose,
               char const* message,
               unsigned int size) throw () {
  (void)verbose;

  // Broker is only notified of non-debug log messages.
  if (message && _enable) {
    concurrency::locker lock(&_lock);
    if (_thread != concurrency::thread::get_current_id()) {
      _thread = concurrency::thread::get_current_id();

      // Copy message because broker module might modify it.
      unique_array_ptr<char> copy(new char[size + 1]);
      strcpy(copy.get(), message);

      // Event broker callback.
      broker_log_data(
        NEBTYPE_LOG_DATA,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        copy.get(),
        types,
        time(NULL),
        NULL);

      // Reset thread.
      memset(&_thread, 0, sizeof(_thread));
    }
  }
  return;
}

/**
 *  Open broker log.
 */
void broker::open() {
  concurrency::locker lock(&_lock);
  _enable = true;
}

/**
 *  Open borker log.
 */
void broker::reopen() {
  concurrency::locker lock(&_lock);
  _enable = true;
}

