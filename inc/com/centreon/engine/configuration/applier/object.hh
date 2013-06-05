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

#  include <list>
#  include "com/centreon/engine/configuration/applier/difference.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

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
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_OBJECT_HH
