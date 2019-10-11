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

#ifndef CCE_COMMANDS_RAW_HH
#define CCE_COMMANDS_RAW_HH

#include <deque>
#include <mutex>
#include <string>
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/process.hh"
#include "com/centreon/process_listener.hh"

CCE_BEGIN()

namespace commands {
class environment;

/**
 *  @class raw raw.hh
 *  @brief Raw is a specific implementation of command.
 *
 *  Raw is a specific implementation of command.
 */
class raw : public command, public process_listener {
  std::mutex _lock;
  std::unordered_map<process*, uint64_t> _processes_busy;
  std::deque<process*> _processes_free;

  void data_is_available(process& p) noexcept override;
  void data_is_available_err(process& p) noexcept override;
  void finished(process& p) noexcept override;
  static void _build_argv_macro_environment(nagios_macros const& macros,
                                            environment& env);
  static void _build_contact_address_environment(nagios_macros const& macros,
                                                 environment& env);
  static void _build_custom_contact_macro_environment(nagios_macros& macros,
                                                      environment& env);
  static void _build_custom_host_macro_environment(nagios_macros& macros,
                                                   environment& env);
  static void _build_custom_service_macro_environment(nagios_macros& macros,
                                                      environment& env);
  static void _build_environment_macros(nagios_macros& macros,
                                        environment& env);
  static void _build_macrosx_environment(nagios_macros& macros,
                                         environment& env);
  process* _get_free_process();

 public:
  raw(std::string const& name,
      std::string const& command_line,
      command_listener* listener = NULL);
  raw(raw const& right);
  ~raw() noexcept override;
  raw& operator=(raw const& right);
  command* clone() const override;
  uint64_t run(std::string const& process_cmd,
                    nagios_macros& macros,
                    uint32_t timeout) override;
  void run(std::string const& process_cmd,
           nagios_macros& macros,
           uint32_t timeout,
           result& res) override;
};
}

CCE_END()

#endif  // !CCE_COMMANDS_RAW_HH
