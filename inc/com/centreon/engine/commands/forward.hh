/*
 * Copyright 2011-2013,2015,2019-2021 Centreon (https://www.centreon.com/)
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
#ifndef CCE_COMMANDS_FORWARD_HH
#define CCE_COMMANDS_FORWARD_HH

#include <string>
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace commands {
/**
 *  @class forward commands/forward.hh
 *  @brief Command is a specific implementation of commands::command.
 *
 *  Command is a specific implementation of commands::command who
 *  provide forward, is more efficiente that a raw command.
 */
class forward : public command {
  std::shared_ptr<command> _s_command;
  command* _command;

 public:
  forward(const std::string& command_name,
          const std::string& command_line,
          std::shared_ptr<connector>& cmd);
  ~forward() noexcept = default;
  forward(const forward&) = delete;
  forward& operator=(const forward&) = delete;
  uint64_t run(const std::string& processed_cmd,
               nagios_macros& macros,
               uint32_t timeout) override;
  void run(const std::string& processed_cmd,
           nagios_macros& macros,
           uint32_t timeout,
           result& res) override;
};
}  // namespace commands

CCE_END()

#endif  // !CCE_COMMANDS_FORWARD_HH
