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

#ifndef TEST_OBJECTS_RELEASE_HH
#  define TEST_OBJECTS_RELEASE_HH

#  include <algorithm>
#  include <QList>
#  include "com/centreon/engine/error.hh"
#  include "com/centreon/engine/globals.hh"
#  include "com/centreon/engine/objects/command.hh"
#  include "com/centreon/engine/objects/commandsmember.hh"
#  include "com/centreon/engine/objects/contact.hh"
#  include "com/centreon/engine/objects/contactgroup.hh"
#  include "com/centreon/engine/objects/contactgroupsmember.hh"
#  include "com/centreon/engine/objects/contactsmember.hh"
#  include "com/centreon/engine/objects/customvariablesmember.hh"
#  include "com/centreon/engine/objects/daterange.hh"
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/hostdependency.hh"
#  include "com/centreon/engine/objects/hostescalation.hh"
#  include "com/centreon/engine/objects/hostgroup.hh"
#  include "com/centreon/engine/objects/hostsmember.hh"
#  include "com/centreon/engine/objects/objectlist.hh"
#  include "com/centreon/engine/objects/service.hh"
#  include "com/centreon/engine/objects/servicedependency.hh"
#  include "com/centreon/engine/objects/serviceescalation.hh"
#  include "com/centreon/engine/objects/servicegroup.hh"
#  include "com/centreon/engine/objects/servicesmember.hh"
#  include "com/centreon/engine/objects/timeperiod.hh"
#  include "com/centreon/engine/objects/timeperiodexclusion.hh"
#  include "com/centreon/engine/objects/timerange.hh"

namespace test {
  namespace objects {
    template<class T>
    void release_null_pointer(T const* obj) {
      using namespace com::centreon::engine::objects;

      if (obj != NULL)
        throw (engine_error() << "release null pointer faild obj is not null.");
      release(obj);
    }

    template<class T>
    void release_objects(T* (*create_object)(unsigned int),
                         T* head,
                         T* tail,
                         unsigned int number = 1) {
      using namespace com::centreon::engine::objects;

      init_object_skiplists();
      QList<T*> lst_obj;

      for (unsigned int i = 0; i < number; ++i) {
        T* obj = create_object(i + 1);
        if (obj == NULL)
          throw (engine_error() << Q_FUNC_INFO << " create object failed.");
        lst_obj.push_back(obj);
      }

      for (typename QList<T*>::const_iterator it = lst_obj.begin(),
             end = lst_obj.end();
           it != end;
           ++it)
        release(*it);

      if (head != NULL)
        throw (engine_error() << Q_FUNC_INFO << " failed head is not empty.");

      if (tail != NULL)
        throw (engine_error() << Q_FUNC_INFO << " failed tail is not empty.");

      free_object_skiplists();
    }


    template<class T>
    void release_objects(T* (*create_object)(unsigned int, T**),
                         unsigned int number = 1) {
      using namespace com::centreon::engine::objects;

      init_object_skiplists();
      QList<T*> lst_obj;
      T* head = NULL;

      for (unsigned int i = 0; i < number; ++i) {
        T* obj = create_object(i + 1, &head);
        if (obj == NULL)
          throw (engine_error() << Q_FUNC_INFO << " create object failed.");
        lst_obj.push_front(obj);
      }

      for (typename QList<T*>::const_iterator it = lst_obj.begin(),
             end = lst_obj.end();
           it != end;
           ++it) {
        if (it + 1 != lst_obj.end()) {
          if (release(*it) != *(it + 1))
            throw (engine_error() << Q_FUNC_INFO << " failed invalid return.");
        }
        else if (release(*it) != NULL)
          throw (engine_error() << Q_FUNC_INFO << " failed invalid return.");
      }

      free_object_skiplists();
    }

    template<class T>
    void release_objects(T* (*create_object)(unsigned int),
                         unsigned int number = 1) {
      using namespace com::centreon::engine::objects;

      init_object_skiplists();
      QList<T*> lst_obj;

      for (unsigned int i = 0; i < number; ++i) {
        T* obj = create_object(i + 1);
        if (obj == NULL)
          throw (engine_error() << Q_FUNC_INFO << " create object failed.");
        lst_obj.push_front(obj);
      }

      for (typename QList<T*>::const_iterator it = lst_obj.begin(),
             end = lst_obj.end();
           it != end;
           ++it)
        release(*it);

      free_object_skiplists();
    }
  }
}

#endif // !TEST_OBJECTS_RELEASE_HH
