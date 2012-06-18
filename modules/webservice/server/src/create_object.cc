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

#include <algorithm>
#include <map>
#include <QRegExp>
#include <QScopedArrayPointer>
#include <vector>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/modules/webservice/schedule_object.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostdependency.hh"
#include "com/centreon/engine/objects/hostescalation.hh"
#include "com/centreon/engine/objects/hostgroup.hh"
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/objects/servicedependency.hh"
#include "com/centreon/engine/objects/serviceescalation.hh"
#include "com/centreon/engine/objects/servicegroup.hh"
#include "com/centreon/engine/objects/timeperiod.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;

/**
 *  Parse and return object options.
 *
 *  @param[in] opt         The option to parse.
 *  @param[in] pattern     The option list.
 *  @param[in] default_opt The default option.
 *
 *  @return The options list.
 */
std::map<char, bool> webservice::get_options(
                                   std::string const* opt,
                                   std::string const& pattern,
                                   char const* default_opt) {
  std::map<char, bool> res;
  QString _opt(opt ? opt->c_str() : default_opt);
  _opt.toLower().trimmed();
  if (_opt.contains(QRegExp(
                      QString("[^") + pattern.c_str() + "na, ]",
                      Qt::CaseInsensitive)))
    return (res);

  for (std::string::const_iterator
         it(pattern.begin()),
         end(pattern.end());
       it != end;
       ++it)
    if (_opt == "n")
      res[*it] = false;
    else if (_opt == "a")
      res[*it] = true;
    else
      res[*it] = (_opt.indexOf(*it) != -1);
  return (res);
}

/**
 *  Find services by name and description to create a table of it.
 *
 *  @param[in]  services The object to find.
 *
 *  @return The service's table, stop when the first service are not found.
 */
std::vector<service*> webservice::_find(
                                    std::vector<std::string> const& objs) {
  std::vector<service*> res;
  if (objs.size() % 2)
    return (res);

  res.reserve(objs.size() / 2);
  for (std::vector<std::string>::const_iterator it = objs.begin(),
	 end = objs.end();
       it != end;
       ++it) {
    // check if the object exist..
    char const* host_name = it->c_str();
    char const* service_description = (++it)->c_str();
    void* obj = find_service(host_name, service_description);
    if (obj == NULL)
      return (res);
    res.push_back(static_cast<service*>(obj));
  }
  return (res);
}

template<class T, class U>
static void _extract_object_from_objectgroup(
              std::vector<T*> const& groups,
              std::vector<U*>& objects) {
  for (typename std::vector<T*>::const_iterator
         it(groups.begin()),
         end(groups.end());
       it != end;
       ++it) {
    for (hostsmember* member = (*it)->members;
         member != NULL;
         member = member->next) {
      objects.push_back(member->host_ptr);
    }
  }
  qSort(objects.begin(), objects.end());
  std::unique(objects.begin(), objects.end());
}

/**
 *  Create a new host dependency into the engine.
 *
 *  @param[in] hostdependency The struct with all information to create new host dependency.
 */
