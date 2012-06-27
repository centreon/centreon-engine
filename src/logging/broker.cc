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

#include <cstring>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/logging/broker.hh"
#include "com/centreon/engine/logging/object.hh"
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
broker::broker() : _thread(NULL) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
broker::broker(broker const& right) : object(right) {
  concurrency::locker lock(&right._mutex);
  _thread = right._thread;
}

/**
 *  Destructor.
 */
broker::~broker() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
broker& broker::operator=(broker const& right) {
  if (this != &right) {
    concurrency::locker lock1(&_mutex);
    concurrency::locker lock2(&right._mutex);
    _thread = right._thread;
  }
  return (*this);
}

/**
 *  Send message to broker.
 *
 *  @param[in] message   Message to log.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Unused.
 */
void broker::log(
               char const* message,
               unsigned long long type,
               unsigned int verbosity) throw () {
  (void)verbosity;

  // Broker is only notified of non-debug log messages.
  if (message && (type & dbg_all) == 0) {
    concurrency::locker lock(&_mutex);
    if (_thread != QThread::currentThread()) {
      _thread = QThread::currentThread();

      // Copy message because broker module might modify it.
      unique_array_ptr<char> copy(new char[strlen(message) + 1]);
      strcpy(copy.get(), message);

      // Event broker callback.
      broker_log_data(
        NEBTYPE_LOG_DATA,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        copy.get(),
        static_cast<unsigned long>(type),
        time(NULL),
        NULL);

      _thread = NULL;
    }
  }
  return ;
}
