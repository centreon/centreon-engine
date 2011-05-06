/*
** Copyright 2011      Merethis
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

#include <string.h>

#include "broker.hh"
#include "logging/object.hh"
#include "logging/broker.hh"

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
  : _mutex(QMutex::Recursive), _thread(NULL) {

}

/**
 *  Default destructor.
 */
broker::~broker() throw() {

}

/**
 *  Send message to broker.
 *
 *  @param[in] message   Message to log.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Unused.
 */
void broker::log(char const* message,
	  unsigned long long type,
	  unsigned int verbosity) throw() {
  (void)verbosity;

  if (message != NULL && (type & object::dbg_all) == 0) {
    _mutex.lock();
    if (_thread != QThread::currentThread()) {
      _thread = QThread::currentThread();
      char* copy = new char[strlen(message) + 1];
      strcpy(copy, message);

      broker_log_data(NEBTYPE_LOG_DATA,
		      NEBFLAG_NONE,
		      NEBATTR_NONE,
		      copy,
		      type,
		      time(NULL),
		      NULL);

      delete[] copy;
      _thread = NULL;
    }
    _mutex.lock();
  }
}
