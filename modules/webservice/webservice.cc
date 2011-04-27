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

#include "ssl.hh"
#include "logging.hh"
#include "webservice.hh"
#include "error.hh"

using namespace com::centreon::engine::modules;

/**
 *  Default Constructor.
 *
 *  @param[in] config The configuration object
 *                    with all configuration.
 */
webservice::webservice(configuration const& config)
  : QThread(0),
    _is_end(false),
    _config(config) {
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
  while (_is_end != true) {
    SOAP_SOCKET s = soap_accept(&_soap_ctx);
    if (!soap_valid_socket(s)) {
      if (!_soap_ctx.errnum) {
	continue;
      }
      char buf_error[1024];
      soap_sprint_fault(&_soap_ctx, buf_error, sizeof(buf_error));
      logit(NSLOG_RUNTIME_ERROR, false,
	    "webservice runtime error `%s'.\n",
	    buf_error);
      break;
    }

    soap* soap_cpy = soap_copy(&_soap_ctx);
    if (!soap_cpy) {
      char buf_error[1024];
      soap_sprint_fault(&_soap_ctx, buf_error, sizeof(buf_error));
      logit(NSLOG_RUNTIME_ERROR, false,
	    "webservice runtime error `%s'.\n",
	    buf_error);
      break;
    }

    if (_config.get_ssl_enable() == true
	&& soap_ssl_accept(soap_cpy) != SOAP_OK) {
      char buf_error[1024];
      soap_sprint_fault(soap_cpy, buf_error, sizeof(buf_error));
      logit(NSLOG_RUNTIME_ERROR, false,
	    "webservice runtime error `%s'.\n",
	    buf_error);
      continue;
    }

    soap_serve(soap_cpy);

    soap_destroy(soap_cpy);
    soap_end(soap_cpy);
    soap_free(soap_cpy);
  }

  soap_destroy(&_soap_ctx); // remove deserialized class instances (C++ only).
  soap_end(&_soap_ctx);     // clean up and remove deserialized data.
  soap_done(&_soap_ctx);    // detach context (last use and no longer in scope).

  if (_config.get_ssl_enable() == true) {
    CRYPTO_thread_cleanup();
  }
}

void webservice::_init() {
  if (_config.get_ssl_enable() == true) {
    soap_ssl_init();
    if (CRYPTO_thread_setup()) {
      throw (error() << "ssl thread setup failed.");
    }
  }
  soap_init(&_soap_ctx);

  if (_config.get_ssl_enable() == true) {
    char const* keyfile = (_config.get_ssl_keyfile() != "" ? _config.get_ssl_keyfile().toStdString().c_str() : NULL);
    char const* cacert = (_config.get_ssl_cacert() != "" ? _config.get_ssl_cacert().toStdString().c_str() : NULL);
    char const* dh = (_config.get_ssl_dh() != "" ? _config.get_ssl_dh().toStdString().c_str() : NULL);
    char const* password = (_config.get_ssl_password() != "" ? _config.get_ssl_password().toStdString().c_str() : NULL);
    if (soap_ssl_server_context(&_soap_ctx,
				SOAP_SSL_DEFAULT,
				keyfile,
				password,
				cacert,
				NULL,
				dh,
				NULL,
				NULL)) {
      throw (error() << "create ssl context with host `"
	     << _config.get_host() << "' on port `" << _config.get_port() << "' failed.");
    }
  }

  char const* host = (_config.get_host() != "" ? _config.get_host().toStdString().c_str() : NULL);
  SOAP_SOCKET m_socket = soap_bind(&_soap_ctx,
				   host,
				   _config.get_port(),
				   100);
  if (!soap_valid_socket(m_socket)) {
    throw (error() << "bind with host `" << _config.get_host()
	   << "' on port `" << _config.get_port() << "' failed.");
  }
}
