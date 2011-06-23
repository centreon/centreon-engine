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

#include "logging/engine.hh"
#include "configuration/applier/logging.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration::applier;

/**
 *  Default constructor.
 */
logging::logging()
  : _debug_limit(0),
    _debug_level(0),
    _debug_verbosity(0),
    _stdout_id(0),
    _stderr_id(0),
    _syslog_id(0),
    _file_id(0),
    _debug_id(0) {
  _add_stdout();
  _add_stderr();
}

/**
 *  Construct and aply configuration.
 *
 *  @param[in] config The initial confiuration.
 */
logging::logging(state const& config)
  : _debug_limit(0),
    _debug_level(0),
    _debug_verbosity(0),
    _stdout_id(0),
    _stderr_id(0),
    _syslog_id(0),
    _file_id(0),
    _debug_id(0) {
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
    _debug_limit(0),
    _debug_level(0),
    _debug_verbosity(0),
    _stdout_id(0),
    _stderr_id(0),
    _syslog_id(0),
    _file_id(0),
    _debug_id(0) {
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
    _log_file = right._log_file;
    _log_archive_path = right._log_archive_path;
    _debug_file = right._debug_file;
    _debug_limit = right._debug_limit;
    _debug_level = right._debug_verbosity;
    _stdout_id = right._stdout_id;
    _stderr_id = right._stderr_id;
    _syslog_id = right._syslog_id;
    _file_id = right._file_id;
    _debug_id = right._debug_id;

    right._stdout_id = 0;
    right._stderr_id = 0;
    right._syslog_id = 0;
    right._file_id = 0;
    right._debug_id = 0;
  }
  return (*this);
}

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void logging::apply(state const& config) {
  if (config.get_use_syslog() == true && _syslog_id == 0) {
    _add_syslog();
  }
  else if (config.get_use_syslog() == false && _syslog_id != 0) {
    _del_syslog();
  }

  if (config.get_log_file() == "") {
    _del_log_file();
  }
  else if (config.get_log_file() != _log_file
      || config.get_log_archive_path() != _log_archive_path) {
    _add_log_file(config);
    _del_stdout();
    _del_stderr();
  }

  if (config.get_debug_file() == "") {
    _del_debug();
  }
  else if (config.get_debug_file() != _debug_file
	   || config.get_max_debug_file_size() != _debug_limit
	   || config.get_debug_level() != _debug_level
	   || config.get_debug_verbosity() != _debug_verbosity) {
    _add_debug(config);
  }
}

/**
 *  Add stdout object logging.
 */
void logging::_add_stdout() {
  if (_stdout_id == 0) {
    QSharedPointer<standard> obj(new standard());
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
    QSharedPointer<standard> obj(new standard(false));
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
  QSharedPointer<syslog> obj(new syslog);
  ::engine::obj_info info(obj, log_all, basic);
  _syslog_id = ::engine::instance().add_object(info);
}

/**
 *  Add file object logging.
 */
void logging::_add_log_file(state const& config) {
  if (_file_id != 0) {
    ::engine::instance().remove_object(_file_id);
  }
  QSharedPointer<file> obj(new file(config.get_log_file(), config.get_log_archive_path()));
  ::engine::obj_info info(obj, log_all, most);
  _file_id = ::engine::instance().add_object(info);
}

/**
 *  Add debug object logging.
 */
void logging::_add_debug(state const& config) {
  if (_debug_id != 0) {
    ::engine::instance().remove_object(_debug_id);
  }
  QSharedPointer<file> obj(new file(config.get_debug_file(),
				    config.get_max_debug_file_size()));
  ::engine::obj_info info(obj,
			  config.get_debug_level(),
			  config.get_debug_verbosity());
  _debug_id = ::engine::instance().add_object(info);
}

/**
 *  Remove syslog object logging.
 */
void logging::_del_syslog() {
  if (_syslog_id != 0) {
    ::engine::instance().remove_object(_syslog_id);
    _syslog_id = 0;
  }
}

/**
 *  Remove file object logging.
 */
void logging::_del_log_file() {
  if (_file_id != 0) {
    ::engine::instance().remove_object(_file_id);
    _file_id = 0;
  }
}

/**
 *  Remove debug object logging.
 */
void logging::_del_debug() {
  if (_debug_id != 0) {
    ::engine::instance().remove_object(_debug_id);
    _debug_id = 0;
  }
}

/**
 *  Remove stdout object logging.
 */
void logging::_del_stdout() {
  if (_stdout_id != 0) {
    ::engine::instance().remove_object(_stdout_id);
    _stdout_id = 0;
  }
}

/**
 *  Remove stderr object logging.
 */
void logging::_del_stderr() {
  if (_stderr_id != 0) {
    ::engine::instance().remove_object(_stderr_id);
    _stderr_id = 0;
  }
}
