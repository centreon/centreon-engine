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

#include <syslog.h>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/logging/syslog.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 */
logging::syslog::syslog() : _facility(LOG_USER) {
  openlog("centreon-engine", LOG_ODELAY, _facility);
}

/**
 *  Default destructor.
 */
logging::syslog::~syslog() throw () {
  closelog();
}

/**
 *  Get instance of syslog singleton.
 */
logging::syslog& logging::syslog::instance() {
  static logging::syslog instance;
  return (instance);
}

/**
 *  Write log into syslog.
 *
 *  @param[in] message   Message to log.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 */
void logging::syslog::log(
                        char const* message,
                        unsigned long long type,
                        unsigned int verbosity) throw () {
  (void)type;
  (void)verbosity;

  if (message) {
    concurrency::locker lock(&_mutex);
    ::syslog(_facility | LOG_INFO, "%s", message);
  }
  return ;
}

/**
 *  Set the syslog facility.
 *
 *  @param[in] facility Used to specify what type of
 *                      program is logging the message.
 */
void logging::syslog::set_facility(int facility) throw () {
  concurrency::locker lock(&_mutex);
  _facility = facility;
  closelog();
  openlog("centreon-engine", LOG_ODELAY, _facility);
  return ;
}
