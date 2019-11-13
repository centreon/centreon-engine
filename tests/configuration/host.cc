/*
** Copyright 2016 Centreon
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

#include "com/centreon/engine/configuration/host.hh"
#include <gtest/gtest.h>
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine;

// Given a host configuration object
// When it is default constructed
// Then its acknowledgements timeout is set to 0
TEST(ConfigurationHostAcknowledgementTimeoutTest, DefaultConstruction) {
  configuration::host h;
  ASSERT_EQ(0, h.get_acknowledgement_timeout());
}

// Given a host configuration object
// When the acknowledgement timeout is set to a positive value
// Then the method returns true
// And the value is properly set
TEST(ConfigurationHostAcknowledgementTimeoutTest, SetToPositiveValue) {
  configuration::host h;
  ASSERT_TRUE(h.set_acknowledgement_timeout(42));
  ASSERT_EQ(42, h.get_acknowledgement_timeout());
}

// Given a host configuration object
// When the acknowledgement timeout is set to 0
// Then the method returns true
// And the value is properly set
TEST(ConfigurationHostAcknowledgementTimeoutTest, SetToZero) {
  configuration::host h;
  h.set_acknowledgement_timeout(42);
  ASSERT_TRUE(h.set_acknowledgement_timeout(0));
  ASSERT_EQ(0, h.get_acknowledgement_timeout());
}

// Given a host configuration object
// When the acknowledgement timeout is set to a negative value
// Then the method returns false
// And the original value is not changed
TEST(ConfigurationHostAcknowledgementTimeoutTest, SetToNegativeValue) {
  configuration::host h;
  h.set_acknowledgement_timeout(42);
  ASSERT_FALSE(h.set_acknowledgement_timeout(-36));
  ASSERT_EQ(42, h.get_acknowledgement_timeout());
}
