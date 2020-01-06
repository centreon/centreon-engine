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

#include <com/centreon/engine/configuration/applier/state.hh>
#include <com/centreon/engine/checks/checker.hh>
#include "helper.hh"

using namespace com::centreon::engine;

extern configuration::state* config;

void init_config_state(void) {
  if (config == nullptr)
    config = new configuration::state;
}

void deinit_config_state(void) {
  delete config;
  config = nullptr;

  configuration::applier::state::instance().clear();
  checks::checker::instance().clear();
}