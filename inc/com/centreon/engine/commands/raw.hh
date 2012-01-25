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

# include <string>
# include <QSharedPointer>
# include <QHash>
# include <QMutex>
# include <sys/time.h>

# include "commands/command.hh"
# include "commands/process.hh"

namespace                               com {
  namespace                             centreon {
    namespace                           engine {
      namespace                         commands {
	/**
	 *  @class raw raw.hh
	 *  @brief Raw is a specific implementation of command.
	 *
	 *  Raw is a specific implementation of command.
	 */
	class                           raw : public command {
	  Q_OBJECT
	public:
	                                raw(std::string const& name,
					    std::string const& command_line);
	                                raw(raw const& right);
                                        ~raw() throw();

	  raw&                          operator=(raw const& right);

	  command*                      clone() const;

	  unsigned long                 run(std::string const& process_cmd,
					    nagios_macros const& macros,
					    unsigned int timeout);

	  void                          run(std::string const& process_cmd,
					    nagios_macros const& macros,
					    unsigned int timeout,
					    result& res);

        public slots:
	  void                          raw_ended();

	private:
          static void                   _deletelater_process(process* obj);

	  struct                        process_info {
	    QSharedPointer<process>     proc;
	    unsigned long               cmd_id;
	  };

	  QHash<QObject*, process_info> _processes;
	  QMutex                        _mutex;

	signals:
	  void                          _empty_hash();
	};
      }
    }
  }
}

#endif // !CCE_COMMANDS_RAW_HH
