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

#include <algorithm>
#include "com/centreon/engine/configuration/applier/state.hh"
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
#include "find.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

//
// Check interface.
//
class check {
 public:
  virtual ~check() throw() {}
  virtual std::string const& id() const throw() = 0;
  virtual void id(configuration::object const& obj) = 0;
  virtual bool find_into_config() = 0;
  virtual bool find_into_applier() = 0;
  virtual std::string const& type_name() const throw() = 0;
};

//
// Check generic.
//
template <typename T,
          std::string const& (T::*get_id)() const throw(),
          typename U,
          U const& (configuration::state::*get_config)() const throw(),
          typename V,
          V* (*find_object)(char const*)>
class chk_generic : public check {
 public:
  ~chk_generic() throw() {}
  std::string const& id() const throw() { return ((_obj.*get_id)()); }

  void id(configuration::object const& obj) {
    _obj = *static_cast<T const*>(&obj);
  }

  bool find_into_config() {
    U const& objects((config->*get_config)());
    for (typename U::const_iterator it(objects.begin()), end(objects.end());
         it != end; ++it)
      if (((**it).*get_id)() == id())
        return (true);
    return (false);
  }

  bool find_into_applier() { return (find_object(id().c_str())); }

  std::string const& type_name() const throw() { return (_obj.type_name()); }

 private:
  T _obj;
};

//
// Check service
//
class chk_service : public check {
 public:
  ~chk_service() throw() {}
  std::string const& id() const throw() { return (_obj.service_description()); }

  void id(configuration::object const& obj) {
    _obj = *static_cast<configuration::service const*>(&obj);
    _all_hosts = _obj.hosts();
    for (list_string::const_iterator it(_obj.hostgroups().begin()),
         end(_obj.hostgroups().end());
         it != end; ++it) {
      hostgroup_struct* hg(find_hostgroup(it->c_str()));
      if (!hg)
        throw(engine_error() << "invalid service: hostgroup not found!");
      for (hostsmember_struct* m(hg->members); m; m = m->next)
        _all_hosts.push_back(m->host_name);
    }
  }

  bool find_into_config() {
    configuration::set_service const& objects(config->services());
    for (configuration::set_service::const_iterator it(objects.begin()),
         end(objects.end());
         it != end; ++it)
      if (**it == _obj)
        return (true);
    return (false);
  }

  bool find_into_applier() {
    for (list_string::const_iterator it(_all_hosts.begin()),
         end(_all_hosts.end());
         it != end; ++it)
      if (find_service(it->c_str(), _obj.service_description().c_str()))
        return (true);
    return (false);
  }

  std::string const& type_name() const throw() { return (_obj.type_name()); }

 private:
  list_string _all_hosts;
  configuration::service _obj;
};

//
// Check hostescalation
//
class chk_hostescalation : public check {
 public:
  ~chk_hostescalation() throw() {}
  std::string const& id() const throw() { return (_obj.type_name()); }

  void id(configuration::object const& obj) {
    _obj = *static_cast<configuration::hostescalation const*>(&obj);
    _all_hosts = _obj.hosts();
    for (list_string::const_iterator it(_obj.hostgroups().begin()),
         end(_obj.hostgroups().end());
         it != end; ++it) {
      hostgroup_struct* hg(find_hostgroup(it->c_str()));
      if (!hg)
        throw(engine_error() << "invalid escalation: hostgroup not found!");
      for (hostsmember_struct* m(hg->members); m; m = m->next)
        _all_hosts.push_back(m->host_name);
    }
  }

  bool find_into_config() {
    configuration::set_hostescalation const& objects(config->hostescalations());
    for (configuration::set_hostescalation::const_iterator it(objects.begin()),
         end(objects.end());
         it != end; ++it)
      if (**it == _obj)
        return (true);
    return (false);
  }

