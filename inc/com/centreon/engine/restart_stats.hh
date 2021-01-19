/*
 * Copyright 2020 Centreon (https://www.centreon.com/)
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
#include <chrono>

#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

struct restart_stats {
  std::chrono::system_clock::time_point apply_start;
  std::chrono::duration<long, std::milli> objects_expansion;
  std::chrono::duration<long, std::milli> objects_difference;
  std::chrono::duration<long, std::milli> apply_config;
  std::chrono::duration<long, std::milli> apply_timeperiods;
  std::chrono::duration<long, std::milli> apply_connectors;
  std::chrono::duration<long, std::milli> apply_commands;
  std::chrono::duration<long, std::milli> apply_contacts;
  std::chrono::duration<long, std::milli> apply_hosts;
  std::chrono::duration<long, std::milli> apply_services;
  std::chrono::duration<long, std::milli> resolve_hosts;
  std::chrono::duration<long, std::milli> resolve_services;
  std::chrono::duration<long, std::milli> apply_host_dependencies;
  std::chrono::duration<long, std::milli> resolve_host_dependencies;
  std::chrono::duration<long, std::milli> apply_service_dependencies;
  std::chrono::duration<long, std::milli> resolve_service_dependencies;
  std::chrono::duration<long, std::milli> apply_host_escalations;
  std::chrono::duration<long, std::milli> resolve_host_escalations;
  std::chrono::duration<long, std::milli> apply_service_escalations;
  std::chrono::duration<long, std::milli> resolve_service_escalations;
  std::chrono::duration<long, std::milli> apply_new_config;
  std::chrono::duration<long, std::milli> apply_scheduler;
  std::chrono::duration<long, std::milli> check_circular_paths;
  std::chrono::duration<long, std::milli> reload_modules;
  std::chrono::system_clock::time_point apply_end;
};

CCE_END()
