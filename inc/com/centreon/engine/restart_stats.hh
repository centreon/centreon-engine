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
  std::time_t apply_start;
  std::chrono::duration<uint32_t, std::milli> objects_expansion;
  std::chrono::duration<uint32_t, std::milli> objects_difference;
  std::chrono::duration<uint32_t, std::milli> apply_config;
  std::chrono::duration<uint32_t, std::milli> apply_timeperiods;
  std::chrono::duration<uint32_t, std::milli> apply_connectors;
  std::chrono::duration<uint32_t, std::milli> apply_commands;
  std::chrono::duration<uint32_t, std::milli> apply_contacts;
  std::time_t apply_end;
};

CCE_END()
