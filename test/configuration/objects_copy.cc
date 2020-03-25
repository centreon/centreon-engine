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

#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/hostdependency.hh"
#include "com/centreon/engine/configuration/hostescalation.hh"
#include "com/centreon/engine/configuration/hostextinfo.hh"
#include "com/centreon/engine/configuration/hostgroup.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/servicedependency.hh"
#include "com/centreon/engine/configuration/serviceescalation.hh"
#include "com/centreon/engine/configuration/serviceextinfo.hh"
#include "com/centreon/engine/configuration/servicegroup.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/configuration/timeperiod.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/io/file_entry.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

template <typename T,
          typename U,
          U const& (configuration::state::*ptr)() const throw()>
static void check_objects(std::string const& path) {
  configuration::state config;
  configuration::parser p;
  p.parse(path, config);

  T last;
  U const& objects((config.*ptr)());
  for (typename U::const_iterator it(objects.begin()), end(objects.end());
       it != end; ++it) {
    if (last == **it || !(last != **it)) {
      std::string const& type((*it)->type_name());
      throw(engine_error() << type
                           << " equal operator failed: "
                              "current "
                           << type << " is equal to the last " << type);
    }
    last = **it;
    if (!(last == **it) || last != **it) {
      std::string const& type((*it)->type_name());
      throw(engine_error() << type
                           << " not equal operator failed: "
                              "current "
                           << type << " is not equal to the copy");
    }
  }
}

/**
 *  Check the parsing argument.
 *
 *  @param[in] argc Unused.
 *  @param[in] argv Unused.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  if (argc != 2)
    throw(engine_error() << "usage: " << argv[0] << " object_name.cfg");

  io::file_entry fe(argv[1]);
  std::string type(fe.base_name());

  if (type == "command") {
    check_objects<configuration::command, configuration::set_command,
                  &configuration::state::commands>(fe.path());
  } else if (type == "contactgroup") {
    check_objects<configuration::contactgroup, configuration::set_contactgroup,
                  &configuration::state::contactgroups>(fe.path());
  } else if (type == "contact") {
    check_objects<configuration::contact, configuration::set_contact,
                  &configuration::state::contacts>(fe.path());
  } else if (type == "hostdependency") {
    check_objects<configuration::hostdependency,
                  configuration::set_hostdependency,
                  &configuration::state::hostdependencies>(fe.path());
  } else if (type == "hostescalation") {
    check_objects<configuration::hostescalation,
                  configuration::set_hostescalation,
                  &configuration::state::hostescalations>(fe.path());
  }
  // else if (type == "hostextinfo") {
  //   check_objects<
  //     configuration::hostextinfo,
  //     configuration::set_hostextinfo,
  //     &configuration::state::hostextinfos>(fe.path());
  // }
  else if (type == "hostgroup") {
    check_objects<configuration::hostgroup, configuration::set_hostgroup,
                  &configuration::state::hostgroups>(fe.path());
  } else if (type == "host") {
    check_objects<configuration::host, configuration::set_host,
                  &configuration::state::hosts>(fe.path());
  } else if (type == "servicedependency") {
    check_objects<configuration::servicedependency,
                  configuration::set_servicedependency,
                  &configuration::state::servicedependencies>(fe.path());
  } else if (type == "serviceescalation") {
    check_objects<configuration::serviceescalation,
                  configuration::set_serviceescalation,
                  &configuration::state::serviceescalations>(fe.path());
  }
  // else if (type == "serviceextinfo") {
  //   check_objects<
  //     configuration::serviceextinfo,
  //     configuration::set_serviceextinfo,
  //     &configuration::state::serviceextinfos>(fe.path());
  // }
  else if (type == "servicegroup") {
    check_objects<configuration::servicegroup, configuration::set_servicegroup,
                  &configuration::state::servicegroups>(fe.path());
  } else if (type == "service") {
    check_objects<configuration::service, configuration::set_service,
                  &configuration::state::services>(fe.path());
  } else if (type == "timeperiod") {
    check_objects<configuration::timeperiod, configuration::set_timeperiod,
                  &configuration::state::timeperiods>(fe.path());
  }
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
