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

#ifndef CCE_CHECKS_CHECKER_HH
# define CCE_CHECKS_CHECKER_HH

# include <QSharedPointer>
# include <QObject>
# include <QQueue>
# include <QHash>
# include <QMutex>
# include "objects.hh"
# include "commands/command.hh"
# include "commands/result.hh"
# include "checks.hh"

namespace                                    com {
  namespace                                  centreon {
    namespace                                engine {
      namespace                              checks {
	/**
	 *  @class checks checks.hh
	 *  @brief Run object and reap the result.
	 *
	 *  Checker is a singleton to run host or service and reap the
	 *  result.
	 */
	class                                checker : public QObject {
	  Q_OBJECT
	public:
	  static checker&                    instance();
	  static void                        cleanup();

          bool                               reaper_is_empty();
	  void                               reap();

	  void                               push_check_result(check_result const& result);

	  void                               run(host* hst,
						 int check_options = CHECK_OPTION_NONE,
						 double latency = 0.0,
						 bool scheduled_check = false,
						 bool reschedule_check = false,
						 int* time_is_valid = NULL,
						 time_t* preferred_time = NULL);
	  void                               run(service* svc,
						 int check_options = CHECK_OPTION_NONE,
						 double latency = 0.0,
						 bool scheduled_check = false,
						 bool reschedule_check = false,
						 int* time_is_valid = NULL,
						 time_t* preferred_time = NULL);

	  void                               run_sync(host* hst,
						      int* check_result_code,
						      int check_options,
						      int use_cached_result,
						      unsigned long check_timestamp_horizon);

	private slots:
	  void                               _command_executed(commands::result const& res);

	private:
	                                     checker();
	                                     checker(checker const& right);
	                                     ~checker() throw();

	  checker&                           operator=(checker const& right);

	  int                                _execute_sync(host* hst);

	  QQueue<check_result>               _to_reap;
	  QHash<unsigned long, check_result> _list_id;
	  QMutex                             _mut_reap;
	  QMutex                             _mut_id;
          static checker*                    _instance;
	};
      }
    }
  }
}

#endif // !CCE_CHECKS_CHECKER_HH
