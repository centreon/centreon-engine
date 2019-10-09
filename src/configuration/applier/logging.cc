/*
** Copyright 2011-2014,2018 Centreon
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
#include <syslog.h>
#include "com/centreon/engine/configuration/applier/logging.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/debug_file.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/logging/engine.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;

static applier::logging* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::logging::apply(state& config) {
  if (verify_config || test_scheduling)
    return ;

  // Syslog.
  if (config.use_syslog() && !_syslog)
    _add_syslog();
  else if (!config.use_syslog() && _syslog)
    _del_syslog();

  // Standard log file.
  if (config.log_file() == "")
    _del_log_file();
  else if (!_log || config.log_file() != _log->filename()) {
    _add_log_file(config);
    _del_stdout();
    _del_stderr();
  }

  // Debug file.
  if ((config.debug_file() == "")
      || !config.debug_level()
      || !config.debug_verbosity()) {
    _del_debug();
    _debug_level = config.debug_level();
    _debug_verbosity = config.debug_verbosity();
    _debug_max_size = config.max_debug_file_size();
  }
  else if (!_debug
           || config.debug_file() != _debug->filename()
           || config.debug_level() != _debug_level
           || config.debug_verbosity() != _debug_verbosity
           || config.max_debug_file_size() != _debug_max_size)
    _add_debug(config);
}

/**
 *  Get the singleton instance of logging applier.
 *
 *  @return Singleton instance.
 */
applier::logging& applier::logging::instance() {
  assert(_instance);
  return (*_instance);
}

/**
 *  Load logging applier singleton.
 */
void applier::logging::load() {
  if (!_instance)
    _instance = new applier::logging;
}

/**
 *  Unload logging applier singleton.
 */
void applier::logging::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::logging::logging()
  : _debug(NULL),
    _debug_level(0),
    _debug_max_size(0),
    _debug_verbosity(0),
    _log(NULL),
    _stderr(NULL),
    _stdout(NULL),
    _syslog(NULL) {
  _add_stdout();
  _add_stderr();
}

/**
 *  Construct and apply configuration.
 *
 *  @param[in] config The initial confiuration.
 */
applier::logging::logging(state& config)
  : _debug(NULL),
    _debug_level(0),
    _debug_max_size(0),
    _debug_verbosity(0),
    _log(NULL),
    _stderr(NULL),
    _stdout(NULL),
    _syslog(NULL) {
  _add_stdout();
  _add_stderr();
  apply(config);
}

/**
 *  Default destructor.
 */
applier::logging::~logging() throw() {
  _del_stdout();
  _del_stderr();
  _del_syslog();
  _del_log_file();
  _del_debug();
}

/**
 *  Add stdout object logging.
 */
void applier::logging::_add_stdout() {
  if (!_stdout) {
    _stdout = new com::centreon::logging::file(stdout);
    unsigned long long type(
                           engine::logging::log_process_info
                         | engine::logging::log_verification_error
                         | engine::logging::log_verification_warning
                         | engine::logging::log_config_error
                         | engine::logging::log_config_warning
                         | engine::logging::log_event_handler
                         | engine::logging::log_external_command
                         | engine::logging::log_host_up
                         | engine::logging::log_host_down
                         | engine::logging::log_host_unreachable
                         | engine::logging::log_service_ok
                         | engine::logging::log_service_unknown
                         | engine::logging::log_service_warning
                         | engine::logging::log_service_critical
                         | engine::logging::log_passive_check
                         | engine::logging::log_info_message
                         | engine::logging::log_host_notification
                         | engine::logging::log_service_notification);
    com::centreon::logging::engine::instance().add(
                                                 _stdout,
                                                 type,
                                                 engine::logging::most);
  }
}

/**
 *  Add stderr object logging.
 */
void applier::logging::_add_stderr() {
  if (!_stderr) {
    _stderr = new com::centreon::logging::file(stderr);
    unsigned long long type(
                         engine::logging::log_runtime_error
                         | engine::logging::log_runtime_warning);
    com::centreon::logging::engine::instance().add(
                                                 _stderr,
                                                 type,
                                                 engine::logging::most);
  }
}
/**
 *  Add syslog object logging.
 */
void applier::logging::_add_syslog() {
  if (!_syslog) {
    _syslog = new com::centreon::logging::syslogger(
                                            "centreon-engine",
                                            LOG_USER);
    com::centreon::logging::engine::instance().add(
                                                 _syslog,
                                                 engine::logging::log_all,
                                                 engine::logging::basic);
  }
}

/**
 *  Add file object logging.
 */
void applier::logging::_add_log_file(state const& config) {
  _del_log_file();
  _log = new com::centreon::logging::file(
                                       config.log_file(),
                                       true,
                                       config.log_pid());
  com::centreon::logging::engine::instance().add(
                                               _log,
                                               engine::logging::log_all,
                                               engine::logging::most);
}

/**
 *  Add debug object logging.
 */
void applier::logging::_add_debug(state const& config) {
  _del_debug();
  _debug_level = (config.debug_level() << 32) | engine::logging::log_all;
  _debug_verbosity = config.debug_verbosity();
  _debug_max_size = config.max_debug_file_size();
  _debug = new com::centreon::engine::logging::debug_file(
                                                 config.debug_file(),
                                                 _debug_max_size);
  com::centreon::logging::engine::instance().add(
                                               _debug,
                                               _debug_level,
                                               _debug_verbosity);
}

/**
 *  Remove syslog object logging.
 */
void applier::logging::_del_syslog() {
  if (_syslog) {
    com::centreon::logging::engine::instance().remove(_syslog);
    delete _syslog;
    _syslog = NULL;
  }
}

/**
 *  Remove file object logging.
 */
void applier::logging::_del_log_file() {
  if (_log) {
    com::centreon::logging::engine::instance().remove(_log);
    delete _log;
    _log = NULL;
  }
}

/**
 *  Remove debug object logging.
 */
void applier::logging::_del_debug() {
  if (_debug) {
    com::centreon::logging::engine::instance().remove(_debug);
    delete _debug;
    _debug = NULL;
  }
}

/**
 *  Remove stdout object logging.
 */
void applier::logging::_del_stdout() {
  if (_stdout) {
    com::centreon::logging::engine::instance().remove(_stdout);
    delete _stdout;
    _stdout = NULL;
  }
}

/**
 *  Remove stderr object logging.
 */
void applier::logging::_del_stderr() {
  if (_stderr) {
    com::centreon::logging::engine::instance().remove(_stderr);
    delete _stderr;
    _stderr = NULL;
  }
}
