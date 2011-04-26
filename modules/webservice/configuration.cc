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

#include <QFile>
#include "configuration.hh"
#include "error.hh"

using namespace com::centreon::engine::modules;

/**
 *  Default Constructor.
 *
 *  @param[in] filename The configuration filename.
 */
configuration::configuration(QString const& filename)
  : _filename(filename),
    _port(80),
    _ssl_enable(false),
    _recv_timeout(5),
    _send_timeout(5),
    _accept_timeout(500) {
  _keytab["/webservice"] = NULL;
  _keytab["/webservice/host"] = &configuration::_set_host;
  _keytab["/webservice/port"] = &configuration::_set_port;
  _keytab["/webservice/recv_timeout"] = &configuration::_set_recv_timeout;
  _keytab["/webservice/send_timeout"] = &configuration::_set_send_timeout;
  _keytab["/webservice/accept_timeout"] = &configuration::_set_accept_timeout;
  _keytab["/webservice/ssl"] = NULL;
  _keytab["/webservice/ssl/enable"] = &configuration::_set_ssl_enable;
  _keytab["/webservice/ssl/keyfile"] = &configuration::_set_ssl_keyfile;
  _keytab["/webservice/ssl/cacert"] = &configuration::_set_ssl_cacert;
  _keytab["/webservice/ssl/dh"] = &configuration::_set_ssl_dh;
  _keytab["/webservice/ssl/password"] = &configuration::_set_ssl_password;
}

/**
 *  Default Destructor.
 */
configuration::~configuration() throw() {

}

/**
 *  Set the configuration filename.
 *
 *  @param[in] filename The configuration filename.
 */
void configuration::set_filename(QString const& filename) {
  _filename = filename;
}

/**
 *  Get the configuration filename.
 *
 *  @return The configuration filename.
 */
QString const& configuration::get_filename() const throw() {
  return (_filename);
}

/**
 *  Get the server host name.
 *
 *  @return The host name.
 */
QString const& configuration::get_host() const throw() {
  return (_host);
}

/**
 *  Get the listening port.
 *
 *  @return The port.
 */
int configuration::get_port() const throw() {
  return (_port);
}

/**
 *  Get The receive socket timeout.
 *
 *  @return The timeout in second.
 */
int configuration::get_recv_timeout() const throw() {
  return (_recv_timeout);
}

/**
 *  Get The send socket timeout.
 *
 *  @return The timeout in second.
 */
int configuration::get_send_timeout() const throw() {
  return (_send_timeout);
}

/**
 *  Get the accept socket timeout.
 *
 *  @return The timeout in micosecond.
 */
int configuration::get_accept_timeout() const throw() {
  return (_accept_timeout);
}

/**
 *  Get if the ssl is enable.
 *
 *  @return true is enable, false otherwise.
 */
bool configuration::get_ssl_enable() const throw() {
  return (_ssl_enable);
}

/**
 *  Get the keyfile path (for authentication).
 *
 *  @return The keyfile path.
 */
QString const& configuration::get_ssl_keyfile() const throw() {
  return (_ssl_keyfile);
}

/**
 *  Get the certificate path (for authentication).
 *
 *  @return The certificate path.
 */
QString const& configuration::get_ssl_cacert() const throw() {
  return (_ssl_cacert);
}

/**
 *  Get the Diffie-Helman path (for authentication).
 *
 *  @return The Diffie-Helman path.
 */
QString const& configuration::get_ssl_dh() const throw() {
  return (_ssl_dh);
}

/**
 *  Get the password (for authentication).
 *
 *  @return The password.
 */
QString const& configuration::get_ssl_password() const throw() {
  return (_ssl_password);
}

/**
 *  Parse configuration file.
 */
void configuration::parse() {
  QFile file(_filename);
  if (file.open(QFile::ReadOnly | QFile::Text) == false) {
    throw (error() << file.errorString());
  }
  _reader.setDevice(&file);

  while (!_reader.atEnd()) {
    if (_reader.isStartElement()) {
      _path += "/" + _reader.name().toString();

      QHash<QString, void (configuration::*)()>::const_iterator it = _keytab.find(_path);
      if (it == _keytab.end()) {
      	throw (error() << "line " << _reader.lineNumber());
      }
      if (it.value() != NULL) {
      	(this->*it.value())();
      }
    }
    if (_reader.isEndElement()) {
      _path = _path.left(_path.lastIndexOf('/'));
    }
    _reader.readNext();
  }
  file.close();
}

/**
 *  Set the server host name.
 */
void configuration::_set_host() {
  _host = _reader.readElementText();
}

/**
 *  Set the listening port.
 */
void configuration::_set_port() {
  bool ok;
  _port = _reader.readElementText().toInt(&ok);
  if (ok == false) {
    throw (error() << "line " << _reader.lineNumber());
  }
}

/**
 *  Set the receive socket timeout.
 */
void configuration::_set_recv_timeout() {
  bool ok;
  _recv_timeout = _reader.readElementText().toInt(&ok);
  if (ok == false) {
    throw (error() << "line " << _reader.lineNumber());
  }
}

/**
 *  Set the send socket timeout.
 */
void configuration::_set_send_timeout() {
  bool ok;
  _send_timeout = _reader.readElementText().toInt(&ok);
  if (ok == false) {
    throw (error() << "line " << _reader.lineNumber());
  }
}

/**
 *  Set the accept socket timeout.
 */
void configuration::_set_accept_timeout() {
  bool ok;
  _accept_timeout = _reader.readElementText().toInt(&ok) * -1000;
  if (ok == false) {
    throw (error() << "line " << _reader.lineNumber());
  }
}

/**
 *  Set if ssl is enable.
 */
void configuration::_set_ssl_enable() {
  QString const& value = _reader.readElementText();
  if (value == "true") {
    _ssl_enable = true;
  }
  else if (value == "false") {
    _ssl_enable = false;
  }
  else {
    throw (error() << "line " << _reader.lineNumber());
  }
}

/**
 *  Set the keyfile path (for authentication).
 */
void configuration::_set_ssl_keyfile() {
  _ssl_keyfile = _reader.readElementText();
}

/**
 *  Set the certificate path (for authentication).
 */
void configuration::_set_ssl_cacert() {
  _ssl_cacert = _reader.readElementText();
}

/**
 *  Set the Diffie-Helman path (for authentication).
 */
void configuration::_set_ssl_dh() {
  _ssl_dh = _reader.readElementText();
}

/**
 *  Set the password (for authentication).
 */
void configuration::_set_ssl_password() {
  _ssl_password = _reader.readElementText();
}
