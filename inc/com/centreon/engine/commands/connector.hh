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
 *  connector is a specific implementation of commands::command that
 *  is more efficient than a raw command. A connector is an external software
 *  that launches checks and returns when available their result. For example,
 *  we have a perl connector. Since, it centralizes various checks, it compiles
 *  them while reading and then they are are already compiled and can be
 *  executed faster.
 *
 *  A connector usually executes many checks, whereas a commands::raw works
 *  with only one check. This is a significant difference.
 *
 *  To exchange with scripts executed by the connector, we use specific commands
 *  to the connector. Those internal functions all begins with _recv_query_ or
 *  _send_query_.
 *
 *  The connector is connected to an external program named connector and also
 *  has an internal thread used to restart the connector if needed. So, we have
 *  several variables to control all that:
 *  * _is_running: the connector is up and running.
 *  * _try_to_restart: This boolean tells if the connector is stopped, if we
 *    should start it again. Usually it is the case but when we want to stop
 *    everything, we don't want.
 *  * _thread_running: The thread is running, so it is possible to restart the
 *    the connector.
 *  * _thread_action: This enum asks the thread to start/stop the connector.
 *  * _thread_cv: Here is the condition variable used in pair with
 *    _thread_action.
 *
 */
class connector : public command, public process_listener {
  struct query_info {
    std::string processed_cmd;
    timestamp start_time;
    uint32_t timeout;
    bool waiting_result;
  };

  enum thread_action { none, start, stop };
  std::condition_variable _cv_query;
  std::string _data_available;
  bool _is_running;
  std::unordered_map<uint64_t, std::shared_ptr<query_info> > _queries;
  bool _query_quit_ok;
  bool _query_version_ok;
  mutable std::mutex _lock;
  process _process;
  std::unordered_map<uint64_t, result> _results;
  bool _try_to_restart;

  std::thread _restart;
  bool _thread_running;
  thread_action _thread_action;
  std::mutex _thread_m;
  std::condition_variable _thread_cv;

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
  void _restart_loop();

 public:
  connector(std::string const& connector_name,
            std::string const& connector_line,
            command_listener* listener = nullptr);
  ~connector() noexcept override;
  connector(const connector&) = delete;
  connector& operator=(const connector&) = delete;
  uint64_t run(std::string const& processed_cmd,
               nagios_macros& macros,
               uint32_t timeout) override;
  void run(std::string const& processed_cmd,
           nagios_macros& macros,
           uint32_t timeout,
           result& res) override;
  void set_command_line(std::string const& command_line) override;
  void restart_connector();

  static connector_map connectors;
};
}  // namespace commands

CCE_END()

std::ostream& operator<<(std::ostream& os,
                         com::centreon::engine::commands::connector const& obj);

#endif  // !CCE_COMMANDS_CONNECTOR_HH