void webservice::create_host_dependency(ns1__hostDependencyType const& hstdependency) {
  // check all arguments and set default option for optional options.
  if (!hstdependency.executionFailureCriteria
      && !hstdependency.notificationFailureCriteria)
    throw (engine_error() << "hostdependency have no notification failure "
           << "criteria and no execution failure criteria define.");

  if (hstdependency.hostgroupsName.empty() == true
      && hstdependency.hostsName.empty() == true)
    throw (engine_error() << "hostdependency have no hosts and no host groups define.");
  if (hstdependency.dependentHostgroupsName.empty() == true
      && hstdependency.dependentHostsName.empty() == true)
    throw (engine_error() << "hostdependency have no dependency hosts "
           << "and no dependency host groups define.");

  std::map<char, bool> execution_opt = get_options(hstdependency.executionFailureCriteria, "odup", "n");
  if (execution_opt.empty())
    throw (engine_error() << "hostdependency invalid execution failure criteria.");

  std::map<char, bool> notif_opt = get_options(hstdependency.notificationFailureCriteria, "odup", "n");
  if (notif_opt.empty())
    throw (engine_error() << "hostdependency invalid notification failure criteria.");

  char const* dependency_period = NULL;
  timeperiod* dependency_period_ptr = NULL;
  if (hstdependency.dependencyPeriod != NULL) {
    dependency_period = hstdependency.dependencyPeriod->c_str();
    if ((dependency_period_ptr = find_timeperiod(dependency_period)) == NULL)
      throw (engine_error() << "hostdependency invalid dependency period");
  }

  std::vector<host*> hstdep_hosts(
                       _find<host>(
                         hstdependency.hostsName,
                         (void* (*)(char const*))&find_host));
  if (hstdependency.hostsName.size() != hstdep_hosts.size())
    throw (engine_error() << "hostdependency invalid hosts name");

  std::vector<host*> hstdep_dependent_hosts(
                       _find<host>(
                         hstdependency.dependentHostsName,
                         (void* (*)(char const*))&find_host));
  if (hstdependency.dependentHostsName.size()
      != hstdep_dependent_hosts.size())
    throw (engine_error()
           << "hostdependency invalid dependent hosts name");

  std::vector<hostgroup*> hstdep_hostgroups(
                            _find<hostgroup>(
                              hstdependency.hostgroupsName,
                              (void* (*)(char const*))&find_hostgroup));
  if (hstdependency.hostgroupsName.size() != hstdep_hostgroups.size())
    throw (engine_error() << "hostdependency invalid host groups name");

  std::vector<hostgroup*> hstdep_dependent_hostgroups(
                            _find<hostgroup>(
                              hstdependency.dependentHostgroupsName,
                              (void* (*)(char const*))&find_hostgroup));
  if (hstdependency.dependentHostgroupsName.size()
      != hstdep_dependent_hostgroups.size())
    throw (engine_error()
           << "hostdependency invalid dependent host groups name");

  _extract_object_from_objectgroup(hstdep_dependent_hostgroups, hstdep_dependent_hosts);
  _extract_object_from_objectgroup(hstdep_hostgroups, hstdep_hosts);

  bool inherits_parent = (hstdependency.inheritsParent
                          ? *hstdependency.inheritsParent : true);

  for (std::vector<host*>::const_iterator
         it_dep(hstdep_dependent_hosts.begin()),
         end_dep(hstdep_dependent_hosts.end());
       it_dep != end_dep;
       ++it_dep) {
    for (std::vector<host*>::const_iterator
           it(hstdep_hosts.begin()),
           end(hstdep_hosts.end());
         it != end;
         ++it) {
      if (hstdependency.executionFailureCriteria != NULL) {
        hostdependency* new_hstdependency(
                          add_host_dependency(
                            (*it_dep)->name,
                            (*it)->name,
                            EXECUTION_DEPENDENCY,
                            inherits_parent,
                            execution_opt['o'],
                            execution_opt['d'],
                            execution_opt['u'],
                            execution_opt['p'],
                            dependency_period));

        try {
          objects::link(new_hstdependency, dependency_period_ptr);
        }
        catch (std::exception const& e) {
          (void)e;
          objects::release(new_hstdependency);
          throw;
        }
      }

      if (hstdependency.notificationFailureCriteria != NULL) {
        hostdependency* new_hstdependency =
          add_host_dependency((*it_dep)->name,
                              (*it)->name,
                              NOTIFICATION_DEPENDENCY,
                              inherits_parent,
                              notif_opt['o'],
                              notif_opt['d'],
                              notif_opt['u'],
                              notif_opt['p'],
                              dependency_period);

        try {
          objects::link(new_hstdependency, dependency_period_ptr);
        }
        catch (std::exception const& e) {
          (void)e;
          objects::release(new_hstdependency);
          throw;
        }
      }
    }
  }
}

/**
 *  Create a new host escalation into the engine.
 *
 *  @param[in] hostescalation The struct with all information to create new host escalation.
 */
