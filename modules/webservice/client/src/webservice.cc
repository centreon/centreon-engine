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

#include <signal.h>
#include "com/centreon/engine/modules/webservice/error.hh"
#include "com/centreon/engine/modules/webservice/ssl.hh"
#include "com/centreon/engine/modules/webservice/webservice.hh"

using namespace com::centreon::engine::modules::webservice;

/**
 *  Default constructor.
 *
 *  @param[in] ssl_enable True for enable OpenSSL.
 *  @param[in] keyfile    The keyfile path (for authentication).
 *  @param[in] password   The password (for authentication).
 *  @param[in] cacert     the certificate path (for authentication).
 */
webservice::webservice(
              bool ssl_enable,
              QString const& keyfile,
              QString const& password,
              QString const& cacert)
  : _gen(auto_gen::instance()),
    _ssl_enable(ssl_enable) {
#ifndef WITH_OPENSSL
  (void)keyfile;
  (void)password;
  (void)cacert;
#endif // !WITH_OPENSSL

#ifdef WITH_OPENSSL
  if (_ssl_enable == true) {
    signal(SIGPIPE, _sigpipe_handle);
    soap_ssl_init();
    if (CRYPTO_thread_setup()) {
      throw (error("ssl thread setup failed."));
    }
  }
#endif // !WITH_OPENSSL
  soap_init(&_soap_ctx); // initialize the context (only once!).

#ifdef WITH_OPENSSL
  if (ssl_enable == true) {
    char const* key = (keyfile != "" ? keyfile.toStdString().c_str() : NULL);
    char const* pass = (password != "" ? password.toStdString().c_str() : NULL);
    char const* pem = (cacert != "" ? cacert.toStdString().c_str() : NULL);
    int flags = (key == NULL && password == NULL && pem == NULL
		 ? SOAP_SSL_NO_AUTHENTICATION
		 : SOAP_SSL_DEFAULT);
    if (soap_ssl_client_context(&_soap_ctx,
				flags,
				key,
				pass,
				pem,
				NULL,
				NULL)) {
      throw (error("create ssl context failed"));
    }
  }
#endif // !WITH_OPENSSL
}

/**
 *  Default destructor.
 */
webservice::~webservice() {
  soap_destroy(&_soap_ctx); // delete deserialized class instances (for C++).
  soap_end(&_soap_ctx);     // remove deserialized data and clean up.
  soap_done(&_soap_ctx);    // detach the gSOAP context.

#ifdef WITH_OPENSSL
  if (_ssl_enable == true) {
    CRYPTO_thread_cleanup();
  }
#endif // !WITH_OPENSSL
}

/**
 *  Execute soap function.
 *
 *  @param[in] function The webservice function name.
 *  @param[in] args     The arguments of this function.
 *
 *  @return Return true if the execution succeed, false otherwise.
 */
bool webservice::execute(QString const& function, QHash<QString, QString> const& args) {
  return (_gen.execute(
            function,
            &_soap_ctx,
            _end_point.toStdString().c_str(),
            _action.toStdString().c_str(),
            args));
}

/**
 *  Get the soap action.
 *
 *  @return The soap action.
 */
QString const& webservice::get_action() const throw() {
  return (_action);
}

/**
 *  Get the endpoint address.
 *
 *  @return The endpoint address.
 */
QString const& webservice::get_end_point() const throw() {
  return (_end_point);
}

/**
 *  Check if ssl is enable.
 *
 *  @return Return true if ssl is enable, false otherwise.
 */
bool webservice::is_ssl_enable() const throw() {
  return (_ssl_enable);
}

/**
 *  Set the soap action.
 *
 *  @return The soap action.
 */
void webservice::set_action(QString const& action) {
  _action = action;
}

/**
 *  Set the endpoint address.
 *
 *  @param[in] end_point The endpoint address.
 */
void webservice::set_end_point(QString const& end_point) {
  _end_point = end_point;
}

#ifdef WITH_OPENSSL
/**
 *  The Sigpipe handle for openssl.
 */
void webservice::_sigpipe_handle(int x) {
  (void)x;
}
#endif // !WITH_OPENSSL
