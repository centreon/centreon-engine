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

#ifndef CCE_CONFIGURATION_APPLIER_OBJECT_HH
#  define CCE_CONFIGURATION_APPLIER_OBJECT_HH

#  include <cstring>
#  include <list>
#  include <string>
#  include <vector>
#  include "com/centreon/engine/configuration/applier/difference.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/shared.hh"
#  include "com/centreon/shared_ptr.hh"

#  define NULL_IF_EMPTY(str) ((str).empty() ? NULL : (str).c_str())

CCE_BEGIN()

namespace          configuration {
  namespace        applier {
    template<typename T>
    class          object {
    public:
      virtual      ~object() throw () {}

    protected:
      void         _diff(
                     std::list<shared_ptr<T> > const& old_objects,
                     std::list<shared_ptr<T> > const& new_objects) {
        difference<std::list<shared_ptr<T> > >
          diff(old_objects, new_objects);
        for (typename std::list<shared_ptr<T> >::const_iterator
               it(diff.added().begin()), end(diff.added().end());
             it != end;
             ++it)
          _add_object(*it);

        for (typename std::list<shared_ptr<T> >::const_iterator
               it(diff.modified().begin()), end(diff.modified().end());
             it != end;
             ++it)
          _modify_object(*it);

        for (typename std::list<shared_ptr<T> >::const_iterator
               it(diff.deleted().begin()), end(diff.deleted().end());
             it != end;
             ++it)
          _remove_object(*it);
      }

      virtual void _add_object(shared_ptr<T> obj) = 0;
      virtual void _modify_object(shared_ptr<T> obj) = 0;
      virtual void _remove_object(shared_ptr<T> obj) = 0;
    };

    template <typename T>
    void modify_if_different(T& t1, T t2) {
      if (t1 != t2)
	t1 = t2;
      return ;
    }

    void modify_if_different(char*& s1, char const* s2);

    void modify_if_different(
           char** t1,
           std::vector<std::string> const& t2,
           unsigned int size);

    template<typename T>
    void unregister_object(T** lst, T* ptr) {
      for (T** node(lst); *node; node = &(*node)->next)
        if (*node == ptr) {
          (*node) = (*node)->next;
          break ;
        }
    }
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_OBJECT_HH
