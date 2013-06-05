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

#ifndef CCE_CONFIGURATION_APPLIER_HOSTESCALATION_HH
#  define CCE_CONFIGURATION_APPLIER_HOSTESCALATION_HH

#  include "com/centreon/engine/configuration/applier/base.hh"
#  include "com/centreon/engine/configuration/applier/object.hh"
#  include "com/centreon/engine/configuration/hostescalation.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                    configuration {
  namespace                  applier {
    class             hostescalation
      : public base,
        public object<configuration::hostescalation> {
    public:
      void                   apply(state const& config);
      static hostescalation& instance();
      static void            load();
      static void            unload();

    private:
                             hostescalation();
                             hostescalation(hostescalation const&);
                             ~hostescalation() throw ();
      hostescalation&        operator=(hostescalation const&);
      void                   _add_object(hostescalation_ptr obj);
      void                   _modify_object(hostescalation_ptr obj);
      void                   _remove_object(hostescalation_ptr obj);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_HOSTESCALATION_HH
