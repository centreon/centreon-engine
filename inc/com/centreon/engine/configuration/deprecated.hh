/*
** Copyright 2015 Merethis
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

#ifndef CCE_CONFIGURATION_DEPRECATED_HH
#  define CCE_CONFIGURATION_DEPRECATED_HH

#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                    configuration {
  namespace                  deprecated {
    extern char const* const auto_reschedule_msg;
    extern char const* const check_acceptance_msg;
    extern char const* const check_result_path_msg;
    extern char const* const default_msg;
    extern char const* const embedded_perl_msg;
    extern char const* const engine_performance_msg;
    extern char const* const external_command_msg;
    extern char const* const groups_msg;
    extern char const* const interval_length_msg;
    extern char const* const log_msg;
    extern char const* const notification_msg;
    extern char const* const passive_host_checks_msg;
    extern char const* const perfdata_msg;
    extern char const* const resource_file_msg;
    extern char const* const retention_usage_msg;
    extern char const* const startup_script_msg;
    extern char const* const status_file_usage_msg;
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_DEPRECATED_HH
