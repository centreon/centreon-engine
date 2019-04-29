/*
** Copyright 2017 Centreon
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

#include <memory>
#include <gtest/gtest.h>
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ConfigContact : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

};

// When I create a configuration::contact with an empty name
// Then an exception is thrown.
TEST_F(ConfigContact, NewContactWithNoName) {
  configuration::contact ctct("");
  ASSERT_THROW(ctct.check_validity(), std::exception);
}

// Given a configuration::contact
// When I set one of its attribute
// Then I can retrieve it.
// When I copy the contact
// Then its copy is equal to it.
// When I change some of the parameters of one or the other
// Then comparaison between contacts works as expected.
// When I merge a second contact into a first contact,
// Then the final contact is filled as expected.
//TEST_F(ConfigContact, NewContactFromContact) {
//  configuration::contact ctct("test");
//  ASSERT_TRUE(ctct.parse("contact_name", "test_contact_name"));
//  ASSERT_EQ(ctct.contact_name(), "test_contact_name");
//  ASSERT_EQ(ctct.contact_name(), ctct.key());
//  ASSERT_TRUE(ctct.parse("alias", "test_alias"));
//  ASSERT_EQ(ctct.alias(), "test_alias");
//  ASSERT_TRUE(ctct.parse("contact_groups", "contact1,contact2"));
//  {
//    std::set<std::string> myset;
//    myset.insert("contact1");
//    myset.insert("contact2");
//    ASSERT_EQ(ctct.contactgroups(), myset);
//    ASSERT_TRUE(ctct.parse("contactgroups", "contact1,contact2,contact3"));
//    myset.insert("contact3");
//    ASSERT_EQ(ctct.contactgroups(), myset);
//  }
//  ASSERT_TRUE(ctct.parse("email", "test_email"));
//  ASSERT_EQ(ctct.email(), "test_email");
//  ASSERT_TRUE(ctct.parse("pager", "test_pager"));
//  ASSERT_EQ(ctct.pager(), "test_pager");
//  ASSERT_TRUE(ctct.parse("host_notification_period", "test_notif_period"));
//  ASSERT_EQ(ctct.host_notification_period(), "test_notif_period");
//  ASSERT_TRUE(ctct.parse("service_notification_period", "test_srv_notif_period"));
//  ASSERT_EQ(ctct.service_notification_period(), "test_srv_notif_period");
//  {
//    std::list<std::string> mylist;
//    mylist.push_back("command1");
//    mylist.push_back("command2");
//    ASSERT_TRUE(ctct.parse("host_notification_commands", "command1,command2"));
//    ASSERT_EQ(ctct.host_notification_commands(), mylist);
//    ASSERT_TRUE(ctct.parse("service_notification_commands", "command1,command2,command3"));
//    mylist.push_back("command3");
//    ASSERT_EQ(ctct.service_notification_commands(), mylist);
//  }
//
//  /* Host notification options */
//  /* 'k' is refused */
//  ASSERT_FALSE(ctct.parse("host_notification_options", "k,u,d"));
//
//  /* 'd,u,r,f,s,n,a' are accepted */
//  ASSERT_TRUE(ctct.parse("host_notification_options", "d,u,r,f,s,n,a"));
//
//  /* u and d are set and then verified */
//  ASSERT_TRUE(ctct.parse("host_notification_options", "u,d"));
//  ASSERT_EQ(
//    ctct.host_notification_options(),
//    configuration::host::unreachable | configuration::host::down);
//
//  /* Service notification options */
//  /* 'k' is refused */
//  ASSERT_FALSE(ctct.parse("service_notification_options", "k,w,c"));
//
//  /* 'w,c,r,f,s,n,a,u' are accepted */
//  ASSERT_TRUE(ctct.parse("service_notification_options", "w,c,r,f,s,n,a,u"));
//
//  /* w and c are set and then verified */
//  ASSERT_TRUE(ctct.parse("service_notification_options", "w,c"));
//  ASSERT_EQ(
//    ctct.service_notification_options(),
//    configuration::service::warning | configuration::service::critical);
//
//  ASSERT_TRUE(ctct.parse("host_notifications_enabled", "1"));
//  ASSERT_EQ(ctct.host_notifications_enabled(), true);
//  ASSERT_TRUE(ctct.parse("service_notifications_enabled", "0"));
//  ASSERT_EQ(ctct.service_notifications_enabled(), false);
//
//  ASSERT_TRUE(ctct.parse("can_submit_commands", "1"));
//  ASSERT_EQ(ctct.can_submit_commands(), true);
//
//  ASSERT_TRUE(ctct.parse("retain_status_information", "0"));
//  ASSERT_EQ(ctct.retain_status_information(), false);
//
//  ASSERT_TRUE(ctct.parse("retain_nonstatus_information", "1"));
//  ASSERT_EQ(ctct.retain_nonstatus_information(), true);
//  ASSERT_TRUE(ctct.parse("timezone", "my timezone"));
//  ASSERT_EQ(ctct.timezone(), "my timezone");
//
//  ASSERT_TRUE(ctct.parse("address1", "The moon"));
//  ASSERT_TRUE(ctct.parse("address2", "Jupiter"));
//  ASSERT_FALSE(ctct.parse("address7", "The Sun"));
//  ASSERT_EQ(ctct.address()[0], "The moon");
//  ASSERT_EQ(ctct.address()[1], "Jupiter");
//
//  configuration::contact ctct_copy(ctct);
//
//  ASSERT_EQ(ctct, ctct_copy);
//  ASSERT_FALSE(ctct != ctct_copy);
//
//  ctct.parse("contactgroups", "contact1,contact2,contact3,contact4");
//  ASSERT_TRUE(ctct_copy < ctct);
//
//  ctct_copy.parse("address2", "March");
//  ctct_copy.parse("address3", "Saturn");
//  ASSERT_TRUE(ctct < ctct_copy);
//  ctct.parse("contact_name", "test_contact_name1");
//  ASSERT_TRUE(ctct_copy < ctct);
//
//  ctct.merge(ctct_copy);
//  std::vector<std::string> vec;
//  vec.push_back("The moon");
//  vec.push_back("Jupiter");
//  vec.push_back("Saturn");
//  int i = 0;
//  for (std::vector<std::string>::const_iterator
//         cit(ctct.address().begin()),
//         cend(ctct.address().end()),
//         vit(vec.begin()),
//         vend(vec.end());
//       cit != cend && vit != vend;
//       ++cit, ++vit) {
//    ASSERT_TRUE(*cit == *vit);
//  }
//
//  ASSERT_TRUE(ctct.parse("_MyVariable", "Hello"));
//  map_customvar cv(ctct.customvariables());
//  ASSERT_EQ(cv["MyVariable"], "Hello");
//}

