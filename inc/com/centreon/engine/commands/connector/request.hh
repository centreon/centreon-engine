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

#ifndef CCE_COMMANDS_CONNECTOR_REQUEST_HH
#  define CCE_COMMANDS_CONNECTOR_REQUEST_HH

#  include <QByteArray>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                      commands {
  namespace                    connector {
    /**
     *  @class request commands/connector/request.hh
     *  @brief Request is an abstract object, have to implement
     *  to communicate between engine and a connector.
     *
     *  Request is an abstract object, have to implement
     *  to communicate between engine and a connector.
     *  You hav to implement clone, build and restore methods.
     */
    class                      request {
    public:
      enum                     e_type {
        version_q = 0,
        version_r,
        execute_q,
        execute_r,
        quit_q,
        quit_r,
        error_r
      };

                               request(e_type id);
                               request(request const& right);
      virtual                  ~request() throw ();
      request&                 operator=(request const& right);
      bool                     operator==(
                                 request const& right) const throw ();
      bool                     operator!=(
                                 request const& right) const throw ();
      virtual QByteArray       build() = 0;
      virtual request*         clone() const = 0;
      static QByteArray const& cmd_ending() throw ();
      virtual e_type           get_id() const throw ();
      virtual void             restore(QByteArray const& data) = 0;

    protected:
      e_type                   _id;

    private:
      void                     _internal_copy(request const& right);
    };
  }
}

CCE_END()

#endif // !CCE_COMMANDS_CONNECTOR_REQUEST_HH
