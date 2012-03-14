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

#ifndef CCE_COMMANDS_CONNECTOR_QUIT_QUERY_HH
#  define CCE_COMMANDS_CONNECTOR_QUIT_QUERY_HH

#  include "com/centreon/engine/commands/connector/request.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace         commands {
  namespace       connector {
    /**
     *  @class quit_query commands/connector/.hh
     *  @brief Quit query ask connector to exit.
     */
    class         quit_query : public request {
    public:
                  quit_query();
                  quit_query(quit_query const& right);
                  ~quit_query() throw ();
      quit_query& operator=(quit_query const& right);
      bool        operator==(quit_query const& right) const throw();
      bool        operator!=(quit_query const& right) const throw();
      QByteArray  build();
      request*    clone() const;
      void        restore(QByteArray const& data);
    };
  }
}

CCE_END()

#endif // !CCE_COMMANDS_CONNECTOR_QUIT_QUERY_HH
