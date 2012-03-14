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

#ifndef CCE_COMMANDS_CONNECTOR_EXECUTE_RESPONSE_HH
#  define CCE_COMMANDS_CONNECTOR_EXECUTE_RESPONSE_HH

#  include <QDateTime>
#  include <QString>
#  include "com/centreon/engine/commands/connector/request.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace               commands {
  namespace             connector {
    /**
     *  @class execute_response com/centreon/engine/commands/connector/execute_response.hh
     *  @brief Execute response is the result of the command execution.
     *
     *  Execute reponse is a request who send the result of the command
     *  execution.
     */
    class               execute_response : public request {
    public:
                        execute_response(
                          unsigned long cmd_id = 0,
                          bool is_executed = false,
                          int exit_code = 0,
                          QDateTime const& end_time = QDateTime(),
                          QString const& stderr = "",
                          QString const& stdout = "");
                        execute_response(execute_response const& right);
                        ~execute_response() throw ();
      execute_response& operator=(execute_response const& right);
      bool              operator==(
                          execute_response const& right) const throw ();
      bool              operator!=(
                          execute_response const& right) const throw ();
      QByteArray        build();
      request*          clone() const;
      unsigned long     get_command_id() const throw();
      QDateTime const&  get_end_time() const throw();
      int               get_exit_code() const throw();
      bool              get_is_executed() const throw();
      QString const&    get_stderr() const throw();
      QString const&    get_stdout() const throw();
      void              restore(QByteArray const& data);

    private:
      void              _internal_copy(execute_response const& right);

      unsigned long     _cmd_id;
      QDateTime         _end_time;
      int               _exit_code;
      bool              _is_executed;
      QString           _stderr;
      QString           _stdout;
    };
  }
}

CCE_END()

#endif // !CCE_COMMANDS_CONNECTOR_EXECUTE_RESPONSE_HH
