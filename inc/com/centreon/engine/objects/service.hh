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

#ifndef CCE_OBJECTS_SERVICE_HH
#  define CCE_OBJECTS_SERVICE_HH

#  include "com/centreon/engine/objects.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

bool link_service(
       service* obj,
       contact** contacts,
       contactgroup** contactgroups,
       servicegroup** servicegroups,
       char** custom_variables,
       int initial_state,
       timeperiod* check_period,
       timeperiod* notification_period,
       command* cmd_event_handler,
       command* cmd_check_command);
void release_service(service const* obj);

#  ifdef __cplusplus
}

namespace       com {
  namespace     centreon {
    namespace   engine {
      namespace objects {
        void    link(
                  service* obj,
                  QVector<contact*> const& contacts,
                  QVector<contactgroup*> const& contactgroups,
                  QVector<servicegroup*> const& servicegroups,
                  QVector<QString> const& custom_variables,
                  int initial_state,
                  timeperiod* check_period,
                  timeperiod* notification_period,
                  command* cmd_event_handler = NULL,
                  command* cmd_check_command = NULL);
        void    release(service const* obj);
      }
    }
  }
}
#  endif // C++

#endif // !CCE_OBJECTS_SERVICE_HH
