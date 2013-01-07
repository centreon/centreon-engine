/*
** Copyright 2011-2013 Merethis
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
#  include <list>
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

namespace    test {
  namespace  objects {
    template <class T>
    void     release_null_pointer(T const* obj) {
      using namespace com::centreon::engine::objects;

      // Release null pointer only.
      if (obj)
        throw (engine_error()
               << __func__ << " failed: pointer is not null");
      release(obj);

      return ;
    }

    template <class T>
    void     release_objects(
               T* (*create_object)(unsigned int),
               T* head,
               T* tail,
               unsigned int number = 1) {
      using namespace com::centreon::engine::objects;

      // Initialization.
      init_object_skiplists();
      std::list<T*> lst_obj;

      // Create objects.
      for (unsigned int i(0); i < number; ++i) {
        T* obj(create_object(i + 1));
        if (!obj)
          throw (engine_error()
                 << __func__ << " failed: cannot create new object");
        lst_obj.push_back(obj);
      }

      // Release objects.
      for (typename std::list<T*>::const_iterator
             it(lst_obj.begin()),
             end(lst_obj.end());
           it != end;
           ++it)
        release(*it);

      // Check that release() successfully executed.
      if (head)
        throw (engine_error()
               << __func__ << " failed: list head is not empty");
      if (tail)
        throw (engine_error()
               << __func__ << " failed: list tail is not empty");

      // Free skiplist.
      free_object_skiplists();

      return ;
    }


    template <class T>
    void     release_objects(
               T* (*create_object)(unsigned int, T**),
               unsigned int number = 1) {
      using namespace com::centreon::engine::objects;

      // Initialization.
      init_object_skiplists();
      std::list<T*> lst_obj;
      T* head(NULL);

      // Create objects.
      for (unsigned int i(0); i < number; ++i) {
        T* obj(create_object(i + 1, &head));
        if (!obj)
          throw (engine_error()
                 << __func__ << " failed: cannot create new object");
        lst_obj.push_front(obj);
      }

      // Release objects.
      for (typename std::list<T*>::const_iterator
             it(lst_obj.begin()),
             end(lst_obj.end());
           it != end;
           ++it) {
        typename std::list<T*>::const_iterator tmp(it);
        ++tmp;
        if (tmp != lst_obj.end()) {
          if (release(*it) != *tmp)
            throw (engine_error() << __func__
                   << " failed: invalid release() return value");
        }
        else if (release(*it) != NULL)
          throw (engine_error() << __func__
                 << " failed: invalid release() return value");
      }

      // Free skiplsit.
      free_object_skiplists();

      return ;
    }

    template <class T>
    void     release_objects(
               T* (*create_object)(unsigned int),
               unsigned int number = 1) {
      using namespace com::centreon::engine::objects;

      // Initialization.
      init_object_skiplists();
      std::list<T*> lst_obj;

      // Create objects.
      for (unsigned int i(0); i < number; ++i) {
        T* obj(create_object(i + 1));
        if (!obj)
          throw (engine_error()
                 << __func__ << " failed: cannot create new object");
        lst_obj.push_front(obj);
      }

      // Release objects.
      for (typename std::list<T*>::const_iterator
             it(lst_obj.begin()),
             end(lst_obj.end());
           it != end;
           ++it)
        release(*it);

      // Free skiplist.
      free_object_skiplists();
    }
  }
}

#endif // !TEST_OBJECTS_RELEASE_HH
