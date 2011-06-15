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

#ifndef TEST_COMMANDS_WAIT_PROCESS_HH
# define TEST_COMMANDS_WAIT_PROCESS_HH

# include <QEventLoop>
# include "commands/command.hh"

namespace               com {
  namespace             centreon {
    namespace           engine {
      namespace         commands {
	class           wait_process : public QObject {
	  Q_OBJECT
	public:
	  wait_process(commands::command const& cmd)
	    : QObject() {
	    connect(&cmd, SIGNAL(command_executed(commands::result const&)),
		    this, SLOT(cmd_executed(commands::result const&)));
	  }

	  void          wait() const throw() {
	    QEventLoop loop;
	    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
	    loop.exec();
	  }

	  result const& get_result() const throw() {
	    return (_res);
	  }

	signals:
	  void          finished();

	public slots:
	  void          cmd_executed(commands::result const& res) {
	    _res = res;
	    emit finished();
	  }

	private:
	  result        _res;
	};
      }
    }
  }
}

#endif // !TEST_COMMANDS_WAIT_PROCESS_HH

