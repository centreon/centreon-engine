/*
** Copyright 2011-2012 Merethis
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
#  define CCE_COMMANDS_PROCESS_HH

#  include <QDateTime>
#  include <QString>
#  include "com/centreon/engine/commands/basic_process.hh"
#  include "com/centreon/engine/macros.hh"

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
        class              process : public basic_process {
          Q_OBJECT

        public:
                           process(
                             nagios_macros const& macros = nagios_macros(),
                             unsigned int timeout = 0);
                           process(process const& right);
                           ~process() throw ();
          process&         operator=(process const& right);
          QDateTime const& get_end_time() const throw ();
          unsigned int     get_executed_time() const throw ();
          int              get_exit_code() const throw ();
          bool             get_is_executed() const throw ();
          bool             get_is_timeout() const throw ();
          QDateTime const& get_start_time() const throw ();
          QString const&   get_stderr() const throw ();
          QString const&   get_stdout() const throw ();
          unsigned int     get_timeout() const throw ();
          void             wait();

        signals:
          void             process_ended();

        protected:
          void             setupChildProcess();

        private slots:
          void             _error(QProcess::ProcessError error);
          void             _finished(
                             int exit_code,
                             QProcess::ExitStatus exit_status);
          void             _internal_copy(process const& right);
          void             _started();
          void             _timedout();

        private:
          QDateTime        _end_time;
          unsigned int     _executed_time;
          int              _exit_code;
          bool             _is_executed;
          bool             _is_timeout;
          nagios_macros    _macros;
          QDateTime        _start_time;
          QString          _stderr;
          QString          _stdout;
          unsigned int     _timeout;
        };
      }
    }
  }
}

#endif // !CCE_COMMANDS_PROCESS_HH
