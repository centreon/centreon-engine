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

#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/statistics.hh"

using namespace com::centreon::engine;

/**
 *  The default constructor
 */
statistics::statistics() {}

/**
 * @brief Just an accessor to the statistics instance.
 *
 * @return A reference to the instance.
 */
statistics& statistics::instance() {
  static statistics instance;
  return instance;
}

/**
 * @brief Returns the centengine pid.
 *
 * @return A pid_t
  */
pid_t statistics::get_pid() const noexcept {
  return getpid();
}

/**
 * @brief This function gets informations on the external commands buffer if
 * used. In that case, it also returns true, otherwise returns false.
 *
 * @param retval A reference to a buffer_stats struct.
 *
 * @return A boolean telling if the struct has been filled.
 */
bool statistics::get_external_command_buffer_stats(buffer_stats& retval) const
    noexcept {
  if (config->check_external_commands()) {
    pthread_mutex_lock(&external_command_buffer.buffer_lock);
    retval.used = external_command_buffer.items;
    retval.high = external_command_buffer.high;
    pthread_mutex_unlock(&external_command_buffer.buffer_lock);
    retval.total = config->external_command_buffer_slots();
    return true;
  } else
    return false;
}
