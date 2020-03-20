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
#define CCE_CHECKS_CHECKER_HH

#include <mutex>
#include <queue>
#include "com/centreon/engine/anomalydetection.hh"
#include "com/centreon/engine/check_result.hh"
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/commands/command_listener.hh"
#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/service.hh"

CCE_BEGIN()

namespace checks {
/**
 *  @class checks check_result.hh
 *  @brief Run object and reap the result.
 *
 *  Checker is a singleton to run host or service and reap the
 *  result.
 */
class checker : public commands::command_listener {
 public:
  static checker& instance();
  void clear() noexcept;
  void reap();
  void run_sync(host* hst,
                host::host_state* check_result_code,
                int check_options,
                int use_cached_result,
                unsigned long check_timestamp_horizon);
  void add_check_result(uint64_t id, check_result* result) noexcept;
  void add_check_result_to_reap(check_result* result) noexcept;

 private:
  checker();
  checker(checker const& right);
  ~checker() noexcept override;
  checker& operator=(checker const& right);
  void finished(commands::result const& res) noexcept override;
  host::host_state _execute_sync(host* hst);

  /* A mutex to protect access on _waiting_check_result and _to_reap_partial */
  std::mutex _mut_reap;
  /*
   * Here is the list of prepared check results but with a command being
   * running. When the command will be finished, each check result is get back
   * updated and moved to _to_reap_partial list. */
  std::unordered_map<uint64_t, check_result*> _waiting_check_result;
  /* This queue is filled during a cycle. When it is time to reap, its elements
   * are passed to _to_reap. It can then be filled in parallel during the
   * _to_reap treatment. */
  std::queue<check_result*> _to_reap_partial;
  /*
   * The list of check_results to reap: they contain data that have to be
   * translated to services/hosts. */
  std::queue<check_result*> _to_reap;
};
}  // namespace checks

CCE_END()

#endif  // !CCE_CHECKS_CHECKER_HH
