/*
** Copyright 2011-2013,2017 Centreon
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

#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace               configuration {
  // Forward declarations.
  class                 connector;
  class                 state;

  namespace             applier {
    class               connector {
     public:
                        connector();
                        ~connector() throw ();
      void              add_object(
                          configuration::connector const& obj);
      void              expand_objects(configuration::state& s);
      void              modify_object(
                          configuration::connector const& obj);
      void              remove_object(
                          configuration::connector const& obj);
      void              resolve_object(
                          configuration::connector const& obj);
     private:
                        connector(connector const& right);
      connector&        operator=(connector const& right);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_CONNECTOR_HH
