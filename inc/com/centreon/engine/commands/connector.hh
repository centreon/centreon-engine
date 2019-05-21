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
#  define CCE_COMMANDS_CONNECTOR_HH

#  include <memory>
#  include <string>
#  include "com/centreon/concurrency/condvar.hh"
#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/concurrency/thread.hh"
#  include "com/centreon/engine/commands/command.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/process.hh"
#  include "com/centreon/process_listener.hh"
#  include "com/centreon/unordered_hash.hh"

CCE_BEGIN()

namespace                commands {
  /**
   *  @class connector commands/connector.hh
   *  @brief Command is a specific implementation of commands::command.
   *
   *  Command is a specific implementation of commands::command who
   *  provide connector, is more efficiente that a raw command.
   */
  class                  connector
    : public command,
      public process_listener {
  public:
                         connector(
                           std::string const& connector_name,
                           std::string const& connector_line,
                           command_listener* listener = NULL);
                         connector(connector const& right);
                         ~connector() throw();
    connector&           operator=(connector const& right);
    commands::command*   clone() const;
    unsigned long        run(
                           std::string const& processed_cmd,
                           nagios_macros& macros,
                           unsigned int timeout);
    void                 run(
                           std::string const& processed_cmd,
                           nagios_macros& macros,
                           unsigned int timeout,
                           result& res);
    void                 set_command_line(
                           std::string const& command_line);

  private:
    class                restart : public concurrency::thread {
    public:
                         restart(connector* c);
                         ~restart() throw ();

    private:
                         restart(restart const& right);
      restart&           operator=(restart const& right);
      void               _run();

      connector*         _c;
    };

    struct               query_info {
      std::string        processed_cmd;
      timestamp          start_time;
      unsigned int       timeout;
      bool               waiting_result;
    };

    void                 data_is_available(process& p) throw ();
    void                 data_is_available_err(process& p) throw ();
    void                 finished(process& p) throw ();
    void                 _connector_close();
    void                 _connector_start();
    void                 _internal_copy(connector const& right);
    std::string const&   _query_ending() const throw ();
    void                 _recv_query_error(char const* data);
    void                 _recv_query_execute(char const* data);
    void                 _recv_query_quit(char const* data);
    void                 _recv_query_version(char const* data);
    void                 _send_query_execute(
                           std::string const& cmdline,
                           unsigned int command_id,
                           timestamp const& start,
                           unsigned int timeout);
    void                 _send_query_quit();
    void                 _send_query_version();

    concurrency::condvar _cv_query;
    std::string          _data_available;
    bool                 _is_running;
    umap<unsigned long, std::shared_ptr<query_info> >
                         _queries;
    bool                 _query_quit_ok;
    bool                 _query_version_ok;
    concurrency::mutex   _lock;
    process              _process;
    umap<unsigned long, result>
                         _results;
    restart              _restart;
    bool                 _try_to_restart;
  };
}

CCE_END()

std::ostream& operator<<(std::ostream& os, com::centreon::engine::commands::connector const& obj);


#endif // !CCE_COMMANDS_CONNECTOR_HH
