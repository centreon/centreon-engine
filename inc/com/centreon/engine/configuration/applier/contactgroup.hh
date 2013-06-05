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

#ifndef CCE_CONFIGURATION_APPLIER_CONTACTGROUP_HH
#  define CCE_CONFIGURATION_APPLIER_CONTACTGROUP_HH

#  include "com/centreon/engine/configuration/applier/base.hh"
#  include "com/centreon/engine/configuration/applier/object.hh"
#  include "com/centreon/engine/configuration/contactgroup.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  namespace                applier {
    class                  contactgroup
      : public base,
        public object<configuration::contactgroup> {
    public:
      void                 apply(state const& config);
      static contactgroup& instance();
      static void          load();
      static void          unload();

    private:
                           contactgroup();
                           contactgroup(contactgroup const&);
                           ~contactgroup() throw ();
      contactgroup&        operator=(contactgroup const&);
      void                 _add_object(contactgroup_ptr obj);
      void                 _modify_object(contactgroup_ptr obj);
      void                 _remove_object(contactgroup_ptr obj);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_CONTACTGROUP_HH