  bool find_into_applier() {
    umultimap<std::string, std::shared_ptr<hostescalation_struct> > const&
        escalations(
            configuration::applier::state::instance().hostescalations());
    for (list_string::const_iterator it(_all_hosts.begin()),
         end(_all_hosts.end());
         it != end; ++it) {
      for (umultimap<std::string,
                     std::shared_ptr<hostescalation_struct> >::const_iterator
               it_escalation(escalations.find(*it)),
           end(escalations.end());
           it_escalation != end; ++it_escalation) {
        hostescalation_struct const& escalation(*it_escalation->second);
        if (escalation.first_notification ==
                static_cast<int>(_obj.first_notification()) &&
            escalation.last_notification ==
                static_cast<int>(_obj.last_notification()) &&
            escalation.notification_interval == _obj.notification_interval() &&
            escalation.escalation_period == _obj.escalation_period() &&
            escalation.escalate_on_recovery ==
                static_cast<bool>(_obj.escalation_options() &
                                  configuration::hostescalation::recovery) &&
            escalation.escalate_on_down ==
                static_cast<bool>(_obj.escalation_options() &
                                  configuration::hostescalation::down) &&
            escalation.escalate_on_unreachable ==
                static_cast<bool>(_obj.escalation_options() &
                                  configuration::hostescalation::unreachable))
          return (true);
      }
    }
    return (false);
  }

  std::string const& type_name() const throw() { return (_obj.type_name()); }

 private:
  list_string _all_hosts;
  configuration::hostescalation _obj;
};

/**
 *  Template to deeply copy a collection.
 *
 *  @param[in]  from Source collection.
 *  @param[out] to   Destination collection.
 */
template <typename T, typename U>
static void deep_copy(T const& from, T& to) {
  to.clear();
  for (typename T::const_iterator it(from.begin()), end(from.end()); it != end;
       ++it)
    to.insert(new U(**it));
  return;
}

/**
 *  Deep copy of a configuration state.
 *
 *  @param[in] s State to copy.
 *
 *  @return A deep copy of s.
 */
static configuration::state deep_copy(configuration::state const& s) {
  // First copy construct the copy.
  configuration::state c(s);

  // Copy members.
  deep_copy<configuration::set_command, configuration::command>(s.commands(),
                                                                c.commands());
  deep_copy<configuration::set_connector, configuration::connector>(
      s.connectors(), c.connectors());
  deep_copy<configuration::set_contactgroup, configuration::contactgroup>(
      s.contactgroups(), c.contactgroups());
  deep_copy<configuration::set_contact, configuration::contact>(s.contacts(),
                                                                c.contacts());
  deep_copy<configuration::set_hostdependency, configuration::hostdependency>(
      s.hostdependencies(), c.hostdependencies());
  deep_copy<configuration::set_hostescalation, configuration::hostescalation>(
      s.hostescalations(), c.hostescalations());
  deep_copy<configuration::set_hostgroup, configuration::hostgroup>(
      s.hostgroups(), c.hostgroups());
  deep_copy<configuration::set_host, configuration::host>(s.hosts(), c.hosts());
  deep_copy<configuration::set_servicedependency,
            configuration::servicedependency>(s.servicedependencies(),
                                              c.servicedependencies());
  deep_copy<configuration::set_serviceescalation,
            configuration::serviceescalation>(s.serviceescalations(),
                                              c.serviceescalations());
  deep_copy<configuration::set_servicegroup, configuration::servicegroup>(
      s.servicegroups(), c.servicegroups());
  deep_copy<configuration::set_service, configuration::service>(s.services(),
                                                                c.services());
  deep_copy<configuration::set_timeperiod, configuration::timeperiod>(
      s.timeperiods(), c.timeperiods());

  // Clean groups.
  for (configuration::set_contactgroup::iterator it(c.contactgroups().begin()),
       end(c.contactgroups().end());
       it != end; ++it) {
    (*it)->set_resolved(false);
    (*it)->resolved_members().clear();
  }
  for (configuration::set_hostgroup::iterator it(c.hostgroups().begin()),
       end(c.hostgroups().end());
       it != end; ++it) {
    (*it)->set_resolved(false);
    (*it)->resolved_members().clear();
  }
  for (configuration::set_servicegroup::iterator it(c.servicegroups().begin()),
       end(c.servicegroups().end());
       it != end; ++it) {
    (*it)->set_resolved(false);
    (*it)->resolved_members().clear();
  }

  return (c);
}

/**
 *  Remove contact dependency for contact removal.
 *
 *  @param[in,out] cfg The configuration to update.
 *  @param[in]     hst Contact that will be removed.
 */
static void remove_dependency_for_contact(configuration::state& cfg,
                                          configuration::contact const& cntct) {
  for (configuration::set_contactgroup::iterator
           it(cfg.contactgroups().begin()),
       end(cfg.contactgroups().end());
       it != end; ++it)
    (*it)->members().remove(cntct.contact_name());
  return;
}

