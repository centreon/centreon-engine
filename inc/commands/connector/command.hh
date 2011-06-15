/*
** Copyright 2011      Merethis
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
# include <QHash>
# include <QProcess>
# include <QStringList>
# include <QEventLoop>
# include "commands/command.hh"
# include "commands/connector/execute_query.hh"

namespace                             com {
  namespace                           centreon {
    namespace                         engine {
      namespace                       commands {
	namespace                     connector {
	  class                       command : public commands::command {
	    Q_OBJECT
	  public:
	                              command(QString const& name,
					      QString const& command_line,
					      QString const& process_command);
	                              command(command const& right);
	                              ~command() throw();

	    command&                  operator=(command const& right);

	    commands::command*        clone() const;

	    unsigned long             run(QString const& processed_cmd,
					  nagios_macros const& macros,
					  int timeout);

	    void                      run(QString const& processed_cmd,
					  nagios_macros const& macros,
					  int timeout,
					  result& res);

	    QString const&            get_process() const throw();

	    int                       get_max_check_for_restart() const throw();
	    void                      set_max_check_for_restart(int value) throw();

	    int                       get_max_time_for_restart() const throw();
	    void                      set_max_time_for_restart(int value) throw();

	  signals:
	    void                      _wait_ending();
	    void                      _process_ending();

	  private slots:
	    void                      _timeout();
	    void                      _state_change(QProcess::ProcessState new_state);
	    void                      _ready_read();

	  private:
	    struct                    request_info {
	      QSharedPointer<request> req;
	      QDateTime               start_time;
	      int                     timeout;
	      bool                    waiting_result;
	    };

	    void                      _start();
	    void                      _exit();

	    void                      _req_quit_r(request* req);
	    void                      _req_version_r(request* req);
	    void                      _req_execute_r(request* req);

	    QMutex                    _mutex;
	    QEventLoop                _loop;
	    QString                   _process_command;
	    QProcess                  _process;
	    QHash<unsigned long, request_info>
	                              _queries;
	    QHash<unsigned long, result>
	                              _results;
	    QHash<request::e_type, void (command::*)(request*)>
	                              _req_func;
	    int                       _max_check_for_restart;
	    int                       _max_time_for_restart;
	    bool                      _is_good_version;
	    bool                      _active_timer;
	  };
	}
      }
    }
  }
}

#endif // !CCE_COMMANDS_CONNECTOR_COMMAND_HH
