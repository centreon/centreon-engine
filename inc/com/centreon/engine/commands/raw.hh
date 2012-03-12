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

#ifndef CCE_COMMANDS_RAW_HH
#  define CCE_COMMANDS_RAW_HH

#  include <QHash>
#  include <QMutex>
#  include <QSharedPointer>
#  include <QString>
#  include <sys/time.h>
#  include "com/centreon/engine/commands/command.hh"
#  include "com/centreon/engine/commands/process.hh"

namespace                 com {
  namespace               centreon {
    namespace             engine {
      namespace           commands {
        /**
         *  @class raw raw.hh
         *  @brief Raw is a specific implementation of command.
         *
         *  Raw is a specific implementation of command.
         */
        class             raw : public command {
          Q_OBJECT

        public:
                          raw(
                            QString const& name,
                            QString const& command_line);
                          raw(raw const& right);
                          ~raw() throw ();
          raw&            operator=(raw const& right);
          command*        clone() const;
          unsigned long   run(
                            QString const& process_cmd,
                            nagios_macros const& macros,
                            unsigned int timeout);
          void            run(
                            QString const& process_cmd,
                            nagios_macros const& macros,
                            unsigned int timeout,
                            result& res);

        signals:
          void            _empty_hash();

        public slots:
          void            raw_ended();

        private:
          struct                    process_info {
            unsigned long           cmd_id;
            QSharedPointer<process> proc;
          };

          static void     _deletelater_process(process* obj);

          QMutex          _mutex;
          QHash<QObject*, process_info>
                          _processes;
        };
      }
    }
  }
}

#endif // !CCE_COMMANDS_RAW_HH