/**
 *  Remove contact dependency for remove contactgroup.
 *
 *  @param[in,out] config The configuration to update.
 *  @param[in]     grp    The group to remove.
 */
static void remove_dependency_for_contactgroup(
    configuration::state& config,
    configuration::contactgroup const& grp) {
  for (list_string::const_iterator m(grp.members().begin()),
       end(grp.members().end());
       m != end; ++m)
    for (configuration::set_contact::iterator it(config.contacts().begin()),
         end(config.contacts().end());
         it != end; ++it)
      if ((*it)->contact_name() == *m)
        (*it)->contactgroups().remove(grp.contactgroup_name());
  return;
}

/**
 *  Remove host dependency for host removal.
 *
 *  @param[in,out] cfg The configuration to update.
 *  @param[in]     hst Host that will be removed.
 */
static void remove_dependency_for_host(configuration::state& cfg,
                                       configuration::host const& hst) {
  for (configuration::set_host::iterator it(cfg.hosts().begin()),
       end(cfg.hosts().end());
       it != end; ++it)
    (*it)->parents().remove(hst.host_name());
  for (configuration::set_hostgroup::iterator it(cfg.hostgroups().begin()),
       end(cfg.hostgroups().end());
       it != end; ++it)
    (*it)->members().remove(hst.host_name());
  return;
}

/**
 *  Remove host dependency for remove hostgroup.
 *
 *  @param[in,out] config The configuration to update.
 *  @param[in]     grp    The group to remove.
 */
static void remove_dependency_for_hostgroup(
    configuration::state& config,
    configuration::hostgroup const& grp) {
  for (configuration::set_host::iterator it(config.hosts().begin()),
       end(config.hosts().end());
       it != end; ++it)
    (*it)->hostgroups().remove(grp.hostgroup_name());
  return;
}

/**
 *  Remove service dependency for service removal.
 *
 *  @param[in,out] cfg The configuration to update.
 *  @param[in]     svc Service that will be removed.
 */
static void remove_dependency_for_service(configuration::state& cfg,
                                          configuration::service const& svc) {
  for (configuration::set_servicegroup::iterator
           it(cfg.servicegroups().begin()),
       end(cfg.servicegroups().end());
       it != end; ++it)
    for (std::list<std::string>::iterator it_m((*it)->members().begin()),
         end_m((*it)->members().end());
         it_m != end_m;) {
      std::list<std::string>::iterator it_next(it_m);
      ++it_next;
      if (it_next == end_m)
        throw(engine_error() << "processing invalid service group member list");
      if ((*it_m == svc.hosts().front()) &&
          (*it_next == svc.service_description())) {
        std::list<std::string>::iterator to_delete(it_m);
        ++(++it_m);
        (*it)->members().erase(it_next);
        (*it)->members().erase(to_delete);
      } else
        ++(++it_m);
    }
  return;
}

/**
 *  Remove service dependency for remove servicegroup.
 *
 *  @param[in,out] config The configuration to update.
 *  @param[in]     grp    The group to remove.
 */
static void remove_dependency_for_servicegroup(
    configuration::state& config,
    configuration::servicegroup const& grp) {
  for (configuration::set_service::iterator it(config.services().begin()),
       end(config.services().end());
       it != end; ++it)
    (*it)->servicegroups().remove(grp.servicegroup_name());
  return;
}

/**
 *  Check if objects was remove correctly.
 *
 *  @param[in] cfg    Configuration to apply.
 *  @param[in] chk    The check to use to valid remove object.
 *  @param[in] remove A removal function executed every time an object
 *                    is deleted.
 */
