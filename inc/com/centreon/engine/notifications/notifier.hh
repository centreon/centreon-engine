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

#ifndef CCE_NOTIFICATIONS_NOTIFIER_HH
#  define CCE_NOTIFICATIONS_NOTIFIER_HH

#  include "com/centreon/engine/checks/checkable"

CCE_BEGIN()

namespace           notifications {

  /**
   *  @class notifier notifier.hh "com/centreon/engine/notifications/notifier.hh"
   *  @brief Object validating notifications and sending them if needed.
   *
   */
  class             notifier : public checkable {
   public:
    enum            notification_type {
                    PROBLEM,
                    RECOVERY,
                    ACKNOWLEDGEMENT,
                    FLAPPINGSTART,
                    FLAPPINGSTOP,
                    FLAPPINGDISABLED,
                    DOWNTIMESTART,
                    DOWNTIMESTOP,
                    DOWNTIMECANCELLED
    }
                    notifier();
                    notifier(notifier const& other);
                    ~notifier();
    notifier&       operator=(notifier const& other);
    bool            in_downtime();
    bool            notifications_enabled();
    void            notify(notification_type type);
  };
}

CCE_END()

#endif // !CCE_NOTIFICATIONS_NOTIFIER_HH
