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

#ifndef CCE_OBJECTS_HOST_HH
#  define CCE_OBJECTS_HOST_HH

#  include "com/centreon/engine/objects.hh"

#  ifdef __cplusplus
#    include <QString>
#    include <QVector>
extern "C" {
#  endif // C++

bool link_host(
       host* obj,
       host** parents,
       contact** contacts,
       contactgroup** contactgroups,
       hostgroup** hostgroups,
       char** custom_variables,
       int initial_state,
       timeperiod* check_period,
       timeperiod* notification_period,
       command* cmd_event_handler,
       command* cmd_check_command);
void release_host(host const* obj);

#  ifdef __cplusplus
}

namespace       com {
  namespace     centreon {
    namespace   engine {
      namespace objects {
        bool    add_hosts_to_object(
                  QVector<host*> const& hosts,
                  hostsmember** list_host);
        void    link(
                  host* obj,
                  QVector<host*> const& parents,
                  QVector<contact*> const& contacts,
                  QVector<contactgroup*> const& contactgroups,
                  QVector<hostgroup*> const& hostgroups,
                  QVector<QString> const& custom_variables,
                  int initial_state,
                  timeperiod* check_period,
                  timeperiod* notification_period,
                  command* cmd_event_handler = NULL,
                  command* cmd_check_command = NULL);
        void    release(host const* obj);
      }
    }
  }
}
#  endif // C++

#endif // !CCE_OBJECTS_HOST_HH