template <typename T, typename U, T& (configuration::state::*get)() throw()>
static void check_remove_objects(configuration::state& cfg,
                                 check& chk,
                                 void (*remove)(configuration::state&,
                                                U const&) = NULL) {
  // Initial configuration application.
  configuration::applier::state::instance().apply(cfg);

  // Copy applied configuration state to handle expansion.
  cfg = deep_copy(*config);

  // Delete objects one by one.
  T& objects((cfg.*get)());
  while (objects.size()) {
    // Remove first object in the list.
    typename T::iterator it(objects.begin());
    if (remove)
      remove(cfg, **it);
    chk.id(**it);
    objects.erase(it);

    // Apply modified configuration.
    configuration::applier::state::instance().apply(cfg);

    // Checks.
    if (chk.find_into_config())
      throw(engine_error() << "remove " << chk.type_name()
                           << " failed: " << chk.id()
                           << " not remove from state "
                              "configuration");
    if (chk.find_into_applier())
      throw(engine_error() << "remove " << chk.type_name()
                           << " failed: " << chk.id()
                           << " not remove from "
                              "applier configuration");

    // Copy applied configuration.
    cfg = deep_copy(*config);
  }

  return;
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
    throw(engine_error() << "usage: " << argv[0] << " object_type file.cfg");

  std::string type(argv[1]);
  std::string path(argv[2]);

  configuration::state config;

  // tricks to bypass create log file.
  config.log_file("");
  ::config->log_file("");

  configuration::parser p;
  p.parse(path, config);

  if (type == "command") {
    chk_generic<configuration::command, &configuration::command::command_name,
                configuration::set_command, &configuration::state::commands,
                command_struct, &find_command>
        chk_command;
    check_remove_objects<configuration::set_command, configuration::command,
                         &configuration::state::commands>(config, chk_command);
  } else if (type == "contact") {
    chk_generic<configuration::contact, &configuration::contact::contact_name,
                configuration::set_contact, &configuration::state::contacts,
                contact_struct, &find_contact>
        chk_contact;
    check_remove_objects<configuration::set_contact, configuration::contact,
                         &configuration::state::contacts>(
        config, chk_contact, remove_dependency_for_contact);
  } else if (type == "contactgroup") {
    chk_generic<configuration::contactgroup,
                &configuration::contactgroup::contactgroup_name,
                configuration::set_contactgroup,
                &configuration::state::contactgroups, contactgroup_struct,
                &find_contactgroup>
        chk_contactgroup;
    check_remove_objects<configuration::set_contactgroup,
                         configuration::contactgroup,
                         &configuration::state::contactgroups>(
        config, chk_contactgroup, remove_dependency_for_contactgroup);
  } else if (type == "host") {
    chk_generic<configuration::host, &configuration::host::host_name,
                configuration::set_host, &configuration::state::hosts,
                host_struct, &find_host>
        chk_host;
    check_remove_objects<configuration::set_host, configuration::host,
                         &configuration::state::hosts>(
        config, chk_host, remove_dependency_for_host);
  } else if (type == "hostescalation") {
    chk_hostescalation chk_hostescalation;
    check_remove_objects<configuration::set_hostescalation,
                         configuration::hostescalation,
                         &configuration::state::hostescalations>(
        config, chk_hostescalation);
  } else if (type == "hostgroup") {
    chk_generic<configuration::hostgroup,
                &configuration::hostgroup::hostgroup_name,
                configuration::set_hostgroup, &configuration::state::hostgroups,
                hostgroup_struct, &find_hostgroup>
        chk_hostgroup;
    check_remove_objects<configuration::set_hostgroup, configuration::hostgroup,
                         &configuration::state::hostgroups>(
        config, chk_hostgroup, remove_dependency_for_hostgroup);
  } else if (type == "service") {
    config.servicegroups().clear();
    for (configuration::set_service::iterator it(config.services().begin()),
         end(config.services().end());
         it != end; ++it)
      (*it)->parse("servicegroups", "null");
    chk_service chk_service;
    check_remove_objects<configuration::set_service, configuration::service,
                         &configuration::state::services>(
        config, chk_service, remove_dependency_for_service);
  } else if (type == "servicegroup") {
    chk_generic<configuration::servicegroup,
                &configuration::servicegroup::servicegroup_name,
                configuration::set_servicegroup,
                &configuration::state::servicegroups, servicegroup_struct,
                &find_servicegroup>
        chk_servicegroup;
    check_remove_objects<configuration::set_servicegroup,
                         configuration::servicegroup,
                         &configuration::state::servicegroups>(
        config, chk_servicegroup, remove_dependency_for_servicegroup);
  } else if (type == "timeperiod") {
    chk_generic<
        configuration::timeperiod, &configuration::timeperiod::timeperiod_name,
        configuration::set_timeperiod, &configuration::state::timeperiods,
        timeperiod_struct, &find_timeperiod>
        chk_timeperiod;
    check_remove_objects<configuration::set_timeperiod,
                         configuration::timeperiod,
                         &configuration::state::timeperiods>(config,
                                                             chk_timeperiod);
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
