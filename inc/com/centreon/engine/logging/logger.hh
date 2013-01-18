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
#  define CCE_LOGGING_LOGGER_HH

#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/logging/engine.hh"
#  include "com/centreon/logging/temp_logger.hh"

CCE_BEGIN()

namespace logging {
  /**
   *  @enum object::e_type
   *  Logging types.
   */
  enum    type_value {
    none                     = 0ul,

    log_runtime_error        = 1ul,
    log_runtime_warning      = 2ul,
    log_verification_error   = 4ul,
    log_verification_warning = 8ul,
    log_config_error         = 16ul,
    log_config_warning       = 32ul,
    log_process_info         = 64ul,
    log_event_handler        = 128ul,
    log_external_command     = 512ul,
    log_host_up              = 1024ul,
    log_host_down            = 2048ul,
    log_host_unreachable     = 4096ul,
    log_service_ok           = 8192ul,
    log_service_unknown      = 16384ul,
    log_service_warning      = 32768ul,
    log_service_critical     = 65536ul,
    log_passive_check        = 131072ul,
    log_info_message         = 262144ul,
    log_host_notification    = 524288ul,
    log_service_notification = 1048576ul,
    log_all                  = 2096895ul,

    dbg_functions            = 1ul << 32,
    dbg_config               = 2ul << 32,
    dbg_process              = 4ul << 32,
    dbg_statusdata           = 4ul << 32,
    dbg_retentiondata        = 4ul << 32,
    dbg_events               = 8ul << 32,
    dbg_checks               = 16ul << 32,
    dbg_ipc                  = 16ul << 32,
    dbg_flapping             = 16ul << 32,
    dbg_eventhandlers        = 16ul << 32,
    dbg_perfdata             = 16ul << 32,
    dbg_notifications        = 32ul << 32,
    dbg_eventbroker          = 64ul << 32,
    dbg_external_command     = 128ul << 32,
    dbg_commands             = 256ul << 32,
    dbg_downtime             = 512ul << 32,
    dbg_comments             = 1024ul << 32,
    dbg_macros               = 2048ul << 32,
    dbg_all                  = 4095ul << 32,

    all                      = log_all | dbg_all
  };

  /**
   *  @enum object::e_verbose
   *  Logging verbosity.
   */
  enum    verbosity_level {
    basic = 0u,
    more  = 1u,
    most  = 2u
  };
}

CCE_END()

#  define logger(type, verbose) \
  for (unsigned int __com_centreon_engine_logging_define_ui(0); \
       !__com_centreon_engine_logging_define_ui \
       && com::centreon::logging::engine::instance().is_log( \
               type, \
               verbose); \
       ++__com_centreon_engine_logging_define_ui) \
    com::centreon::logging::temp_logger(type, verbose)

#endif // !CCE_LOGGING_LOGGER_HH
