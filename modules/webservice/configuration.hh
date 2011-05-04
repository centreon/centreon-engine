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

#ifndef CCE_MOD_WS_CONFIGURATION_HH
# define CCE_MOD_WS_CONFIGURATION_HH

# include <QXmlStreamReader>
# include <QString>
# include <QHash>

namespace                  com {
  namespace                centreon {
    namespace              engine {
      namespace            modules {
      /**
       *  @class configuration configuration.hh
       *  @brief Base configuration class.
       *
       *  Simple configuration class to parse
       *  configuration file.
       */
	class              configuration {
	public:
	                   configuration(QString const& filename = "");
	                   ~configuration() throw();

	  void             set_filename(QString const& filename);
	  QString const&   get_filename() const throw();

	  QString const&   get_host() const throw();
	  int              get_port() const throw();
	  int              get_recv_timeout() const throw();
	  int              get_send_timeout() const throw();
	  int              get_accept_timeout() const throw();
	  bool             get_ssl_enable() const throw();
	  QString const&   get_ssl_keyfile() const throw();
	  QString const&   get_ssl_cacert() const throw();
	  QString const&   get_ssl_dh() const throw();
	  QString const&   get_ssl_password() const throw();

	  void             parse();

	private:
	                   configuration(configuration const& right);
	  configuration&   operator=(configuration const& right);

	  void             _set_host();
	  void             _set_port();
	  void             _set_recv_timeout();
	  void             _set_send_timeout();
	  void             _set_accept_timeout();
	  void             _set_ssl_enable();
	  void             _set_ssl_keyfile();
	  void             _set_ssl_cacert();
	  void             _set_ssl_dh();
	  void             _set_ssl_password();

	  QHash<QString, void (configuration::*)()> _keytab;
	  QXmlStreamReader _reader;
	  QString          _filename;
	  QString          _path;
	  QString          _host;
	  QString          _ssl_keyfile;
	  QString          _ssl_cacert;
	  QString          _ssl_dh;
	  QString          _ssl_password;
	  int              _port;
	  bool             _ssl_enable;
	  int              _recv_timeout;
	  int              _send_timeout;
	  int              _accept_timeout;
	};
      }
    }
  }
}

#endif // !CCE_MOD_WS_CONFIGURATION_HH