void webservice::create_host_escalation(ns1__hostEscalationType const& hstescalation) {
  // check all arguments and set default option for optional options.
  if (hstescalation.contacts.empty() == true && hstescalation.contactGroups.empty() == true)
    throw (engine_error() << "hostescalation no contact and no contact groups are defined.");

  if (hstescalation.hostsName.empty() == true && hstescalation.hostgroupsName.empty() == true)
    throw (engine_error() << "hostescalation no host and no host groups are defined.");

  std::map<char, bool> escalation_opt = get_options(hstescalation.escalationOptions, "dur" , "n");
  if (escalation_opt.empty())
    throw (engine_error() << "hostescalation invalid escalation options.");

  char const* escalation_period = NULL;
  timeperiod* escalation_period_ptr = NULL;
  if (hstescalation.escalationPeriod != NULL) {
    escalation_period = hstescalation.escalationPeriod->c_str();
    if ((escalation_period_ptr = find_timeperiod(escalation_period)) == NULL)
      throw (engine_error() << "hostescalation invalid check period");
  }

  std::vector<contact*> hstesc_contacts(
                          _find<contact>(
                            hstescalation.contacts,
                            (void* (*)(char const*))&find_contact));
  if (hstescalation.contacts.size() != hstesc_contacts.size())
    throw (engine_error() << "hostescalation invalid contacts");

  std::vector<contactgroup*> hstesc_contactgroups(
                               _find<contactgroup>(
                                 hstescalation.contactGroups,
                                 (void* (*)(char const*))&find_contactgroup));
  if (hstescalation.contactGroups.size() != hstesc_contactgroups.size())
    throw (engine_error() << "hostescalation invalid contact groups");

  std::vector<hostgroup*> hstesc_hostgroups(
                            _find<hostgroup>(
                              hstescalation.hostgroupsName,
                              (void* (*)(char const*))&find_hostgroup));
  if (hstescalation.hostgroupsName.size() != hstesc_hostgroups.size())
    throw (engine_error() << "hostescalation invalid host groups");

  std::vector<host*> hstesc_hosts(
                       _find<host>(
                         hstescalation.hostsName,
                         (void* (*)(char const*))&find_host));
  if (hstescalation.hostsName.size() != hstesc_hosts.size())
    throw (engine_error() << "hostescalation invalid hosts");

  _extract_object_from_objectgroup(hstesc_hostgroups, hstesc_hosts);

  for (std::vector<host*>::const_iterator
         it(hstesc_hosts.begin()),
         end(hstesc_hosts.end());
       it != end;
       ++it) {
    hostescalation* new_hstescalation =
      add_host_escalation((*it)->name,
                         hstescalation.firstNotification,
                         hstescalation.lastNotification,
                         hstescalation.notificationInterval,
                         escalation_period,
                         escalation_opt['d'],
                         escalation_opt['u'],
                         escalation_opt['r']);

    try {
      objects::link(new_hstescalation,
                    hstesc_contacts,
                    hstesc_contactgroups,
                    escalation_period_ptr);
    }
    catch (std::exception const& e) {
      (void)e;
      objects::release(new_hstescalation);
      throw;
    }
  }
}

/**
 *  Create a new service dependency into the engine.
 *
 *  @param[in] servicedependency The struct with all
 *  information to create new service dependency.
 */
