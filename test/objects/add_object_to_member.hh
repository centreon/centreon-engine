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

#ifndef TEST_OBJECTS_ADD_OBJECT_TO_MEMBER_HH
#  define TEST_OBJECTS_ADD_OBJECT_TO_MEMBER_HH

#  include <QVector>
#  include "com/centreon/engine/error.hh"
#  include "test/objects/release.hh"

namespace test{
  namespace objects {
    template<class T, class U>
    void add_with_null_member(bool (*add)(QVector<T*> const&, U**)) {
      QVector<T*> objs;
      if (add(objs, NULL) == true)
        throw (engine_error() << Q_FUNC_INFO << " invalid return.");
    }

    template<class T, class U>
    void add_without_objects(bool (*add)(QVector<T*> const&, U**)) {
      U* head = NULL;
      QVector<T*> objs;
      if (add(objs, &head) == false)
        throw (engine_error() << Q_FUNC_INFO << " invalid return.");
    }

    template<class T, class U>
    void add_with_objects(bool (*add)(QVector<T*> const&, U**),
                          T* (create)(unsigned int),
                          unsigned int id) {
      init_object_skiplists();
      U* head = NULL;
      QVector<T*> objs;
      for (unsigned int i = 0; i < id; ++i)
        objs.push_back(create(i + 1));

      if (add(objs, &head) == false)
        throw (engine_error() << Q_FUNC_INFO << " invalid return.");
      for (typename QVector<T*>::const_iterator it = objs.begin(),
             end = objs.end();
           it != end;
           ++it)
        release(*it);

      U const* member = head;
      while ((member = release(member)));
      free_object_skiplists();
    }
  }
}

#endif // !TEST_OBJECTS_ADD_OBJECT_TO_MEMBER_HH
