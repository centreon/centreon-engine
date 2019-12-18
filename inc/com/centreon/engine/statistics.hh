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

#ifndef CCE_STATISTICS_HH
#define CCE_STATISTICS_HH

#include <atomic>
#include <sys/types.h>
#include <unistd.h>
#include "com/centreon/engine/namespace.hh"

struct buffer_stats {
  uint32_t used;
  uint32_t high;
  uint32_t total;
};

CCE_BEGIN()
class statistics {
  statistics();
 public:
  static statistics& instance();
  pid_t get_pid() const noexcept;
  bool get_external_command_buffer_stats(buffer_stats& retval) const noexcept;
};

CCE_END()
#endif /* !CCE_STATISTICS_HH */
