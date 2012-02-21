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

#ifndef CCE_COMMANDS_CONNECTOR_COMMAND_HH
# define CCE_COMMANDS_CONNECTOR_COMMAND_HH

# include <QTimer>
# include <QMutex>
# include <map>
# include "commands/command.hh"
# include "commands/basic_process.hh"
# include "commands/connector/execute_query.hh"

namespace                              com {
  namespace                            centreon {
    namespace                          engine {
      namespace                        commands {
	namespace                      connector {
	/**
	 *  @class command commands/connector/command.hh
	 *  @brief Command is a specific implementation of commands::command.
	 *
	 *  Command is a specific implementation of commands::command who
	 *  provide connector, is more efficiente that a raw command.
	 */
	  class                        command : public commands::command {
	    Q_OBJECT
	  public:
	                               command(std::string const& connector_name,
                                               std::string const& connector_line,
                                               std::string const& command_name,
					       std::string const& command_line);
	                               command(command const& right);
	                               ~command() throw();

	    command&                   operator=(command const& right);

	    commands::command*         clone() const;

	    unsigned long              run(std::string const& processed_cmd,
					   nagios_macros const& macros,
					   unsigned int timeout);

	    void                       run(std::string const& processed_cmd,
					   nagios_macros const& macros,
					   unsigned int timeout,
					   result& res);

	    std::string const&             get_connector_name() const throw();
	    std::string const&             get_connector_line() const throw();

	    unsigned long              get_max_check_for_restart() throw();
	    void                       set_max_check_for_restart(unsigned long value) throw();

	  signals:
	    void                       _wait_ending();
	    void                       _process_ending();

	  private slots:
	    void                       _timeout();
	    void                       _state_change(QProcess::ProcessState new_state);
	    void                       _ready_read();

	  private:
	    struct                     request_info {
	      QSharedPointer<request>  req;
	      QDateTime                start_time;
	      unsigned int             timeout;
	      bool                     waiting_result;
	    };

	    void                       _exit();
	    void                       _start();

	    void                       _req_quit_r(request* req);
	    void                       _req_version_r(request* req);
	    void                       _req_execute_r(request* req);
	    void                       _req_error_r(request* req);

	    std::string                 _read_data;
	    QMutex                     _mutex;
	    std::string                    _connector_name;
	    std::string                    _connector_line;
	    QSharedPointer<basic_process>
                                       _process;
	    std::map<unsigned long, request_info>
	                               _queries;
	    std::map<unsigned long, result>
	                               _results;
	    std::map<request::e_type, void (command::*)(request*)>
	                               _req_func;
	    unsigned long              _max_check_for_restart;
	    unsigned long              _nbr_check;
	    bool                       _is_good_version;
	    bool                       _active_timer;
	    bool                       _is_exiting;
            bool                       _state_already_change;

	    static const unsigned long DEFAULT_MAX_CHECK = 10000;
	  };
	}
      }
    }
  }
}

#endif // !CCE_COMMANDS_CONNECTOR_COMMAND_HH
