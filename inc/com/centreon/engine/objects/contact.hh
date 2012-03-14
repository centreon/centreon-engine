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

#ifndef CCE_OBJECTS_CONTACT_HH
#  define CCE_OBJECTS_CONTACT_HH

#  include "com/centreon/engine/objects.hh"

#  ifdef __cplusplus
#    include <QString>
#    include <QVector>
extern "C" {
#  endif // C++

bool link_contact(
       contact* obj,
       timeperiod* host_notification_period,
       timeperiod* service_notification_period,
       contactgroup** contactgroups,
       command** host_notification_commands,
       command** service_notification_commands,
       char** custom_variables);
void release_contact(contact const* obj);

#  ifdef __cplusplus
}

namespace       com {
  namespace     centreon {
    namespace   engine {
      namespace objects {
        bool    add_contacts_to_object(
                  QVector<contact*> const& contacts,
                  contactsmember** list_contact);
        void    link(
                  contact* obj,
                  timeperiod* host_notification_period,
                  timeperiod* service_notification_period,
                  QVector<contactgroup*> const& contactgroups,
                  QVector<command*> const& host_notification_commands,
                  QVector<command*> const& service_notification_commands,
                  QVector<QString> const& custom_variables);
        void    release(contact const* obj);
      }
    }
  }
}
#  endif // C++

#endif // !CCE_OBJECTS_CONTACT_HH
