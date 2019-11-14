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

#ifndef CCE_COMMANDS_CONNECTOR_HH
#define CCE_COMMANDS_CONNECTOR_HH

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/process.hh"
#include "com/centreon/process_listener.hh"

CCE_BEGIN()
namespace commands {
class connector;
}
CCE_END()

typedef std::unordered_map<
    std::string,
    std::shared_ptr<com::centreon::engine::commands::connector> >
    connector_map;

CCE_BEGIN()

namespace commands {
/**
 *  @class connector commands/connector.hh
 *  @brief Command is a specific implementation of commands::command.
 *
 *  Command is a specific implementation of commands::command who
 *  provide connector, is more efficiente that a raw command.
 */
class connector : public command, public process_listener {
 public:
  connector(std::string const& connector_name,
            std::string const& connector_line,
            command_listener* listener = NULL);
  connector(connector const& right);
  ~connector() noexcept override;
  connector& operator=(connector const& right) = delete;
  commands::command* clone() const override;
  uint64_t run(std::string const& processed_cmd,
               nagios_macros& macros,
               uint32_t timeout) override;
  void run(std::string const& processed_cmd,
           nagios_macros& macros,
           uint32_t timeout,
           result& res) override;
  void set_command_line(std::string const& command_line) override;

  static connector_map connectors;

 private:
  //  class restart {
  //   public:
  //    restart(connector* c) : _c{c} {
  //      _thread = new std::thread(&restart::_run, this);
  //    }
  //    restart(restart const& right) = delete;
  //    restart& operator=(restart const& right) = delete;
  //    ~restart() noexcept;
  //    void wait();
  //    bool wait(uint32_t timeout);
  //
  //   private:
  //    void _run();
  //
  //    std::thread* _thread;
  //    connector* _c;
  //  };

  struct query_info {
    std::string processed_cmd;
    timestamp start_time;
    uint32_t timeout;
    bool waiting_result;
  };

  void data_is_available(process& p) noexcept override;
  void data_is_available_err(process& p) noexcept override;
  void finished(process& p) noexcept override;
  void _connector_close();
  void _connector_start();
  void _internal_copy(connector const& right);
  std::string const& _query_ending() const noexcept;
  void _recv_query_error(char const* data);
  void _recv_query_execute(char const* data);
  void _recv_query_quit(char const* data);
  void _recv_query_version(char const* data);
  void _send_query_execute(std::string const& cmdline,
                           uint64_t command_id,
                           timestamp const& start,
                           uint32_t timeout);
  void _send_query_quit();
  void _send_query_version();
  void _run_restart();

  std::condition_variable _cv_query;
  std::string _data_available;
  bool _is_running;
  std::unordered_map<uint64_t, std::shared_ptr<query_info> > _queries;
  bool _query_quit_ok;
  bool _query_version_ok;
  mutable std::mutex _lock;
  process _process;
  std::unordered_map<uint64_t, result> _results;
  std::thread* _restart;
  bool _try_to_restart;
};
}  // namespace commands

CCE_END()

std::ostream& operator<<(std::ostream& os,
                         com::centreon::engine::commands::connector const& obj);

#endif  // !CCE_COMMANDS_CONNECTOR_HH
