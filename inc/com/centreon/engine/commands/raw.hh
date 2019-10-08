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
#  define CCE_COMMANDS_RAW_HH

#  include <list>
#  include <string>
#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/commands/command.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/process.hh"
#  include "com/centreon/process_listener.hh"

CCE_BEGIN()

namespace               commands {
  class environment;

  /**
   *  @class raw raw.hh
   *  @brief Raw is a specific implementation of command.
   *
   *  Raw is a specific implementation of command.
   */
  class                 raw
    : public command,
      public process_listener {
  public:
                        raw(
                          std::string const& name,
                          std::string const& command_line,
                          command_listener* listener = NULL);
                        raw(raw const& right);
                        ~raw() throw () override;
    raw&                operator=(raw const& right);
    command*            clone() const override;
    unsigned long       run(
                          std::string const& process_cmd,
                          nagios_macros& macros,
                          unsigned int timeout) override;
    void                run(
                          std::string const& process_cmd,
                          nagios_macros& macros,
                          unsigned int timeout,
                          result& res) override;

  private:
    void                data_is_available(process& p) throw () override;
    void                data_is_available_err(process& p) throw () override;
    void                finished(process& p) throw () override;
    static void         _build_argv_macro_environment(
                          nagios_macros const& macros,
                          environment& env);
    static void         _build_contact_address_environment(
                          nagios_macros const& macros,
                          environment& env);
    static void         _build_custom_contact_macro_environment(
                          nagios_macros& macros,
                          environment& env);
    static void         _build_custom_host_macro_environment(
                          nagios_macros& macros,
                          environment& env);
    static void         _build_custom_service_macro_environment(
                          nagios_macros& macros,
                          environment& env);
    static void         _build_environment_macros(
                          nagios_macros& macros,
                          environment& env);
    static void         _build_macrosx_environment(
                          nagios_macros& macros,
                          environment& env);
    process*            _get_free_process();

    concurrency::mutex  _lock;
    std::unordered_map<process*, unsigned long>
                        _processes_busy;
    std::list<process*> _processes_free;
  };
}

CCE_END()

#endif // !CCE_COMMANDS_RAW_HH
