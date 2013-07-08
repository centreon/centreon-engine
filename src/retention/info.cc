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

#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/retention/info.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;

/**
 *  Constructor.
 */
retention::info::info()
  : object(object::info),
    _scheduling_info_is_ok(false) {

}

/**
 *  Destructor.
 */
retention::info::~info() throw () {

}

/**
 *  Get the scheduling state.
 *
 *  @return True if the scheduling is ok.
 */
bool retention::info::scheduling_info_is_ok() {
  return (_scheduling_info_is_ok);
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::info::set(
       std::string const& key,
       std::string const& value) {
  if (key == "created") {
    time_t current_time(time(NULL));
    time_t creation_time;
    string::to(value, creation_time);
    if ((current_time - creation_time)
        < static_cast<time_t>(config->retention_scheduling_horizon()))
      _scheduling_info_is_ok = true;
    else
      _scheduling_info_is_ok = false;
  }
  // else if (key == "version")
  //   ;
  // else if (key == "update_available")
  //   ;
  // else if (key == "update_uid")
  //   ;
  // else if (key == "last_version")
  //   ;
  // else if (key == "new_version")
  //   ;
  return (true);
}
