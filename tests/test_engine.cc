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

#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "test_engine.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::downtimes;

configuration::contact TestEngine::valid_contact_config() const {
  // Add command.
  {
    configuration::command cmd;
    cmd.parse("command_name", "cmd");
    cmd.parse("command_line", "true");
    configuration::applier::command aplyr;
    aplyr.add_object(cmd);
  }
  // Add timeperiod.
  {
    configuration::timeperiod tperiod;
    tperiod.parse("timeperiod_name", "24x7");
    tperiod.parse("alias", "24x7");
    tperiod.parse("monday", "00:00-24:00");
    tperiod.parse("tuesday", "00:00-24:00");
    tperiod.parse("wednesday", "00:00-24:00");
    tperiod.parse("thursday", "00:00-24:00");
    tperiod.parse("friday", "00:00-24:00");
    tperiod.parse("saterday", "00:00-24:00");
    tperiod.parse("sunday", "00:00-24:00");
    configuration::applier::timeperiod aplyr;
    aplyr.add_object(tperiod);
  }
  // Valid contact configuration
  // (will generate 0 warnings or 0 errors).
  configuration::contact ctct;
  ctct.parse("contact_name", "admin");
  ctct.parse("host_notification_period", "24x7");
  ctct.parse("service_notification_period", "24x7");
  ctct.parse("host_notification_commands", "cmd");
  ctct.parse("service_notification_commands", "cmd");
  ctct.parse("host_notification_options", "d,r,f,s");
  ctct.parse("host_notifications_enabled", "1");
  return ctct;
}

configuration::host TestEngine::new_configuration_host(
    std::string const& hostname,
    std::string const& contacts) {
  configuration::host hst;
  hst.parse("host_name", hostname.c_str());
  hst.parse("address", "127.0.0.1");
  hst.parse("_HOST_ID", "12");
  hst.parse("contacts", contacts.c_str());
  return hst;
}
