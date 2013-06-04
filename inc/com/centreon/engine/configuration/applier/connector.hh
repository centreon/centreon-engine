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

#ifndef CCE_CONFIGURATION_APPLIER_CONNECTOR_HH
#  define CCE_CONFIGURATION_APPLIER_CONNECTOR_HH

#  include "com/centreon/engine/configuration/applier/base.hh"
#  include "com/centreon/engine/configuration/connector.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace               configuration {
  namespace             applier {
    class               connector
      : public base {
    public:
      void              apply(state const& config);
      static connector& instance();
      static void       load();
      static void       unload();

    private:
                        connector();
                        connector(connector const&);
                        ~connector() throw ();
      connector&        operator=(connector const&);
      void              _add_connectors(list_connector const& data);
      void              _modify_connectors(list_connector const& data);
      void              _remove_connectors(list_connector const& data);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_CONNECTOR_HH
