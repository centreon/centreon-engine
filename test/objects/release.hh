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

#ifndef TEST_OBJECTS_RELEASE_HH
# define TEST_OBJECTS_RELEASE_HH

# include <QList>
# include <algorithm>
# include "error.hh"
# include "globals.hh"
# include "objects/command.hh"
# include "objects/contact.hh"
# include "objects/contactgroup.hh"
# include "objects/host.hh"
# include "objects/hostdependency.hh"
# include "objects/hostescalation.hh"
# include "objects/hostgroup.hh"
# include "objects/service.hh"
# include "objects/servicedependency.hh"
# include "objects/serviceescalation.hh"
# include "objects/servicegroup.hh"
# include "objects/timeperiod.hh"
# include "objects/commandsmember.hh"
# include "objects/hostsmember.hh"
# include "objects/servicesmember.hh"
# include "objects/contactsmember.hh"
# include "objects/contactgroupsmember.hh"
# include "objects/customvariablesmember.hh"
# include "objects/timerange.hh"
# include "objects/daterange.hh"
# include "objects/timeperiodexclusion.hh"
# include "objects/objectlist.hh"

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
