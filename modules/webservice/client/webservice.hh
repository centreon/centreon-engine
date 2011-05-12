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

#ifndef CCE_MOD_WS_CLIENT_WEBSERVICE_HH
# define CCE_MOD_WS_CLIENT_WEBSERVICE_HH

# include <QString>

# include "auto_gen.hh"
# include "soapH.h"

namespace               com {
  namespace             centreon {
    namespace           engine {
      namespace         modules {
	namespace       client {
	  /**
	   *  @class webservice webservice.hh
	   *  @brief Base webservice class.
	   *
	   *  Webservice class provide system to execute commands
	   *  over a network with a Simple Object Access Protocol.
	   */
	  class         webservice {
	  public:
	    webservice(bool ssl_enable = false,
		       QString const& keyfile = "",
		       QString const& password = "",
		       QString const& cacert = "");

	    webservice(webservice const& right);
	    ~webservice();

	    webservice& operator=(webservice const& right);

	    QString const& get_end_point() const throw();
	    QString const& get_action() const throw();
	    bool           is_ssl_enable() const throw();

	    void           set_end_point(QString const& end_point);
	    void           set_action(QString const& action);

	    bool        execute(QString const& function,
				QList<QString> const& args);

	  private:
#ifdef WITH_OPENSSL
	    static void _sigpipe_handle(int x);
#endif // !WITH_OPENSSL

	    QString     _end_point;
	    QString     _action;
	    soap        _soap_ctx;
	    bool        _ssl_enable;
	    auto_gen&   _gen;
	  };
	}
      }
    }
  }
}
#endif // !CCE_MOD_WS_CLIENT_WEBSERVICE_HH
