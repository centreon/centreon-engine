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

#ifndef CCE_TIMEZONE_MANAGER_HH
#define CCE_TIMEZONE_MANAGER_HH

#include <stack>
#include <string>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

/**
 *  @class timezone_manager timezone_manager.hh
 * "com/centreon/engine/timezone_manager.hh"
 *  @brief Manage timezone changes.
 *
 *  This class handle timezone change. This can either be setting a new
 *  timezone or restoring a previous one.
 */
class timezone_manager {
 public:
  void pop_timezone();
  void push_timezone(std::string const& tz);

  /**
   *  Get class instance.
   *
   *  @return Class instance.
   */
  static timezone_manager& instance() { static timezone_manager instance; return instance; }

 private:
  struct tz_info {
    bool is_set;
    std::string tz_name;
  };

  timezone_manager();
  timezone_manager(timezone_manager const& other);
  ~timezone_manager();
  timezone_manager& operator=(timezone_manager const& other);
  void _backup_timezone(tz_info* info);
  void _set_timezone(std::string const& tz);

  tz_info _base;
  std::stack<tz_info> _tz;
};

CCE_END()

#endif  // !CCE_TIMEZONE_MANAGER_HH
