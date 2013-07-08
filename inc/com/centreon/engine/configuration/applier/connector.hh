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

#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace               configuration {
  // Forward declarations.
  class                 connector;
  class                 state;

  namespace             applier {
    class               connector {
    public:
                        connector();
                        connector(connector const& right);
                        ~connector() throw ();
      connector&        operator=(connector const& right);
      void              add_object(
                          shared_ptr<configuration::connector> obj);
      void              expand_object(
                          shared_ptr<configuration::connector> obj,
                          configuration::state& s);
      void              modify_object(
                          shared_ptr<configuration::connector> obj);
      void              remove_object(
                          shared_ptr<configuration::connector> obj);
      void              resolve_object(
                          shared_ptr<configuration::connector> obj);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_CONNECTOR_HH
