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

#include "com/centreon/engine/namespace.hh"
#include "com/centreon/logging/temp_logger.hh"

CCE_BEGIN()

namespace logging {
/**
 *  @enum object::e_type
 *  Logging types.
 */
enum type_value {
  none = 0ull,

  log_runtime_error = 1ull,
  log_runtime_warning = 2ull,
  log_verification_error = 4ull,
  log_verification_warning = 8ull,
  log_config_error = 16ull,
  log_config_warning = 32ull,
  log_process_info = 64ull,
  log_event_handler = 128ull,
  log_external_command = 512ull,
  log_host_up = 1024ull,
  log_host_down = 2048ull,
  log_host_unreachable = 4096ull,
  log_service_ok = 8192ull,
  log_service_unknown = 16384ull,
  log_service_warning = 32768ull,
  log_service_critical = 65536ull,
  log_passive_check = 131072ull,
  log_info_message = 262144ull,
  log_host_notification = 524288ull,
  log_service_notification = 1048576ull,
  log_all = 2096895ull,

  dbg_functions = (1ull << 32),
  dbg_config = (2ull << 32),
  dbg_process = (4ull << 32),
  dbg_statusdata = (4ull << 32),
  dbg_retentiondata = (4ull << 32),
  dbg_events = (8ull << 32),
  dbg_checks = (16ull << 32),
  dbg_ipc = (16ull << 32),
  dbg_flapping = (16ull << 32),
  dbg_eventhandlers = (16ull << 32),
  dbg_perfdata = (16ull << 32),
  dbg_notifications = (32ull << 32),
  dbg_eventbroker = (64ull << 32),
  dbg_external_command = (128ull << 32),
  dbg_commands = (256ull << 32),
  dbg_downtime = (512ull << 32),
  dbg_comments = (1024ull << 32),
  dbg_macros = (2048ull << 32),
  dbg_all = (4095ull << 32),

  all = log_all | dbg_all
};

/**
 *  @enum object::e_verbose
 *  Logging verbosity.
 */
enum verbosity_level { basic = 0u, more = 1u, most = 2u };
}  // namespace logging

CCE_END()

#define logger(type, verbose)                                            \
  for (unsigned int __com_centreon_engine_logging_define_ui(0);          \
       !__com_centreon_engine_logging_define_ui &&                       \
       com::centreon::logging::engine::instance().is_log(type, verbose); \
       ++__com_centreon_engine_logging_define_ui)                        \
  com::centreon::logging::temp_logger(type, verbose)

#endif  // !CCE_LOGGING_LOGGER_HH
