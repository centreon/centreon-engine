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

#ifndef TEST_ENGINE_HH
#define TEST_ENGINE_HH

#include <gtest/gtest.h>
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/hostescalation.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/serviceescalation.hh"

using namespace com::centreon::engine;

class TestEngine : public ::testing::Test {
 public:
  configuration::contact new_configuration_contact(std::string const& name,
                                                   bool full) const;
  configuration::host new_configuration_host(std::string const& hostname,
                                             std::string const& contacts);
  configuration::service new_configuration_service(
      std::string const& hostname,
      std::string const& description,
      std::string const& contacts);
  configuration::hostescalation new_configuration_hostescalation(
      std::string const& hostname,
      std::string const& contactgroup);
  configuration::serviceescalation new_configuration_serviceescalation(
      std::string const& hostname,
      std::string const& svc_desc,
      std::string const& contactgroup);
  configuration::serviceescalation new_configuration_serviceescalation_contact(
    std::string const& hostname,
    std::string const& svc_desc,
    std::string const& contact);
  configuration::contactgroup new_configuration_contactgroup(
      std::string const& name,
      std::string const& contactname);
};

#endif /* !TEST_ENGINE_HH */
