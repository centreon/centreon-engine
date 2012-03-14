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

#ifndef CCE_CHECKS_CHECKER_HH
#  define CCE_CHECKS_CHECKER_HH

#  include <memory>
#  include <QHash>
#  include <QMutex>
#  include <QObject>
#  include <QQueue>
#  include <QSharedPointer>
#  include "com/centreon/engine/checks.hh"
#  include "com/centreon/engine/commands/command.hh"
#  include "com/centreon/engine/commands/result.hh"
#  include "com/centreon/engine/objects.hh"

namespace                      com {
  namespace                    centreon {
    namespace                  engine {
      namespace                checks {
        /**
         *  @class checks checks.hh
         *  @brief Run object and reap the result.
         *
         *  Checker is a singleton to run host or service and reap the
         *  result.
         */
        class                  checker : public QObject {
          Q_OBJECT

        public:
                               ~checker() throw ();
          static checker&      instance();
          static void          load();
          void                 push_check_result(
                                 check_result const& result);
          void                 reap();
          bool                 reaper_is_empty();
          void                 run(
                                 host* hst,
                                 int check_options = CHECK_OPTION_NONE,
                                 double latency = 0.0,
                                 bool scheduled_check = false,
                                 bool reschedule_check = false,
                                 int* time_is_valid = NULL,
                                 time_t* preferred_time = NULL);
          void                 run(
                                 service* svc,
                                 int check_options = CHECK_OPTION_NONE,
                                 double latency = 0.0,
                                 bool scheduled_check = false,
                                 bool reschedule_check = false,
                                 int* time_is_valid = NULL,
                                 time_t* preferred_time = NULL);
          void                 run_sync(
                                 host* hst,
                                 int* check_result_code,
                                 int check_options,
                                 int use_cached_result,
                                 unsigned long check_timestamp_horizon);
          static void          unload();

        private slots:
          void                 _command_executed(cce_commands_result const& res);

        private:
                               checker();
                               checker(checker const& right);
          checker&             operator=(checker const& right);
          int                  _execute_sync(host* hst);
          void                 _internal_copy(checker const& right);

          static std::auto_ptr<checker>
                               _instance;
          QHash<unsigned long, check_result>
                               _list_id;
          QMutex               _mut_id;
          QMutex               _mut_reap;
          QQueue<check_result> _to_reap;
        };
      }
    }
  }
}

#endif // !CCE_CHECKS_CHECKER_HH
