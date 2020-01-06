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

#include "com/centreon/engine/timezone_manager.hh"
#include <cstdlib>
#include <ctime>

using namespace com::centreon::engine;

/**
 *  Restore timezone previously saved.
 */
void timezone_manager::pop_timezone() {
  // No more timezone available equals no change.
  if (!_tz.empty()) {
    // Timezone was set.
    if (_tz.top().is_set)
      _set_timezone(_tz.top().tz_name);
    // Timezone was not set.
    else
      _set_timezone("");
    _tz.pop();
  }
}

/**
 *  Save current timezone and set new one.
 *
 *  @param[in] tz  New timezone.
 */
void timezone_manager::push_timezone(std::string const& tz) {
  // Backup previous timezone.
  tz_info info;
  _backup_timezone(&info);
  _tz.push(info);

  // Set new timezone.
  if (!tz.empty())
    _set_timezone(tz);
  else if (_base.is_set)
    _set_timezone(_base.tz_name);
  else
    _set_timezone("");
}

/**
 *  Default constructor.
 */
timezone_manager::timezone_manager() {
  _backup_timezone(&_base);
}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
timezone_manager::timezone_manager(timezone_manager const& other)
    : _tz{other._tz} {}

/**
 *  Destructor.
 */
timezone_manager::~timezone_manager() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
timezone_manager& timezone_manager::operator=(timezone_manager const& other) {
  if (this != &other)
    _tz = other._tz;
  return *this;
}

/**
 *  Backup current timezone.
 *
 *  @param[out] info  Timezone information.
 */
void timezone_manager::_backup_timezone(timezone_manager::tz_info* info) {
  char* old_tz(getenv("TZ"));
  if (old_tz) {
    info->is_set = true;
    info->tz_name = old_tz;
  } else
    info->is_set = false;
}

/**
 *  Set new timezone.
 *
 *  @param[in] info  Timezone information.
 */
void timezone_manager::_set_timezone(std::string const& tz) {
  if (!tz.empty())
    setenv("TZ", tz.c_str(), 1);
  else
    unsetenv("TZ");
  tzset();
}
