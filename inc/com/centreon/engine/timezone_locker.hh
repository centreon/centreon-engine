/*
** Copyright 2014-2019 Centreon
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

#ifndef CCE_TIMEZONE_LOCKER_HH
#  define CCE_TIMEZONE_LOCKER_HH

#  include <string>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

/**
 *  @class timezone_locker timezone_locker.hh "com/centreon/engine/timezone_locker.hh"
 *  @brief Handle timezone changes, even in case of exception.
 *
 *  This class works on a timezone_manager to push a new timezone at
 *  construction and pop it when destructed.
 */
class                 timezone_locker {
public:
                      timezone_locker(std::string const& tz);
                      ~timezone_locker();
                      timezone_locker(timezone_locker const& other) = delete;
  timezone_locker&    operator=(timezone_locker const& other) = delete;
};

CCE_END()

#endif // !CCE_TIMEZONE_LOCKER_HH
