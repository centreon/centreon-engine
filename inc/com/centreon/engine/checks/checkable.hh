/*
** Copyright 2017 Centreon
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

#ifndef CCE_CHECKS_CHECKABLE_HH
#  define CCE_CHECKS_CHECKABLE_HH

CCE_BEGIN()

namespace           checks {

  /**
   *  @class checkable checkable.hh "com/centreon/engine/checks/checkable.hh"
   *  @brief Object executing checks.
   *
   */
  class             checkable {
   public:
                    checkable();
                    checkable(checkable const& other);
                    ~checkable();
    checkable&      operator=(checkable const& other);
    bool            is_flapping();
    int             get_state();
    int             get_last_state();
    int             get_last_hard_state();
  };

}

CCE_END()

#endif // !CCE_CHECKS_CHECKABLE_HH
