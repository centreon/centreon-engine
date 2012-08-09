/*
** Copyright 2011-2012 Merethis
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

#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include "com/centreon/engine/objects/timeperiod.hh"
#include "test/objects/create_object.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine::objects;
using namespace test;

objectlist* objects::create_objectlist(unsigned int id) {
  objectlist* head = NULL;

  for (unsigned int i = 0; i < id + 1; ++i) {
    objectlist* obj = new objectlist;
    memset(obj, 0, sizeof(*obj));
    obj->object_ptr = reinterpret_cast<void*>(0x4242);
    obj->next = head;
    head = obj;
  }
  return (head);
}

timerange* objects::create_timerange(unsigned int id) {
  timerange* obj = new timerange;
  memset(obj, 0, sizeof(*obj));
  obj->range_start = id;
  obj->range_end = id;
  return (obj);
}

daterange* objects::create_daterange(unsigned int id) {
  daterange* obj = new daterange;
  memset(obj, 0, sizeof(*obj));
  obj->times = create_timerange(id);
  return (obj);
}

timeperiodexclusion* objects::create_timeperiodexclusion(unsigned int id) {
  timeperiodexclusion* obj(new timeperiodexclusion);
  memset(obj, 0, sizeof(*obj));
  std::ostringstream oss;
  oss << "name_" << id;
  obj->timeperiod_name = my_strdup(oss.str().c_str());
  obj->timeperiod_ptr = NULL;
  return (obj);
}

timeperiod* objects::create_timeperiod(unsigned int id) {
  std::vector<std::string> range;
  range.push_back("sunday             00:00-24:00");
  range.push_back("monday             00:00-09:00,17:00-24:00");
  range.push_back("tuesday            00:00-09:00,17:00-24:00");
  range.push_back("wednesday          00:00-09:00,17:00-24:00");
  range.push_back("thursday           00:00-09:00,17:00-24:00");
  range.push_back("friday             00:00-09:00,17:00-24:00");
  range.push_back("saturday           00:00-24:00");
  range.push_back("july 10 - 15 / 2   00:00-24:00");
  range.push_back("day 1 - 15 / 5     00:00-24:00");

  std::vector<std::string> exclude;
  if (id > 1) {
    std::ostringstream oss;
    oss << "name_" << id - 1;
    exclude.push_back(oss.str());
  }
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream alias;
  alias << "alias_" << id;
  add_timeperiod(
    name.str().c_str(),
    alias.str().c_str(),
    range,
    exclude);
  return (find_timeperiod(name.str().c_str()));
}

servicesmember* objects::create_servicesmember(unsigned int id, servicesmember** head) {
  servicesmember* member = new servicesmember;
  memset(member, 0, sizeof(*member));
  {
    std::ostringstream oss;
    oss << "name_" << id;
    member->host_name = my_strdup(oss.str().c_str());
  }
  {
    std::ostringstream oss;
    oss << "description_" << id;
    member->service_description = my_strdup(oss.str().c_str());
  }
  if (head != NULL) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

servicegroup* objects::create_servicegroup(unsigned int id) {
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream alias;
  alias << "alias_" << id;
  return (add_servicegroup(
            name.str().c_str(),
            alias.str().c_str(),
            "notes",
            "notes_url",
            "action_url"));
}

serviceescalation* objects::create_serviceescalation(unsigned int id) {
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream description;
  description << "description_" << id;
  return (add_service_escalation(
            name.str().c_str(),
            description.str().c_str(),
            1,
            1,
            1.0,
            "timeperiod",
            true,
            true,
            true,
            true));
}

servicedependency* objects::create_servicedependency(unsigned int id) {
  std::ostringstream dep_name;
  dep_name << "dep_name_" << id;
  std::ostringstream dep_description;
  dep_description << "dep_description_" << id;
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream description;
  description << "description_" << id;
  return (add_service_dependency(
            dep_name.str().c_str(),
            dep_description.str().c_str(),
            name.str().c_str(),
            description.str().c_str(),
            EXECUTION_DEPENDENCY,
            true,
            true,
            true,
            true,
            true,
            true,
            "timeperiod"));
}

service* objects::create_service(unsigned int id) {
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream description;
  description << "description_" << id;
  return (add_service(
            name.str().c_str(),
            description.str().c_str(),
            "display_name",
            "timeperiod",
            STATE_OK,
            1,
            true,
            true,
            5,
            5,
            5,
            60,
            "timeperiod",
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            "event_handler",
            true,
            "command",
            true,
            true,
            0.0,
            0.0,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            NULL,
            true,
            false,
            "notes",
            "notes_url",
            "action_url",
            "icon_image",
            "icon_image_alt",
            true,
            true,
            true));
}

hostsmember* objects::create_hostsmember(unsigned int id, hostsmember** head) {
  hostsmember* member(new hostsmember);
  memset(member, 0, sizeof(*member));
  std::ostringstream name;
  name << "name_" << id;
  member->host_name = my_strdup(name.str().c_str());
  if (head != NULL) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

hostgroup* objects::create_hostgroup(unsigned int id) {
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream alias;
  alias << "alias_" << id;
  return (add_hostgroup(
            name.str().c_str(),
            alias.str().c_str(),
            "notes",
            "notes_url",
            "action_url"));
}

hostescalation* objects::create_hostescalation(unsigned int id) {
  std::ostringstream name;
  name << "name_" << id;
  return (add_host_escalation(
            name.str().c_str(),
            5,
            5,
            60.0,
            "timeperiod",
            true,
            true,
            true));
}

hostdependency* objects::create_hostdependency(unsigned int id) {
  std::ostringstream dep_name;
  dep_name << "dep_name_" << id;
  std::ostringstream name;
  name << "name_" << id;
  return (add_host_dependency(
            dep_name.str().c_str(),
            name.str().c_str(),
            EXECUTION_DEPENDENCY,
            true,
            true,
            true,
            true,
            true,
            "timeperiod"));
}

host* objects::create_host(unsigned int id) {
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream display;
  display << "display_" << id;
  std::ostringstream alias;
  alias << "alias_" << id;
  std::ostringstream address;
  address << "address_" << id;
  return (add_host(
            name.str().c_str(),
            display.str().c_str(),
            alias.str().c_str(),
            address.str().c_str(),
            "timeperiod",
            0,
            5,
            5,
            100,
            true,
            true,
            true,
            true,
            true,
            10,
            60,
            "timeperiod",
            0,
            "command",
            true,
            true,
            "event_handler",
            true,
            true,
            0.0,
            0.0,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            NULL,
            false,
            false,
            "notes",
            "notes_url",
            "action_url",
            "icon_image",
            "icon_image_alt",
            "vrml_image",
            "statusmap_image",
            -1,
            -1,
            false,
            0,
            0,
            0,
            false,
            true,
            true,
            true,
            true));
}

customvariablesmember* objects::create_customvariablesmember(unsigned int id, customvariablesmember** head) {
  customvariablesmember* member(new customvariablesmember);
  memset(member, 0, sizeof(*member));
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream value;
  value << "value_" << id;
  member->variable_name = my_strdup(name.str().c_str());
  member->variable_value = my_strdup(value.str().c_str());
  if (head) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

contactsmember* objects::create_contactsmember(unsigned int id, contactsmember** head) {
  contactsmember* member(new contactsmember);
  memset(member, 0, sizeof(*member));
  std::ostringstream name;
  name << "name_" << id;
  member->contact_name = my_strdup(name.str().c_str());
  if (head) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

contactgroupsmember* objects::create_contactgroupsmember(unsigned int id, contactgroupsmember** head) {
  contactgroupsmember* member(new contactgroupsmember);
  memset(member, 0, sizeof(*member));
  std::ostringstream name;
  name << "name_" << id;
  member->group_name = my_strdup(name.str().c_str());
  if (head) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

contactgroup* objects::create_contactgroup(unsigned int id) {
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream alias;
  alias << "alias_" << id;
  return (add_contactgroup(
            name.str().c_str(),
            alias.str().c_str()));
}

contact* objects::create_contact(unsigned int id) {
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream alias;
  alias << "alias_" << id;
  return (add_contact(
            name.str().c_str(),
            alias.str().c_str(),
            "email",
            "pager",
            NULL, // XXX: (cntct.address.empty() ? NULL : &(*address)),
            "service_notification_period",
            "host_notification_period",
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true));
}

commandsmember* objects::create_commandsmember(unsigned int id, commandsmember** head) {
  commandsmember* member(new commandsmember);
  memset(member, 0, sizeof(*member));
  std::ostringstream name;
  name << "name_" << id;
  member->cmd = my_strdup(name.str().c_str());
  if (head) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

command* objects::create_command(unsigned int id) {
  std::ostringstream name;
  name << "name_" << id;
  std::ostringstream alias;
  alias << "alias_" << id;
  return (add_command(
            name.str().c_str(),
            alias.str().c_str()));
}
