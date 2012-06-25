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
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration::applier;

logging* logging::_instance = NULL;

/**
 *  Get the singleton instance of logging applier.
 *
 *  @return Singleton instance.
 */
logging& logging::instance() {
  return (*_instance);
}

/**
 *  Load logging applier singleton.
 */
void logging::load() {
  if (!_instance)
    _instance = new logging;
}

/**
 *  Unload logging applier singleton.
 */
void logging::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void logging::apply(state const& config) {
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

  return ;
}

/**
 *  Default constructor.
 */
logging::logging()
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
 *  Construct and aply configuration.
 *
 *  @param[in] config The initial confiuration.
 */
logging::logging(state const& config)
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
logging::logging(logging& right)
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
logging::~logging() throw() {
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
logging& logging::operator=(logging& right) {
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
void logging::_add_stdout() {
  if (_stdout_id == 0) {
    shared_ptr<object>
      obj(new com::centreon::engine::logging::standard());
    unsigned long long type(log_process_info
                            | log_event_handler
                            | log_external_command
                            | log_host_up
                            | log_host_down
                            | log_host_unreachable
                            | log_service_ok
                            | log_service_unknown
                            | log_service_warning
                            | log_service_critical
                            | log_passive_check
                            | log_info_message
                            | log_host_notification
                            | log_service_notification);
    ::engine::obj_info info(obj, type, most);
    _stdout_id = ::engine::instance().add_object(info);
  }
}

/**
 *  Add stderr object logging.
 */
void logging::_add_stderr() {
  if (_stderr_id == 0) {
    shared_ptr<object> obj(new standard(false));
    unsigned long long type(log_runtime_error
                            | log_runtime_warning
                            | log_verification_error
                            | log_verification_warning
                            | log_config_error
                            | log_config_warning);
    ::engine::obj_info info(obj, type, most);
    _stderr_id = ::engine::instance().add_object(info);
  }
}
/**
 *  Add syslog object logging.
 */
void logging::_add_syslog() {
  shared_ptr<object> obj(new syslog);
  ::engine::obj_info info(obj, log_all, basic);
  _syslog_id = ::engine::instance().add_object(info);
}

/**
 *  Add file object logging.
 */
void logging::_add_log_file(state const& config) {
  _del_log_file();
  shared_ptr<object> obj(new file(
                               config.get_log_file(),
                               config.get_max_log_file_size()));
  ::engine::obj_info info(obj, log_all, most);
  _log_id = ::engine::instance().add_object(info);
}

/**
 *  Add debug object logging.
 */
void logging::_add_debug(state const& config) {
  _del_debug();
  shared_ptr<object> obj(new file(
                               config.get_debug_file(),
                               config.get_max_debug_file_size()));
  ::engine::obj_info info(
                       obj,
                       config.get_debug_level(),
                       config.get_debug_verbosity());
  _debug_id = ::engine::instance().add_object(info);
  return ;
}

/**
 *  Remove syslog object logging.
 */
void logging::_del_syslog() {
  if (_syslog_id != 0) {
    ::engine::instance().remove_object(_syslog_id);
    _syslog_id = 0;
  }
  return ;
}

/**
 *  Remove file object logging.
 */
void logging::_del_log_file() {
  if (_log_id != 0) {
    ::engine::instance().remove_object(_log_id);
    _log_id = 0;
  }
  return ;
}

/**
 *  Remove debug object logging.
 */
void logging::_del_debug() {
  if (_debug_id != 0) {
    ::engine::instance().remove_object(_debug_id);
    _debug_id = 0;
  }
  return ;
}

/**
 *  Remove stdout object logging.
 */
void logging::_del_stdout() {
  if (_stdout_id != 0) {
    ::engine::instance().remove_object(_stdout_id);
    _stdout_id = 0;
  }
  return ;
}

/**
 *  Remove stderr object logging.
 */
void logging::_del_stderr() {
  if (_stderr_id != 0) {
    ::engine::instance().remove_object(_stderr_id);
    _stderr_id = 0;
  }
  return ;
}
