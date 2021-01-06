/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#ifndef CCE_COMMAND_MANAGER_HH
#define CCE_COMMAND_MANAGER_HH

#include <deque>
#include <future>
#include <mutex>

#include "com/centreon/engine/engine_impl.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()
class command_manager {
  std::mutex _queue_m;
  std::deque<std::packaged_task<int()>> _queue;
  command_manager();

 public:
  static command_manager& instance();
  void enqueue(std::packaged_task<int(void)>&& f);

  int process_passive_service_check(time_t check_time,
                                    const std::string& host_name,
                                    const std::string& svc_description,
                                    uint32_t return_code,
                                    const std::string& output);
  int process_passive_host_check(time_t check_time,
                                 const std::string& host_name,
                                 uint32_t return_code,
                                 const std::string& output);
  int get_stats(std::string const& request, Stats* response);
  int get_restart_stats(RestartStats* response);
  int get_services_stats(ServicesStats* sstats);
  int get_hosts_stats(HostsStats* hstats);
  void execute();
  static void schedule_and_propagate_downtime(host* h,
                                              time_t entry_time,
                                              char const* author,
                                              char const* comment,
                                              time_t start,
                                              time_t end,
                                              int fixed,
                                              unsigned long trigerred,
                                              unsigned long duration);
};

CCE_END()

#endif /* !CCE_STATISTICS_HH */
