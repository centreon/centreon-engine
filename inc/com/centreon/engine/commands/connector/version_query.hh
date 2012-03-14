/*
** Copyright 2011-2012 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version_query 2
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

#ifndef CCE_COMMANDS_CONNECTOR_VERSION_QUERY_HH
#  define CCE_COMMANDS_CONNECTOR_VERSION_QUERY_HH

#  include "com/centreon/engine/commands/connector/request.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace            commands {
  namespace          connector {
    /**
     *  @class version_query commands/connector/version_query.hh
     *  @brief Version query ask the minimum version of engine
     *  supported by the connector.
     *
     *  Version query is a request, who ask the minimum version of
     *  engine supported by the connector.
     */
    class            version_query : public request {
    public:
                     version_query();
                     version_query(version_query const& right);
                     ~version_query() throw ();
      version_query& operator=(version_query const& right);
      bool           operator==(
                       version_query const& right) const throw ();
      bool           operator!=(
                       version_query const& right) const throw ();
      QByteArray     build();
      request*       clone() const;
      void           restore(QByteArray const& data);
    };
  }
}

CCE_END()

#endif // !CCE_COMMANDS_CONNECTOR_VERSION_QUERY_HH
