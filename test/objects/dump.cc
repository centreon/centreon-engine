/*
** Copyright 2011-2013 Merethis
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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/objects.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check objects dump.
 *
 *  @param[in] argc Size of argv array.
 *  @param[in] argv Argumments array.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  if (argc != 2)
    throw(engine_error() << "usage: " << argv[0] << " object_name");

  std::ostringstream oss;
  std::string const type(argv[1]);

  if (type == "command") {
    command obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "commandsmember") {
    commandsmember obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "comment") {
    comment obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "contactgroup") {
    contactgroup obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "contactgroupsmember") {
    contactgroupsmember obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "contact") {
    contact obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "contactsmember") {
    contactsmember obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "customvariablesmember") {
    customvariablesmember obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "daterange") {
    daterange obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "downtime") {
    scheduled_downtime obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "hostdependency") {
    hostdependency obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "hostescalation") {
    hostescalation obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "hostgroup") {
    hostgroup obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "host") {
    host obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "hostsmember") {
    hostsmember obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "servicedependency") {
    servicedependency obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "serviceescalation") {
    serviceescalation obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "servicegroup") {
    servicegroup obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "service") {
    service obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "servicesmember") {
    servicesmember obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "timeperiod") {
    timeperiod obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "timeperiodexclusion") {
    timeperiodexclusion obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  } else if (type == "timerange") {
    timerange obj;
    memset(&obj, 0, sizeof(obj));
    oss << obj;
  }

  std::cout << oss.str();
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
