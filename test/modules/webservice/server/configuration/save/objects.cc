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

#include <fstream>
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/webservice/configuration/save/objects.hh"
#include "com/centreon/engine/globals.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::modules;

/**
 *  Run add_host_comment test.
 */
static int check_save_objects(int argc, char** argv) {
  if (argc < 2)
    throw (engine_error()
           << "check_save_object failed: bad arguments.");

  std::string config_file(argv[2]);
  config->parse(config_file.c_str());
  if (read_object_config_data(
        config_file.c_str(),
        READ_ALL_OBJECT_DATA,
        true,
        false) == ERROR)
    throw (engine_error()
           << "check_save_object failed: bad configuration.");

  std::ifstream file(argv[3], std::ios::binary);
  std::string ref;
  while (file.good() && !file.eof()) {
    char buffer[1024];
    file.read(buffer, sizeof(buffer));
    ref.append(buffer, file.gcount());
  }

  std::string type(argv[1]);
  webservice::configuration::save::objects save;
  if (type == "command") {
    if (!command_list)
      throw (engine_error() << "invalid object: empty command_list");
    save.add_list(command_list);
  }
  else if (type == "contact") {
    if (!contact_list)
      throw (engine_error() << "invalid object: empty contact_list");
    save.add_list(contact_list);
  }
  else if (type == "contactgroup") {
    if (!contactgroup_list)
      throw (engine_error() << "invalid object: empty contactgroup_list");
    save.add_list(contactgroup_list);
  }
  else if (type == "host") {
    if (!host_list)
      throw (engine_error() << "invalid object: empty host_list");
    save.add_list(host_list);
  }
  else if (type == "hostescalation") {
    if (!hostescalation_list)
      throw (engine_error() << "invalid object: empty hostescalation_list");
    save.add_list(hostescalation_list);
  }
  else if (type == "hostdependency") {
    if (!hostdependency_list)
      throw (engine_error() << "invalid object: empty hostdependency_list");
    save.add_list(hostdependency_list);
  }
  else if (type == "hostgroup") {
    if (!hostgroup_list)
      throw (engine_error() << "invalid object: empty hostgroup_list");
    save.add_list(hostgroup_list);
  }
  else if (type == "service") {
    if (!service_list)
      throw (engine_error() << "invalid object: empty serivce_list");
    save.add_list(service_list);
  }
  else if (type == "servicedependency") {
    if (!servicedependency_list)
      throw (engine_error() << "invalid object: empty servicedependency_list");
    save.add_list(servicedependency_list);
  }
  else if (type == "serviceescalation") {
    if (!serviceescalation_list)
      throw (engine_error() << "invalid object: empty serviceescalation_list");
    save.add_list(serviceescalation_list);
  }
  else if (type == "servicegroup") {
    if (!servicegroup_list)
      throw (engine_error() << "invalid object: empty serivcegroup_list");
    save.add_list(servicegroup_list);
  }
  else if (type == "timeperiod") {
    if (!timeperiod_list)
      throw (engine_error() << "invalid object: empty timeperiod_list");
    save.add_list(timeperiod_list);
  }
  else
    throw (engine_error()
           << "check_save_object failed: bad argument type: " << type);

  if (save.to_string() != ref) {
    save.backup("./save_" + type + ".cfg");
    throw (engine_error()
           << "check_save_object failed: try with " << type);
  }
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_save_objects);
  return (utest.run());
}
