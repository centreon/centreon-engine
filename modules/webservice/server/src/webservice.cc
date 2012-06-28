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

#include <cassert>
#include <csignal>
#include <cstdlib>
#include <memory>
#include "com/centreon/concurrency/thread_pool.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/ssl.hh"
#include "com/centreon/engine/modules/webservice/webservice.hh"

using namespace com::centreon;
using namespace com::centreon::engine::modules::webservice;
using namespace com::centreon::engine::logging;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] config The configuration object
 *                    with all configuration.
 */
webservice::webservice(configuration const* config)
  : _config(config),
    _is_end(false) {
#ifndef WITH_OPENSSL
  if (_config->get_ssl_enable())
    logger(log_config_warning, basic)
      << "webservice configuration enable SSL but webservice module " \
         "was compiled without OpenSSL support";
#endif // !WITH_OPENSSL
  _init();
}

/**
 *  Destructor.
 */
webservice::~webservice() throw () {
  _is_end = true;
  wait();
}

/**************************************
*                                     *
*          Protected Methods          *
*                                     *
**************************************/

/**
 *  Start the webservice.
 */
void  webservice::_run() {
  // Thread pool.
  concurrency::thread_pool pool;
  pool.set_max_thread_count(_config->get_thread_count());

  // Process while end is not requested.
  while (!_is_end) {
    // Accept new connection.
    SOAP_SOCKET s(soap_accept(&_soap_ctx));
    if (!soap_valid_socket(s)) {
      if (!_soap_ctx.errnum)
        continue ;
      char buf_error[1024];
      soap_sprint_fault(&_soap_ctx, buf_error, sizeof(buf_error));
      logger(log_runtime_error, basic)
        << "webservice runtime error '" << buf_error << "'";
      break ;
    }

    // Copy SOAP context for threaded processing.
    soap* soap_cpy(soap_copy(&_soap_ctx));
    if (!soap_cpy) {
      char buf_error[1024];
      soap_sprint_fault(&_soap_ctx, buf_error, sizeof(buf_error));
      logger(log_runtime_error, basic)
        << "webservice runtime error '" << buf_error << "'";
      break ;
    }

#ifdef WITH_OPENSSL
    // SSL handshake.
    if (_config->get_ssl_enable()
        && (soap_ssl_accept(soap_cpy) != SOAP_OK)) {
      char buf_error[1024];
      soap_sprint_fault(soap_cpy, buf_error, sizeof(buf_error));
      logger(log_runtime_error, basic)
        << "webservice runtime error '" << buf_error << "'";
      continue ;
    }
#endif // !WITH_OPENSSL

    // Add task to thread pool.
    std::auto_ptr<concurrency::runnable>
      t(new webservice::query(soap_cpy));
    pool.start(t.get());
    t.release();
  }

  // Wait for all task termination.
  pool.wait_for_done();

  // Cleanup.
  soap_destroy(&_soap_ctx);
  soap_end(&_soap_ctx);
  soap_done(&_soap_ctx);
#ifdef WITH_OPENSSL
  if (_config->get_ssl_enable())
    CRYPTO_thread_cleanup();
#endif // !WITH_OPENSSL

  return ;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
webservice::webservice(webservice const& right)
  : concurrency::thread() {
  _internal_copy(right);
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
webservice& webservice::operator=(webservice const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Initialize webservice.
 */
void webservice::_init() {
  // SSL initialization.
#ifdef WITH_OPENSSL
  if (_config->get_ssl_enable()) {
    signal(SIGPIPE, _sigpipe_handle);
    soap_ssl_init();
    if (CRYPTO_thread_setup())
      throw (engine_error() << "SSL thread setup failed");
  }
#endif // !WITH_OPENSSL

  // SOAP initialization.
  soap_init(&_soap_ctx);
  _soap_ctx.accept_timeout = _config->get_accept_timeout();
  _soap_ctx.send_timeout = _config->get_send_timeout();
  _soap_ctx.recv_timeout = _config->get_recv_timeout();

  // SSL parameters.
#ifdef WITH_OPENSSL
  if (_config->get_ssl_enable()) {
    char const* keyfile(!_config->get_ssl_keyfile().empty()
                        ? _config->get_ssl_keyfile().c_str()
                        : NULL);
    char const* cacert(!_config->get_ssl_cacert().empty()
                       ? _config->get_ssl_cacert().c_str()
                       : NULL);
    char const* dh(!_config->get_ssl_dh().empty()
                   ? _config->get_ssl_dh().c_str()
                   : NULL);
    char const* password(!_config->get_ssl_password().empty()
                         ? _config->get_ssl_password().c_str()
                         : NULL);
    int flags((!keyfile && !cacert && !dh && !password)
              ? SOAP_SSL_NO_AUTHENTICATION
              : SOAP_SSL_DEFAULT);
    if (soap_ssl_server_context(
          &_soap_ctx,
          flags,
          keyfile,
          password,
          cacert,
          NULL,
          dh,
          NULL,
          NULL))
      throw (engine_error() << "create ssl context with host '"
             << _config->get_host() << "' on port "
             << _config->get_port() << " failed");
  }
#endif // !WITH_OPENSSL

  // SOAP parameters.
  char const* host(!_config->get_host().empty()
                   ? _config->get_host().c_str()
                   : NULL);
  SOAP_SOCKET m_socket(soap_bind(
                         &_soap_ctx,
                         host,
                         _config->get_port(),
                         100));
  if (!soap_valid_socket(m_socket))
    throw (engine_error() << "bind with host '" << _config->get_host()
           << "' on port " << _config->get_port() << " failed");

  return ;
}

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void webservice::_internal_copy(webservice const& right) {
  (void)right;
  assert(!"webservice is not copyable");
  abort();
  return ;
}

/**
 *  Silently discard SIGPIPE.
 *
 *  @param[in] x  Unused.
 */
void webservice::_sigpipe_handle(int x) {
  (void)x;
  return ;
}

/**************************************
*                                     *
*        Query Public Methods         *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] s  The soap context.
 */
webservice::query::query(soap* s) : _soap(s) {}

/**
 *  Destructor.
 */
webservice::query::~query() throw () {}

/**
 *  Execute soap query.
 */
void webservice::query::run() {
  // Processing.
  if (_soap) {
    soap_serve(_soap);
    soap_destroy(_soap);
    soap_end(_soap);
    soap_free(_soap);
  }

  // Self deletion.
  delete this;

  return ;
}
