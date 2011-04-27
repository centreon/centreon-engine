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

#include <syslog.h>
#include "logging/syslog.hh"

using namespace com::centreon::engine::logging;

syslog::syslog() : _facility(LOG_USER) {
  openlog("centreon-engine", LOG_ODELAY, _facility);
}

syslog::~syslog() throw() {
  closelog();
}

com::centreon::engine::logging::syslog& syslog::instance() {
  static syslog instance;
  return (instance);
}

void syslog::set_facility(int facility) throw() {
  _facility = facility;
  closelog();
  openlog("centreon-engine", LOG_ODELAY, _facility);
}

void syslog::log(char const* message,
		 unsigned long long type,
		 unsigned int verbosity) throw() {
  (void)type;
  (void)verbosity;

  ::syslog(_facility | LOG_INFO, "%s", message);
}
