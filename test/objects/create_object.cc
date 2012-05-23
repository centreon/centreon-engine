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

#include <QString>
#include <QVector>
#include <string.h>
#include "com/centreon/engine/objects/timeperiod.hh"
#include "test/objects/create_object.hh"

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
  timeperiodexclusion* obj = new timeperiodexclusion;
  memset(obj, 0, sizeof(*obj));
  obj->timeperiod_name = my_strdup(qPrintable(QString("name_%1").arg(id)));
  obj->timeperiod_ptr = NULL;
  return (obj);
}

timeperiod* objects::create_timeperiod(unsigned int id) {
  QVector<QString> range;
  range.push_back("sunday             00:00-24:00");
  range.push_back("monday             00:00-09:00,17:00-24:00");
  range.push_back("tuesday            00:00-09:00,17:00-24:00");
  range.push_back("wednesday          00:00-09:00,17:00-24:00");
  range.push_back("thursday           00:00-09:00,17:00-24:00");
  range.push_back("friday             00:00-09:00,17:00-24:00");
  range.push_back("saturday           00:00-24:00");
  range.push_back("july 10 - 15 / 2   00:00-24:00");
  range.push_back("day 1 - 15 / 5     00:00-24:00");

  QVector<QString> exclude;
  if (id > 1)
    exclude.push_back(QString("name_%1").arg(id - 1));
  QString name = QString("name_%1").arg(id);
  add_timeperiod(name,
                 QString("alias_%1").arg(id),
                 range,
                 exclude);
  return (find_timeperiod(qPrintable(name)));
}

servicesmember* objects::create_servicesmember(unsigned int id, servicesmember** head) {
  servicesmember* member = new servicesmember;
  memset(member, 0, sizeof(*member));

  member->host_name = my_strdup(qPrintable(QString("name_%1").arg(id)));
  member->service_description = my_strdup(qPrintable(QString("description_%1").arg(id)));
  if (head != NULL) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

servicegroup* objects::create_servicegroup(unsigned int id) {
  return (add_servicegroup(qPrintable(QString("name_%1").arg(id)),
                           qPrintable(QString("alias_%1").arg(id)),
                           "notes",
                           "notes_url",
                           "action_url"));
}

serviceescalation* objects::create_serviceescalation(unsigned int id) {
  return (add_serviceescalation(qPrintable(QString("name_%1").arg(id)),
                                qPrintable(QString("description_%1").arg(id)),
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
  return (add_service_dependency(qPrintable(QString("dep_name_%1").arg(id)),
                                 qPrintable(QString("dep_description_%1").arg(id)),
                                 qPrintable(QString("name_%1").arg(id)),
                                 qPrintable(QString("description_%1").arg(id)),
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
  return (add_service(qPrintable(QString("name_%1").arg(id)),
                      qPrintable(QString("description_%1").arg(id)),
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
  hostsmember* member = new hostsmember;
  memset(member, 0, sizeof(*member));

  member->host_name = my_strdup(qPrintable(QString("name_%1").arg(id)));
  if (head != NULL) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

hostgroup* objects::create_hostgroup(unsigned int id) {
  return (add_hostgroup(qPrintable(QString("name_%1").arg(id)),
                        qPrintable(QString("alias_%1").arg(id)),
                        "notes",
                        "notes_url",
                        "action_url"));
}

hostescalation* objects::create_hostescalation(unsigned int id) {
  return (add_host_escalation(qPrintable(QString("name_%1").arg(id)),
                             5,
                             5,
                             60.0,
                             "timeperiod",
                             true,
                             true,
                             true));
}

hostdependency* objects::create_hostdependency(unsigned int id) {
  return (add_host_dependency(qPrintable(QString("dep_name_%1").arg(id)),
                              qPrintable(QString("name_%1").arg(id)),
                              EXECUTION_DEPENDENCY,
                              true,
                              true,
                              true,
                              true,
                              true,
                              "timeperiod"));
}

host* objects::create_host(unsigned int id) {
  return (add_host(qPrintable(QString("name_%1").arg(id)),
                   qPrintable(QString("display_%1").arg(id)),
                   qPrintable(QString("alias_%1").arg(id)),
                   qPrintable(QString("address_%1").arg(id)),
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
  customvariablesmember* member = new customvariablesmember;
  memset(member, 0, sizeof(*member));

  member->variable_name = my_strdup(qPrintable(QString("name_%1").arg(id)));
  member->variable_value = my_strdup(qPrintable(QString("value_%1").arg(id)));
  if (head != NULL) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

contactsmember* objects::create_contactsmember(unsigned int id, contactsmember** head) {
  contactsmember* member = new contactsmember;
  memset(member, 0, sizeof(*member));

  member->contact_name = my_strdup(qPrintable(QString("name_%1").arg(id)));
  if (head != NULL) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

contactgroupsmember* objects::create_contactgroupsmember(unsigned int id, contactgroupsmember** head) {
  contactgroupsmember* member = new contactgroupsmember;
  memset(member, 0, sizeof(*member));

  member->group_name = my_strdup(qPrintable(QString("name_%1").arg(id)));
  if (head != NULL) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

contactgroup* objects::create_contactgroup(unsigned int id) {
  return (add_contactgroup(qPrintable(QString("name_%1").arg(id)),
                           qPrintable(QString("alias_%1").arg(id))));
}

contact* objects::create_contact(unsigned int id) {
  return (add_contact(qPrintable(QString("name_%1").arg(id)),
                      qPrintable(QString("alias_%1").arg(id)),
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
  commandsmember* member = new commandsmember;
  memset(member, 0, sizeof(*member));

  member->cmd = my_strdup(qPrintable(QString("name_%1").arg(id)));
  if (head != NULL) {
    member->next = *head;
    *head = member;
  }
  return (member);
}

command* objects::create_command(unsigned int id) {
  return (add_command(qPrintable(QString("name_%1").arg(id)),
                      qPrintable(QString("alias_%1").arg(id))));
}
