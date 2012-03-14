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

#ifndef CCE_COMMANDS_CONNECTOR_ERROR_RESPONSE_HH
#  define CCE_COMMANDS_CONNECTOR_ERROR_RESPONSE_HH

#  include "com/centreon/engine/commands/connector/request.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace               commands {
  namespace             connector {
    /**
     *  @class error_response commands/connector/error_response.hh
     *
     *  Error response is the response request, who give the information
     *  on error of connector.
     */
    class               error_response : public request {
    public:
      enum              e_code {
        info = 0,
        warning = 1,
        error = 2
      };

                        error_response(
                          QString const& message = "",
                          e_code code = info);
                        error_response(error_response const& right);
                        ~error_response() throw ();
      error_response&   operator=(error_response const& right);
      bool              operator==(
                          error_response const& right) const throw ();
      bool              operator!=(
                          error_response const& right) const throw ();
      QByteArray        build();
      request*          clone() const;
      e_code            get_code() const throw ();
      QString const&    get_message() const throw ();
      void              restore(QByteArray const& data);

    private:
      void              _internal_copy(error_response const& right);

      e_code            _code;
      QString           _message;
    };
  }
}

CCE_END()

#endif // !CCE_COMMANDS_CONNECTOR_ERROR_RESPONSE_HH
