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

#ifndef CCE_COMMANDS_RAW_HH
# define CCE_COMMANDS_RAW_HH

# include <QObject>
# include <QString>
# include <QStringList>
# include <QProcess>
# include <QSharedPointer>
# include <QHash>
# include <sys/time.h>

# include "commands/command.hh"

namespace                            com {
  namespace                          centreon {
    namespace                        engine {
      namespace                      commands {
	/**
	 *  @class raw raw.hh
	 *  @brief Raw is a specific implementation of command.
	 *
	 *  Raw is a specific implementation of command.
	 */
	class                        raw : public command {
	  Q_OBJECT
	public:
	                             raw(QString const& name = "",
					 QString const& command_line = "");
	                             raw(raw const& right);
	                             ~raw() throw();

	  raw&                       operator=(raw const& right);

	  command*                   clone() const;

	  unsigned long              run(QString const& process_cmd,
					 nagios_macros const& macros,
					 int timeout);

	  void                       run(QString const& process_cmd,
					 nagios_macros const& macros,
					 int timeout,
					 result& res);

	public slots:
	  void                       process_error(QProcess::ProcessError error);
	  void                       process_finished(int exit_code,
						      QProcess::ExitStatus exit_status);
	  void                       process_started();
	  void                       process_stdout();
	  void                       process_stderr();

	private:
	  class                      eprocess : public QProcess {
	  public:
	                             eprocess(nagios_macros const& macros,
					      int timeout);
	                             ~eprocess();

	    int                      get_timeout();

	  protected:
	    void                     setupChildProcess();

	  private:
	    nagios_macros            _macros;
	    int                      _timeout;
	  };

	  struct                     proc_info {
	    QSharedPointer<eprocess> process;
	    timeval                  start_time;
	    timeval                  end_time;
	    QString                  stdout;
	    QString                  stderr;
	    unsigned long            cmd_id;
	  };

	  QHash<QObject*, proc_info> _processes;
	};
      }
    }
  }
}

#endif // !CCE_COMMANDS_RAW_HH