void webservice::create_service_dependency(ns1__serviceDependencyType const& svcdependency) {
  // check all arguments and set default option for optional options.
  if (!svcdependency.executionFailureCriteria
      && !svcdependency.notificationFailureCriteria)
    throw (engine_error() << "servicedependency have no notification failure "
           << "criteria and no execution failure criteria define.");

  if (svcdependency.hostgroupsName.empty() == true
      && svcdependency.hostsName.empty() == true)
    throw (engine_error() << "serviceependency have no hosts and no host groups define.");

  if (svcdependency.dependentHostsName.empty() == true
      && svcdependency.dependentHostgroupsName.empty() == true)
    throw (engine_error() << "serviceependency have no dependency hosts "
           << "and no dependency host groups define.");

  std::map<char, bool> execution_opt = get_options(svcdependency.executionFailureCriteria, "owucp", "n");
  if (execution_opt.empty())
    throw (engine_error() << "servicedependency invalid execution failure criteria.");

  std::map<char, bool> notif_opt = get_options(svcdependency.notificationFailureCriteria, "owucp", "n");
  if (notif_opt.empty())
    throw (engine_error() << "servicedependency invalid notification failure criteria.");

  char const* dependency_period = NULL;
  timeperiod* dependency_period_ptr = NULL;
  if (svcdependency.dependencyPeriod != NULL) {
    dependency_period = svcdependency.dependencyPeriod->c_str();
    if ((dependency_period_ptr = find_timeperiod(dependency_period)) == NULL)
      throw (engine_error() << "servicedependency invalid dependency period.");
  }

  std::vector<host*> svcdep_hosts(
                       _find<host>(
                         svcdependency.hostsName,
                         (void* (*)(char const*))&find_host));
  if (svcdependency.hostsName.size() != svcdep_hosts.size())
    throw (engine_error() << "servicedependency invalid hosts name");

  std::vector<host*> svcdep_dependent_hosts(
                       _find<host>(
                         svcdependency.dependentHostsName,
                         (void* (*)(char const*))&find_host));
  if (svcdependency.dependentHostsName.size()
      != svcdep_dependent_hosts.size())
    throw (engine_error()
           << "servicedependency invalid dependent hosts name");

  std::vector<hostgroup*> svcdep_hostgroups(
                            _find<hostgroup>(
                              svcdependency.hostgroupsName,
                              (void* (*)(char const*))&find_hostgroup));
  if (svcdependency.hostgroupsName.size() != svcdep_hostgroups.size())
    throw (engine_error()
           << "servicedependency invalid host groups name");

  std::vector<hostgroup*> svcdep_dependent_hostgroups(
                            _find<hostgroup>(
                              svcdependency.dependentHostgroupsName,
                              (void* (*)(char const*))&find_hostgroup));
  if (svcdependency.dependentHostgroupsName.size()
      != svcdep_dependent_hostgroups.size())
    throw (engine_error()
           << "servicedependency invalid dependent host groups name");

  _extract_object_from_objectgroup(svcdep_dependent_hostgroups, svcdep_dependent_hosts);
  _extract_object_from_objectgroup(svcdep_hostgroups, svcdep_hosts);

  bool inherits_parent = (svcdependency.inheritsParent
                          ? *svcdependency.inheritsParent : true);

  char const* service_description = svcdependency.serviceDescription.c_str();
  char const* dependent_service_description = svcdependency.dependentServiceDescription.c_str();

  for (std::vector<host*>::const_iterator
         it_dep(svcdep_dependent_hosts.begin()),
         end_dep(svcdep_dependent_hosts.end());
       it_dep != end_dep;
       ++it_dep) {
    for (std::vector<host*>::const_iterator
           it(svcdep_hosts.begin()),
           end(svcdep_hosts.end());
         it != end;
         ++it) {
      if (svcdependency.executionFailureCriteria != NULL) {
        servicedependency* new_svcdependency =
          add_service_dependency((*it_dep)->name,
                                 dependent_service_description,
                                 (*it)->name,
                                 service_description,
                                 EXECUTION_DEPENDENCY,
                                 inherits_parent,
                                 execution_opt['o'],
                                 execution_opt['w'],
                                 execution_opt['u'],
                                 execution_opt['c'],
                                 execution_opt['p'],
                                 dependency_period);

        try {
          objects::link(new_svcdependency, dependency_period_ptr);
        }
        catch (std::exception const& e) {
          (void)e;
          objects::release(new_svcdependency);
          throw;
        }
      }

      if (svcdependency.notificationFailureCriteria != NULL) {
        servicedependency* new_svcdependency =
          add_service_dependency((*it_dep)->name,
                                 dependent_service_description,
                                 (*it)->name,
                                 service_description,
                                 NOTIFICATION_DEPENDENCY,
                                 inherits_parent,
                                 notif_opt['o'],
                                 notif_opt['w'],
                                 notif_opt['u'],
                                 notif_opt['c'],
                                 notif_opt['p'],
                                 dependency_period);

        try {
          objects::link(new_svcdependency, dependency_period_ptr);
        }
        catch (std::exception const& e) {
          (void)e;
          objects::release(new_svcdependency);
          throw;
        }
      }
    }
  }
}

