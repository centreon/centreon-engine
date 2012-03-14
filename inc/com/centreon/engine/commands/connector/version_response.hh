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

#ifndef CCE_COMMANDS_CONNECTOR_VERSION_RESPONSE_HH
#  define CCE_COMMANDS_CONNECTOR_VERSION_RESPONSE_HH

#  include "com/centreon/engine/commands/connector/request.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace               commands {
  namespace             connector {
    /**
     *  @class version_response commands/connector/version_response.hh
     *  @brief Version response give the minimum version of engine
     *  supported by the connector.
     *
     *  Version response is the response request, who give the minimum
     *  version of engine supported by the connector.
     */
    class               version_response : public request {
    public:
                        version_response(
                          unsigned int major = 0,
                          unsigned int minor = 0);
                        version_response(version_response const& right);
                        ~version_response() throw ();
      version_response& operator=(version_response const& right);
      bool              operator==(version_response const& right) const throw ();
      bool              operator!=(version_response const& right) const throw ();
      QByteArray        build();
      request*          clone() const;
      unsigned int      get_major() const throw ();
      unsigned int      get_minor() const throw ();
      void              restore(QByteArray const& data);

    private:
      unsigned int      _major;
      unsigned int      _minor;
    };
  }
}

CCE_END()

#endif // !CCE_COMMANDS_CONNECTOR_VERSION_RESPONSE_HH
