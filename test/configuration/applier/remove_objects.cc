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
#include "com/centreon/engine/configuration/hostgroup.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/servicegroup.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/configuration/timeperiod.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/io/file_entry.hh"
#include "com/centreon/shared_ptr.hh"
#include "find.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class check {
public:
  virtual                    ~check() throw () {}
  virtual std::string const& id() const throw () = 0;
  virtual void               id(configuration::object const& obj) = 0;
  virtual bool               find_into_config() = 0;
  virtual bool               find_into_applier() = 0;
  virtual std::string const& type_name() const throw () = 0;
};

template<
  typename T, std::string const& (T::*get_id)() const throw (),
  typename U, U const& (configuration::state::*get_config)() const throw (),
  typename V, V* (*find_object)(char const*)>
class chk_generic : public check {
public:
                     ~chk_generic() throw () {}
  std::string const& id() const throw () {
    return (_id);
  }

  void               id(configuration::object const& obj) {
    _type_name = obj.type_name();
    T const& real(*static_cast<T const*>(&obj));
    _id = (real.*get_id)();
  }

  bool               find_into_config() {
    U const& objects((config->*get_config)());
    for (typename U::const_iterator
           it(objects.begin()), end(objects.end());
         it != end;
         ++it)
      if (((**it).*get_id)() == _id)
        return (true);
    return (false);
  }

  bool               find_into_applier() {
    return (find_object(_id.c_str()));
  }

  std::string const& type_name() const throw () {
    return (_type_name);
  }

private:
  std::string        _id;
  std::string        _type_name;
};

template<typename T, T& (configuration::state::*get)() throw ()>
static void check_remove_objects(std::string const& path, check& chk) {
  configuration::state config;
  configuration::parser p;
  p.parse(path, config);
  configuration::applier::state::instance().apply(config);

  T& objects((config.*get)());
  while (objects.size()) {
    typename T::iterator it(objects.begin());
    chk.id(**it);
    objects.erase(it);

    configuration::applier::state::instance().apply(config);

    if (chk.find_into_config())
      throw (engine_error() << "remove " << chk.type_name()
             << " failed: " << chk.id() << " not remove from state "
             "configuration");
    if (chk.find_into_applier())
      throw (engine_error() << "remove " << chk.type_name()
             << " failed: " << chk.id() << " not remove from "
             "applier configuration");
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
  if (argc != 3)
    throw (engine_error() << "usage: " << argv[0] << " object_type file.cfg");

  std::string type(argv[1]);

  if (type == "command") {
    chk_generic<
      configuration::command,
      &configuration::command::command_name,
      configuration::set_command,
      &configuration::state::commands,
      command_struct,
      &find_command> chk_command;
    check_remove_objects<
      configuration::set_command,
      &configuration::state::commands>(argv[2], chk_command);
  }
  else if (type == "contact") {
    chk_generic<
      configuration::contact,
      &configuration::contact::contact_name,
      configuration::set_contact,
      &configuration::state::contacts,
      contact_struct,
      &find_contact> chk_contact;
    check_remove_objects<
      configuration::set_contact,
      &configuration::state::contacts>(argv[2], chk_contact);
  }
  else if (type == "contactgroup") {
    chk_generic<
      configuration::contactgroup,
      &configuration::contactgroup::contactgroup_name,
      configuration::set_contactgroup,
      &configuration::state::contactgroups,
      contactgroup_struct,
      &find_contactgroup> chk_contactgroup;
    check_remove_objects<
      configuration::set_contactgroup,
      &configuration::state::contactgroups>(argv[2], chk_contactgroup);
  }
  else if (type == "host") {
    chk_generic<
      configuration::host,
      &configuration::host::host_name,
      configuration::set_host,
      &configuration::state::hosts,
      host_struct,
      &find_host> chk_host;
    check_remove_objects<
      configuration::set_host,
      &configuration::state::hosts>(argv[2], chk_host);
  }
  else if (type == "hostgroup") {
    chk_generic<
      configuration::hostgroup,
      &configuration::hostgroup::hostgroup_name,
      configuration::set_hostgroup,
      &configuration::state::hostgroups,
      hostgroup_struct,
      &find_hostgroup> chk_hostgroup;
    check_remove_objects<
      configuration::set_hostgroup,
      &configuration::state::hostgroups>(argv[2], chk_hostgroup);
  }
  // else if (type == "service") {
  //   chk_generic<
  //     configuration::service, &configuration::service::,
  //     configuration::set_service, &configuration::state::services,
  //     service_struct, &find_service> chk_service;
  //   check_remove_objects<
  //     configuration::set_service,
  //     &configuration::state::services>(argv[2], chk_service);
  // }
  else if (type == "servicegroup") {
    chk_generic<
      configuration::servicegroup,
      &configuration::servicegroup::servicegroup_name,
      configuration::set_servicegroup,
      &configuration::state::servicegroups,
      servicegroup_struct,
      &find_servicegroup> chk_servicegroup;
    check_remove_objects<
      configuration::set_servicegroup,
      &configuration::state::servicegroups>(argv[2], chk_servicegroup);
  }
  else if (type == "timeperiod") {
    chk_generic<
      configuration::timeperiod,
      &configuration::timeperiod::timeperiod_name,
      configuration::set_timeperiod,
      &configuration::state::timeperiods,
      timeperiod_struct,
      &find_timeperiod> chk_timeperiod;
    check_remove_objects<
      configuration::set_timeperiod,
      &configuration::state::timeperiods>(argv[2], chk_timeperiod);
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
