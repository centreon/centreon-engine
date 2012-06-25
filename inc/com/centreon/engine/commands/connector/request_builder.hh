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

#ifndef CCE_COMMANDS_CONNECTOR_REQUEST_BUILDER_HH
#  define CCE_COMMANDS_CONNECTOR_REQUEST_BUILDER_HH

#  include <map>
#  include <map>
#  include "com/centreon/engine/commands/connector/request.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace                     commands {
  namespace                   connector {
    /**
     *  @class request_builder commands/connector/request_builder.hh
     *  @brief Request builder is a request factory.
     */
    class                     request_builder {
    public:
      com::centreon::shared_ptr<request>
                              build(std::string const& data) const;
      static request_builder& instance();

    private:
                              request_builder();
                              request_builder(
                                request_builder const& right);
                              ~request_builder() throw ();
      request_builder&        operator=(request_builder const& right);

      std::map<unsigned int, com::centreon::shared_ptr<request> >
                              _list;
    };
  }
}

CCE_END()

#endif // !CCE_COMMANDS_CONNECTOR_REQUEST_BUILDER_HH
