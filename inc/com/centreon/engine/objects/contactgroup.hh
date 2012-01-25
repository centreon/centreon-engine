/*
** Copyright 2011 Merethis
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

#ifndef CCE_OBJECTS_CONTACTGROUP_HH
# define CCE_OBJECTS_CONTACTGROUP_HH

# ifdef __cplusplus
#  include <QVector>
#  include <string>
# endif
# include "objects.hh"

# ifdef __cplusplus
extern "C" {
# endif

  bool link_contactgroup(contactgroup* obj,
                         contact** members,
                         contactgroup** groups);
  void release_contactgroup(contactgroup const* obj);

# ifdef __cplusplus
}

namespace       com {
  namespace     centreon {
    namespace   engine {
      namespace objects {
        void    link(contactgroup* obj,
                     QVector<contact*> const& members,
                     QVector<contactgroup*> const& groups);
        void    release(contactgroup const* obj);
        bool    add_contactgroups_to_object(QVector<contactgroup*> const& contactgroups,
                                            contactgroupsmember** list_contactgroup);
      }
    }
  }
}
# endif

#endif // !CCE_OBJECTS_CONTACTGROUP_HH
