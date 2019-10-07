/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_LOGGING_LOGGER_HH
#define CCE_LOGGING_LOGGER_HH

#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/logging/temp_logger.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/namespace.hh"

CCE_BEGIN()

namespace logging {
/**
 *  @enum object::e_type
 *  Logging types.
 */
enum type_value {
  none = 0ull,

  log_runtime_error = (1ull << 1),
  log_runtime_warning = (1ull << 2),
  log_verification_error = (1ull << 3),
  log_verification_warning = (1ull << 4),
  log_config_error = (1ull << 5),
  log_config_warning = (1ull << 6),
  log_process_info = (1ull << 7),
  log_event_handler = (1ull << 8),
  log_external_command = (1ull << 9),
  log_host_up = (1ull << 10),
  log_host_down = (1ull << 11),
  log_host_unreachable = (1ull << 12),
  log_service_ok = (1ull << 13),
  log_service_unknown = (1ull << 14),
  log_service_warning = (1ull << 15),
  log_service_critical = (1ull << 16),
  log_passive_check = (1ull << 17),
  log_info_message = (1ull << 18),
  log_host_notification = (1ull << 19),
  log_service_notification = (1ull << 20),
  log_all = (1ull << 21),

  dbg_functions = (1ull << 32),
  dbg_config = (1ull << 33),
  dbg_process = (1ull << 34),
  dbg_statusdata = (1ull << 34),
  dbg_retentiondata = (1ull << 34),
  dbg_events = (1ull << 35),
  dbg_checks = (1ull << 36),
  dbg_ipc = (1ull << 36),
  dbg_flapping = (1ull << 36),
  dbg_eventhandlers = (1ull << 36),
  dbg_perfdata = (1ull << 36),
  dbg_notifications = (1ull << 37),
  dbg_eventbroker = (1ull << 38),
  dbg_external_command = (1ull << 39),
  dbg_commands = (1ull << 40),
  dbg_downtime = (1ull << 41),
  dbg_comments = (1ull << 42),
  dbg_macros = (1ull << 43),
  dbg_all = (1ull << 44),

  type_info = (1ull << 61),
  type_debug = (1ull << 62),
  type_error = (1ull << 63),

  all = log_all | dbg_all
};

/**
 *  @enum object::e_verbose
 *  Logging verbosity.
 */
enum verbosity_level { basic = 0u, more = 1u, most = 2u };
}  // namespace logging

CCE_END()

#define logger(type, verbose)                                              \
  for (unsigned int __com_centreon_engine_logging_define_ui(0);            \
       !__com_centreon_engine_logging_define_ui &&                         \
       com::centreon::engine::logging::engine::instance().is_log(type,     \
                                                                 verbose); \
       ++__com_centreon_engine_logging_define_ui)                          \
  com::centreon::engine::logging::temp_logger(type, verbose)

#define log_error(verbose) \
  logger(com::centreon::engine::logging::type_error, verbose) << "[error] "

#endif  // !CCE_LOGGING_LOGGER_HH