/**
 *  Create a new service escalation into the engine.
 *
 *  @param[in] serviceescalation The struct with all
 *  information to create new service escalation.
 */
void webservice::create_service_escalation(ns1__serviceEscalationType const& svcescalation) {
  // check all arguments and set default option for optional options.
  if (svcescalation.contacts.empty() == true && svcescalation.contactGroups.empty() == true)
    throw (engine_error() << "serviceescalation no contact and no contact groups are defined.");

  if (svcescalation.hostsName.empty() == true && svcescalation.hostgroupsName.empty() == true)
    throw (engine_error() << "serviceescalation no host and no host groups are defined.");

  std::map<char, bool> escalation_opt = get_options(svcescalation.escalationOptions, "wucr" , "n");
  if (escalation_opt.empty())
    throw (engine_error() << "serviceescalation invalid escalation options.");

  char const* escalation_period = NULL;
  timeperiod* escalation_period_ptr = NULL;
  if (svcescalation.escalationPeriod != NULL) {
    escalation_period = svcescalation.escalationPeriod->c_str();
    if ((escalation_period_ptr = find_timeperiod(escalation_period)) == NULL)
      throw (engine_error() << "serviceescalation invalid check period.");
  }

  std::vector<contact*> svcesc_contacts(
                          _find<contact>(
                            svcescalation.contacts,
                            (void* (*)(char const*))&find_contact));
  if (svcescalation.contacts.size() != svcesc_contacts.size())
    throw (engine_error() << "serviceescalation invalid contacts.");

  std::vector<contactgroup*>
    svcesc_contactgroups(
      _find<contactgroup>(
        svcescalation.contactGroups,
        (void* (*)(char const*))&find_contactgroup));
  if (svcescalation.contactGroups.size() != svcesc_contactgroups.size())
    throw (engine_error()
           << "serviceescalation invalid contact groups");

  std::vector<hostgroup*> svcesc_hostgroups(
                            _find<hostgroup>(
                              svcescalation.hostgroupsName,
                              (void* (*)(char const*))&find_hostgroup));
  if (svcescalation.hostgroupsName.size() != svcesc_hostgroups.size())
    throw (engine_error() << "serviceescalation invalid host groups");

  std::vector<host*> svcesc_hosts(
                       _find<host>(
                         svcescalation.hostsName,
                         (void* (*)(char const*))&find_host));
  if (svcescalation.hostsName.size() != svcesc_hosts.size())
    throw (engine_error() << "serviceescalation invalid hosts");

  _extract_object_from_objectgroup(svcesc_hostgroups, svcesc_hosts);

  char const* service_description = svcescalation.serviceDescription.c_str();
  for (std::vector<host*>::const_iterator
         it(svcesc_hosts.begin()),
         end(svcesc_hosts.end());
       it != end;
       ++it) {
    serviceescalation* new_svcescalation =
      add_service_escalation((*it)->name,
                            service_description,
                            svcescalation.firstNotification,
                            svcescalation.lastNotification,
                            svcescalation.notificationInterval,
                            escalation_period,
                            escalation_opt['w'],
                            escalation_opt['u'],
                            escalation_opt['c'],
                            escalation_opt['r']);

    try {
      objects::link(new_svcescalation,
                    svcesc_contacts,
                    svcesc_contactgroups,
                    escalation_period_ptr);
    }
    catch (std::exception const& e) {
      (void)e;
      objects::release(new_svcescalation);
      throw;
    }
  }
}
