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

#ifndef CCE_COMMANDS_PROCESS_HH
# define CCE_COMMANDS_PROCESS_HH

# include <QString>
# include <QDateTime>
# include <QProcess>

# include "macros.hh"

namespace                  com {
  namespace                centreon {
    namespace              engine {
      namespace            commands {
	/**
	 *  @class process commnads/process.hh
	 *  @brief Process is a specific implementation of QProcess.
	 *
	 *  Process is a specific implementation of QProcess, and add
	 *  somme features like timeout.
	 */
	class              process : public QProcess {
	  Q_OBJECT
	public:
	                   process(nagios_macros const& macros = nagios_macros(),
				   unsigned int timeout = 0);
	                   process(process const& right);
	                   ~process() throw();

	  process&         operator=(process const& right);

	  QDateTime const& get_start_time() const throw();
	  QDateTime const& get_end_time() const throw();
	  unsigned int     get_executed_time() const throw();
	  QString const&   get_stderr() const throw();
	  QString const&   get_stdout() const throw();
	  int              get_exit_code() const throw();
	  bool             get_is_timeout() const throw();
	  bool             get_is_executed() const throw();
	  unsigned int     get_timeout() const throw();

	  void             wait();

	signals:
	  void             ended();

	protected:
	  void             setupChildProcess();

	private slots:
	  void             _finished(int exit_code,
				     QProcess::ExitStatus exit_status);
	  void             _started();
	  void             _timedout();
	  void             _error(QProcess::ProcessError error);

	private:
	  QDateTime        _start_time;
	  QDateTime        _end_time;
	  QString          _stderr;
	  QString          _stdout;
	  nagios_macros    _macros;
	  unsigned int     _executed_time;
	  unsigned int     _timeout;
	  int              _exit_code;
	  bool             _is_timeout;
	  bool             _is_executed;
	};
      }
    }
  }
}

#endif // !CCE_COMMANDS_PROCESS_HH
