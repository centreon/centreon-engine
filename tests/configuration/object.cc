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

#include <memory>
#include <gtest/gtest.h>
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/service.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

// Given a configuration object with a defined property
// And the configuration object has one parent template
// And the parent template has this property defined
// When resolve_template() is called
// Then the configuration object property has not changed
TEST(ConfigurationObjectResolveTemplateTest, NoInheritanceOverride) {
  configuration::service obj;
  obj.configuration::object::parse("contacts contact1");
  obj.configuration::object::parse("use parent");
  configuration::service parent;
  parent.parse("contacts", "contact2");
  std::unordered_map<std::string, std::shared_ptr<configuration::object> > templates;
  templates["parent"] = std::shared_ptr<configuration::object>(
    new configuration::service(parent));
  obj.resolve_template(templates);
  set_string result;
  result.insert("contact1");
  ASSERT_EQ(obj.contacts(), result);
}

// Given a configuration object with an undefined property
// And the configuration object has one parent template
// And the parent template has this property defined
// When resolve_template() is called
// Then the configuration object has the property of its parent
TEST(ConfigurationObjectResolveTemplateTest, BasicInheritance) {
  configuration::service obj;
  obj.configuration::object::parse("use parent");
  configuration::service parent;
  parent.configuration::object::parse("contacts contact1");
  std::unordered_map<std::string, std::shared_ptr<configuration::object> > templates;
  templates["parent"] = std::shared_ptr<configuration::object>(
    new configuration::service(parent));
  obj.resolve_template(templates);
  set_string result;
  result.insert("contact1");
  ASSERT_EQ(obj.contacts(), result);
}

// Given a configuration object with a defined property
// And the configuration object property has additive inheritance
// And the configuration object has one parent template
// And the parent template has this property defined
// When resolve_template() is called
// Then the configuration object has the two properties
TEST(ConfigurationObjectResolveTemplateTest, BasicAdditiveInheritance) {
  configuration::service obj;
  obj.configuration::object::parse("contacts +contact1");
  obj.configuration::object::parse("use parent");
  configuration::service parent;
  parent.configuration::object::parse("contacts contact2");
  std::unordered_map<std::string, std::shared_ptr<configuration::object> > templates;
  templates["parent"] = std::shared_ptr<configuration::object>(
    new configuration::service(parent));
  obj.resolve_template(templates);
  set_string result;
  result.insert("contact1");
  result.insert("contact2");
  ASSERT_EQ(obj.contacts(), result);
}

// Given a configuration object with a defined property
// And the configuration object property has additive inheritance
// And the configuration object has two parent templates
// And the parent templates have the property defined
// When resolve_template() is called
// Then the configuration object has its property
// And its first template's
TEST(ConfigurationObjectResolveTemplateTest,
    InheritanceFromFirstTemplateOnly) {
  configuration::service obj;
  obj.configuration::object::parse("contacts +contact1");
  obj.configuration::object::parse("use parent1,parent2");
  configuration::service parent1;
  parent1.configuration::object::parse("contacts contact2");
  configuration::service parent2;
  parent2.configuration::object::parse("contacts contact3");
  std::unordered_map<std::string, std::shared_ptr<configuration::object> > templates;
  templates["parent1"] = std::shared_ptr<configuration::object>(
    new configuration::service(parent1));
  templates["parent2"] = std::shared_ptr<configuration::object>(
    new configuration::service(parent2));
  obj.resolve_template(templates);
  set_string result;
  result.insert("contact1");
  result.insert("contact2");
  ASSERT_EQ(obj.contacts(), result);
}

// Given a configuration object with an undefined property
// And the configuration object has two parent templates
// And the parent templates have the property defined
// And all the properties have additive inheritance
// When resolve_template() is called
// Then the configuration object has its property
// And its first template's
TEST(ConfigurationObjectResolveTemplateTest,
    AdditiveInheritanceFromFirstTemplateOnly1) {
  configuration::service obj;
  obj.configuration::object::parse("use parent1,parent2");
  configuration::service parent1;
  parent1.configuration::object::parse("contacts +contact1");
  configuration::service parent2;
  parent2.configuration::object::parse("contacts +contact2");
  std::unordered_map<std::string, std::shared_ptr<configuration::object> > templates;
  templates["parent1"] = std::shared_ptr<configuration::object>(
    new configuration::service(parent1));
  templates["parent2"] = std::shared_ptr<configuration::object>(
    new configuration::service(parent2));
  obj.resolve_template(templates);
  set_string result;
  result.insert("contact1");
  ASSERT_EQ(obj.contacts(), result);
}

// Given a configuration object with a defined property
// And the configuration object has two parent templates
// And the parent templates have the property defined
// And all the properties have additive inheritance
// When resolve_template() is called
// Then the configuration object has its property
// And its first template's
TEST(ConfigurationObjectResolveTemplateTest,
    AdditiveInheritanceFromFirstTemplateOnly2) {
  configuration::service obj;
  obj.configuration::object::parse("contacts +contact1");
  obj.configuration::object::parse("use parent1,parent2");
  configuration::service parent1;
  parent1.configuration::object::parse("contacts +contact2");
  configuration::service parent2;
  parent2.configuration::object::parse("contacts +contact3");
  std::unordered_map<std::string, std::shared_ptr<configuration::object> > templates;
  templates["parent1"] = std::shared_ptr<configuration::object>(
    new configuration::service(parent1));
  templates["parent2"] = std::shared_ptr<configuration::object>(
    new configuration::service(parent2));
  obj.resolve_template(templates);
  set_string result;
  result.insert("contact1");
  result.insert("contact2");
  ASSERT_EQ(obj.contacts(), result);
}

// Given a configuration object with a defined property
// And the configuration object has three parent templates
// And the parent templates also have parents
// And the parent templates have the property defined
// And the grand-parent templates have the property defined
// And all the properties have additive inheritance
// When resolve_template() is called
// Then the configuration object has its property
// And its first template's
// And its first template's parent
TEST(ConfigurationObjectResolveTemplateTest, RecursiveAdditiveInheritance) {
  configuration::service obj;
  obj.configuration::object::parse("contacts +contact1");
  obj.configuration::object::parse("use parent1,parent2,parent3");
  configuration::service parents[6];
  for (int i(0); i < 3; ++i) {
    {
      std::ostringstream oss;
      oss << "contacts +contact" << i + 2;
      parents[i].configuration::object::parse(oss.str());
    }
    {
      std::ostringstream oss;
      oss << "use parent" << i + 4;
      parents[i].configuration::object::parse(oss.str());
    }
  }
  for (int i(3); i < 6; ++i) {
    std::ostringstream oss;
    oss << "contacts +contact" << i + 2;
    parents[i].configuration::object::parse(oss.str());
  }
  std::unordered_map<std::string, std::shared_ptr<configuration::object> > templates;
  for (int i(0); i < 6; ++i) {
    std::ostringstream oss;
    oss << "parent" << i + 1;
    templates[oss.str()] = std::shared_ptr<configuration::object>(
      new configuration::service(parents[i]));
  }
  obj.resolve_template(templates);
  set_string result;
  result.insert("contact1");
  result.insert("contact2");
  result.insert("contact5");
  ASSERT_EQ(obj.contacts(), result);
}
