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

#include "com/centreon/engine/configuration/applier/logging.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;

static applier::logging* _instance = NULL;

/**
 *  Get the singleton instance of logging applier.
 *
 *  @return Singleton instance.
 */
applier::logging& applier::logging::instance() {
  return (*_instance);
}

/**
 *  Load logging applier singleton.
 */
void applier::logging::load() {
  if (!_instance)
    _instance = new applier::logging;
  return;
}

/**
 *  Unload logging applier singleton.
 */
void applier::logging::unload() {
  delete _instance;
  _instance = NULL;
  return;
}

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::logging::apply(state const& config) {
  // Syslog.
  if (config.get_use_syslog() == true && _syslog_id == 0)
    _add_syslog();
  else if (config.get_use_syslog() == false && _syslog_id != 0)
    _del_syslog();

  // Standard log file.
  if (config.get_log_file() == "")
    _del_log_file();
  else if (config.get_log_file() != _log_file
           || config.get_max_log_file_size() != _log_limit) {
    _add_log_file(config);
    _del_stdout();
    _del_stderr();
  }

  // Debug file.
  if ((config.get_debug_file() == "")
      || !config.get_debug_level()
      || !config.get_debug_verbosity()) {
    _del_debug();
    _debug_file = config.get_debug_file();
    _debug_level = config.get_debug_level();
    _debug_verbosity = config.get_debug_verbosity();
  }
  else if ((config.get_debug_file() != _debug_file)
           || (config.get_max_debug_file_size() != _debug_limit)
           || (config.get_debug_level() != _debug_level)
           || (config.get_debug_verbosity() != _debug_verbosity))
    _add_debug(config);
  return;
}

/**
 *  Default constructor.
 */
applier::logging::logging()
  : _debug_id(0),
    _debug_level(0),
    _debug_limit(0),
    _debug_verbosity(0),
    _log_id(0),
    _log_limit(0),
    _stderr_id(0),
    _stdout_id(0),
    _syslog_id(0) {
  _add_stdout();
  _add_stderr();
}

/**
 *  Construct and apply configuration.
 *
 *  @param[in] config The initial confiuration.
 */
applier::logging::logging(state const& config)
  : _debug_id(0),
    _debug_level(0),
    _debug_limit(0),
    _debug_verbosity(0),
    _log_id(0),
    _log_limit(0),
    _stderr_id(0),
    _stdout_id(0),
    _syslog_id(0) {
  _add_stdout();
  _add_stderr();
  apply(config);
}

/**
 *  Default copy constructor.
 *
 *  @param[in,out] right The class to copy.
 */
applier::logging::logging(applier::logging& right)
  : base(right),
    _debug_id(0),
    _debug_level(0),
    _debug_limit(0),
    _debug_verbosity(0),
    _log_id(0),
    _log_limit(0),
    _stderr_id(0),
    _stdout_id(0),
    _syslog_id(0) {
  operator=(right);
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
 *  Default copy operator.
 *
 *  @param[in,out] right The class to copy.
 */
applier::logging& applier::logging::operator=(applier::logging& right) {
  if (this != &right) {
    _debug_file = right._debug_file;
    _debug_id = right._debug_id;
    _debug_level = right._debug_level;
    _debug_limit = right._debug_limit;
    _debug_verbosity = right._debug_verbosity;
    _log_file = right._log_file;
    _log_id = right._log_id;
    _log_limit = right._log_limit;
    _stderr_id = right._stderr_id;
    _stdout_id = right._stdout_id;
    _syslog_id = right._syslog_id;

    right._stdout_id = 0;
    right._stderr_id = 0;
    right._syslog_id = 0;
    right._log_id = 0;
    right._debug_id = 0;
  }
  return (*this);
}

/**
 *  Add stdout object logging.
 */
void applier::logging::_add_stdout() {
  if (_stdout_id == 0) {
    shared_ptr<engine::logging::object>
      obj(new engine::logging::standard());
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
    engine::logging::engine::obj_info info(
                                        obj,
                                        type,
                                        engine::logging::most);
    _stdout_id = engine::logging::engine::instance().add_object(info);
  }
  return;
}

/**
 *  Add stderr object logging.
 */
void applier::logging::_add_stderr() {
  if (_stderr_id == 0) {
    shared_ptr<engine::logging::object>
      obj(new engine::logging::standard(false));
    unsigned long long type(
                         engine::logging::log_runtime_error
                         | engine::logging::log_runtime_warning);

    engine::logging::engine::obj_info info(
                                        obj,
                                        type,
                                        engine::logging::most);
    _stderr_id = engine::logging::engine::instance().add_object(info);
  }
  return;
}
/**
 *  Add syslog object logging.
 */
void applier::logging::_add_syslog() {
  shared_ptr<engine::logging::object>
    obj(new engine::logging::syslog);
  engine::logging::engine::obj_info info(
                                       obj,
                                       engine::logging::log_all,
                                       engine::logging::basic);
  _syslog_id = engine::logging::engine::instance().add_object(info);
  return;
}

/**
 *  Add file object logging.
 */
void applier::logging::_add_log_file(state const& config) {
  _del_log_file();
  shared_ptr<engine::logging::object>
    obj(new engine::logging::file(
                               config.get_log_file(),
                               config.get_max_log_file_size()));
  engine::logging::engine::obj_info info(
                                      obj,
                                      engine::logging::log_all,
                                      engine::logging::most);
  _log_id = engine::logging::engine::instance().add_object(info);
  return;
}

/**
 *  Add debug object logging.
 */
void applier::logging::_add_debug(state const& config) {
  _del_debug();
  shared_ptr<engine::logging::object>
    obj(new engine::logging::file(
                               config.get_debug_file(),
                               config.get_max_debug_file_size()));
  engine::logging::engine::obj_info info(
                                      obj,
                                      config.get_debug_level(),
                                      config.get_debug_verbosity());
  _debug_id = engine::logging::engine::instance().add_object(info);
  return;
}

/**
 *  Remove syslog object logging.
 */
void applier::logging::_del_syslog() {
  if (_syslog_id != 0) {
    engine::logging::engine::instance().remove_object(_syslog_id);
    _syslog_id = 0;
  }
  return;
}

/**
 *  Remove file object logging.
 */
void applier::logging::_del_log_file() {
  if (_log_id != 0) {
    engine::logging::engine::instance().remove_object(_log_id);
    _log_id = 0;
  }
  return;
}

/**
 *  Remove debug object logging.
 */
void applier::logging::_del_debug() {
  if (_debug_id != 0) {
    engine::logging::engine::instance().remove_object(_debug_id);
    _debug_id = 0;
  }
  return;
}

/**
 *  Remove stdout object logging.
 */
void applier::logging::_del_stdout() {
  if (_stdout_id != 0) {
    engine::logging::engine::instance().remove_object(_stdout_id);
    _stdout_id = 0;
  }
  return;
}

/**
 *  Remove stderr object logging.
 */
void applier::logging::_del_stderr() {
  if (_stderr_id != 0) {
    engine::logging::engine::instance().remove_object(_stderr_id);
    _stderr_id = 0;
  }
  return;
}
