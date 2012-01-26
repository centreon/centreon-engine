/*
** Copyright 2011 Merethis
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
# define CCE_COMMANDS_CONNECTOR_REQUEST_BUILDER_HH

# include <map>
# include <QSharedPointer>
# include <QByteArray>
# include "com/centreon/engine/commands/connector/request.hh"

namespace                           com {
  namespace                         centreon {
    namespace                       engine {
      namespace                     commands {
	namespace                   connector {
	/**
	 *  @class request_builder commands/connector/request_builder.hh
	 *  @brief Request builder is a request factory.
	 */
	  class                     request_builder {
	  public:
	    static request_builder& instance();

	    QSharedPointer<request> build(QByteArray const& data) const;

	  private:
	                            request_builder();
	                            request_builder(request_builder const& right);
	                            ~request_builder() throw();

	    request_builder&        operator=(request_builder const& right);

	    std::map<unsigned int, QSharedPointer<request> >
	                            _list;
	  };
	}
      }
    }
  }
}

#endif // !CCE_COMMANDS_CONNECTOR_REQUEST_BUILDER_HH
