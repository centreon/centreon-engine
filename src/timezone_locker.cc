/*
** Copyright 2014 Merethis
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

#include "com/centreon/engine/timezone_locker.hh"
#include "com/centreon/engine/timezone_manager.hh"

using namespace com::centreon::engine;

/**
 *  Constructor.
 *
 *  @param[in] tz  Timezone to set during object lifetime.
 */
timezone_locker::timezone_locker(std::string const& tz) {
  timezone_manager::instance().push_timezone(tz);
}

/**
 *  Destructor.
 */
timezone_locker::~timezone_locker() {
  timezone_manager::instance().pop_timezone();
}
