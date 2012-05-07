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

#include <QThreadPool>
#include <signal.h>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/ssl.hh"
#include "com/centreon/engine/modules/webservice/webservice.hh"

using namespace com::centreon::engine::modules::webservice;
using namespace com::centreon::engine::logging;

/**
 *  Default Constructor.
 *
 *  @param[in] config The configuration object
 *                    with all configuration.
 */
webservice::webservice(configuration const& config)
  : QThread(0),
    _config(config),
    _is_end(false) {
#ifndef WITH_OPENSSL
  if (config.get_ssl_enable() == true) {
    logger(log_config_warning, basic)
      << "webservice configuration enable ssl but webservice module was " \
      "compiled without openssl library.";
  }
#endif // !WITH_OPENSSL

  _init();
}

/**
 *  Default Destructor.
 */
webservice::~webservice() throw() {
  _is_end = true;
  wait();
}

/**
 *  Start the webservice.
 */
void  webservice::run() {
  QThreadPool pool;
  pool.setMaxThreadCount(_config.get_thread_count());

  while (_is_end != true) {
    SOAP_SOCKET s = soap_accept(&_soap_ctx);
    if (!soap_valid_socket(s)) {
      if (!_soap_ctx.errnum) {
	continue;
      }
      char buf_error[1024];
      soap_sprint_fault(&_soap_ctx, buf_error, sizeof(buf_error));
      logger(log_runtime_error, basic)
        << "webservice runtime error `" << buf_error << "'.";
      break;
    }

    soap* soap_cpy = soap_copy(&_soap_ctx);
    if (!soap_cpy) {
      char buf_error[1024];
      soap_sprint_fault(&_soap_ctx, buf_error, sizeof(buf_error));
      logger(log_runtime_error, basic)
        << "webservice runtime error `" << buf_error << "'.";
      break;
    }

#ifdef WITH_OPENSSL
    if (_config.get_ssl_enable() == true
	&& soap_ssl_accept(soap_cpy) != SOAP_OK) {
      char buf_error[1024];
      soap_sprint_fault(soap_cpy, buf_error, sizeof(buf_error));
      logger(log_runtime_error, basic)
        << "webservice runtime error `" << buf_error << "'.";
      continue;
    }
#endif // !WITH_OPENSSL

    pool.start(new query(soap_cpy));
  }

  pool.waitForDone();

  soap_destroy(&_soap_ctx); // remove deserialized class instances (C++ only).
  soap_end(&_soap_ctx);     // clean up and remove deserialized data.
  soap_done(&_soap_ctx);    // detach context (last use and no longer in scope).

#ifdef WITH_OPENSSL
  if (_config.get_ssl_enable() == true) {
    CRYPTO_thread_cleanup();
  }
#endif // !WITH_OPENSSL
}

/**
 *  Initilize webservice.
 */
void webservice::_init() {
#ifdef WITH_OPENSSL
  if (_config.get_ssl_enable() == true) {
    signal(SIGPIPE, _sigpipe_handle);
    soap_ssl_init();
    if (CRYPTO_thread_setup()) {
      throw (engine_error() << "ssl thread setup failed.");
    }
  }
#endif // !WITH_OPENSSL

  soap_init(&_soap_ctx);
  _soap_ctx.accept_timeout = _config.get_accept_timeout();
  _soap_ctx.send_timeout = _config.get_send_timeout();
  _soap_ctx.recv_timeout = _config.get_recv_timeout();

#ifdef WITH_OPENSSL
  if (_config.get_ssl_enable() == true) {
    char const* keyfile = (_config.get_ssl_keyfile() != "" ? _config.get_ssl_keyfile().toStdString().c_str() : NULL);
    char const* cacert = (_config.get_ssl_cacert() != "" ? _config.get_ssl_cacert().toStdString().c_str() : NULL);
    char const* dh = (_config.get_ssl_dh() != "" ? _config.get_ssl_dh().toStdString().c_str() : NULL);
    char const* password = (_config.get_ssl_password() != "" ? _config.get_ssl_password().toStdString().c_str() : NULL);
    int flags = (keyfile == NULL && cacert == NULL && dh == NULL && password == NULL
		 ? SOAP_SSL_NO_AUTHENTICATION
		 : SOAP_SSL_DEFAULT);
    if (soap_ssl_server_context(&_soap_ctx,
				flags,
				keyfile,
				password,
				cacert,
				NULL,
				dh,
				NULL,
				NULL)) {
      throw (engine_error() << "create ssl context with host `"
	     << _config.get_host() << "' on port `" << _config.get_port() << "' failed.");
    }
  }
#endif // !WITH_OPENSSL

  char const* host = (_config.get_host() != "" ? _config.get_host().toStdString().c_str() : NULL);
  SOAP_SOCKET m_socket = soap_bind(&_soap_ctx,
				   host,
				   _config.get_port(),
				   100);
  if (!soap_valid_socket(m_socket)) {
    throw (engine_error() << "bind with host `" << _config.get_host()
	   << "' on port `" << _config.get_port() << "' failed.");
  }
}

/**
 *  Do nothing.
 *
 *  @param[in] x  Unused.
 */
void webservice::_sigpipe_handle(int x) {
  (void)x;
}

/**
 *  Constructor.
 *  Set qrunnable with auto delete.
 *
 *  @param[in] s  The soap context.
 */
webservice::query::query(soap* s)
  : _soap(s) {
  setAutoDelete(true);
}

/**
 *  Destructor.
 */
webservice::query::~query() {

}

/**
 *  Execute soap query.
 */
void webservice::query::run() {
  if (!_soap)
    return;

  soap_serve(_soap);

  soap_destroy(_soap);
  soap_end(_soap);
  soap_free(_soap);
}
