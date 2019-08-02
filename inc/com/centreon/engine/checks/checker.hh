/*
** Copyright 2011-2014 Merethis
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

#  include <queue>
#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/checks.hh"
#  include "com/centreon/engine/commands/command.hh"
#  include "com/centreon/engine/commands/command_listener.hh"
#  include "com/centreon/engine/commands/result.hh"
#  include "com/centreon/engine/host.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/service.hh"

CCE_BEGIN()

namespace                checks {
  /**
   *  @class checks checks.hh
   *  @brief Run object and reap the result.
   *
   *  Checker is a singleton to run host or service and reap the
   *  result.
   */
class checker : public commands::command_listener {
 public:
  static checker& instance();
  static void load();
  void push_check_result(check_result const* result);
  void push_check_result(check_result&& result);
  void reap();
  bool reaper_is_empty();
  void run(host* hst,
           int check_options = CHECK_OPTION_NONE,
           double latency = 0.0,
           bool scheduled_check = false,
           bool reschedule_check = false,
           int* time_is_valid = NULL,
           time_t* preferred_time = NULL);
  void run(service* svc,
           int check_options = CHECK_OPTION_NONE,
           double latency = 0.0,
           bool scheduled_check = false,
           bool reschedule_check = false,
           int* time_is_valid = NULL,
           time_t* preferred_time = NULL);
  void run_sync(host* hst,
                host::host_state* check_result_code,
                int check_options,
                int use_cached_result,
                unsigned long check_timestamp_horizon);
  static void unload();

 private:
  checker();
  checker(checker const& right);
  ~checker() throw() override;
  checker& operator=(checker const& right);
  void finished(commands::result const& res) throw() override;
  host::host_state _execute_sync(host* hst);

  std::unordered_map<uint64_t, check_result> _list_id;
  concurrency::mutex _mut_reap;
  std::queue<check_result> _to_reap;
  std::unordered_map<uint64_t, check_result> _to_reap_partial;
};
}

CCE_END()

#endif // !CCE_CHECKS_CHECKER_HH
