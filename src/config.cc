/*
** Copyright 1999-2008 Ethan Galstad
** Copyright 2011-2019 Centreon
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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;

/****************************************************************/
/**************** CONFIG VERIFICATION FUNCTIONS *****************/
/****************************************************************/

/* dfs status values */
#define DFS_UNCHECKED                    0      /* default value */
#define DFS_TEMP_CHECKED                 1      /* check just one time */
#define DFS_OK                           2      /* no problem */
#define DFS_NEAR_LOOP                    3      /* has trouble sons */
#define DFS_LOOPY                        4      /* is a part of a loop */

#define dfs_get_status(h) h->get_circular_path_checked()
#define dfs_unset_status(h) h->set_circular_path_checked(0)
#define dfs_set_status(h, flag) h->set_circular_path_checked(flag)
#define dfs_host_status(h) (h ? dfs_get_status(h) : DFS_OK)

/**
 * Modified version of Depth-first Search
 * http://en.wikipedia.org/wiki/Depth-first_search
 */
static int dfs_host_path(host* root) {
  if (!root)
    return DFS_NEAR_LOOP;

  if (dfs_get_status(root) != DFS_UNCHECKED)
    return dfs_get_status(root);

  /* Mark the root temporary checked */
  dfs_set_status(root, DFS_TEMP_CHECKED);

  /* We are scanning the children */
  for (host_map_unsafe::iterator
         it(root->child_hosts.begin()),
         end(root->child_hosts.end());
       it != end;
       it++) {
    int child_status = dfs_get_status(it->second);

    /* If a child is not checked, check it */
    if (child_status == DFS_UNCHECKED)
      child_status = dfs_host_path(it->second);

    /* If a child already temporary checked, its a problem,
     * loop inside, and its a acked status */
    if (child_status == DFS_TEMP_CHECKED) {
      dfs_set_status(it->second, DFS_LOOPY);
      dfs_set_status(root, DFS_LOOPY);
    }

    /* If a child already temporary checked, its a problem, loop inside */
    if (child_status == DFS_NEAR_LOOP || child_status == DFS_LOOPY) {
      /* if a node is know to be part of a loop, do not let it be less */
      if (dfs_get_status(root) != DFS_LOOPY)
        dfs_set_status(root, DFS_NEAR_LOOP);

      /* we already saw this child, it's a problem */
      dfs_set_status(it->second, DFS_LOOPY);
    }
  }

  /*
   * If root have been modified, do not set it OK
   * A node is OK if and only if all of his children are OK
   * If it does not have child, goes ok
   */
  if (dfs_get_status(root) == DFS_TEMP_CHECKED)
    dfs_set_status(root, DFS_OK);
  return dfs_get_status(root);
}

/* check for circular paths and dependencies */
int pre_flight_circular_check(int* w, int* e) {
  int found(false);
  int warnings(0);
  int errors(0);

  /* bail out if we aren't supposed to verify circular paths */
  if (!verify_circular_paths)
    return OK;

  /********************************************/
  /* check for circular paths between hosts   */
  /********************************************/

/* We clean the dsf status from previous check */
  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it)
    dfs_set_status(it->second.get(), DFS_UNCHECKED);

  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it)
    if (dfs_host_path(it->second.get()) == DFS_LOOPY)
      errors = 1;

  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it) {
    if (dfs_get_status(it->second.get()) == DFS_LOOPY)
      logger(log_verification_error, basic)
        << "Error: The host '" << it->first
        << "' is part of a circular parent/child chain!";
    /* clean DFS status */
    dfs_set_status(it->second.get(), DFS_UNCHECKED);
  }

  /********************************************/
  /* check for circular dependencies         */
  /********************************************/

  /* check execution dependencies between all services */
  for (servicedependency_mmap::iterator
         it(servicedependency::servicedependencies.begin()),
         end(servicedependency::servicedependencies.end());
       it != end;
       ++it) {

    /* clear checked flag for all dependencies */
    for (servicedependency_mmap::iterator
           it2(servicedependency::servicedependencies.begin()),
           end2(servicedependency::servicedependencies.end());
         it2 != end2;
         ++it2)
      it2->second->set_circular_path_checked(false);

    found = it->second->check_for_circular_servicedependency_path(
              it->second.get(),
              hostdependency::execution);
    if (found) {
      logger(log_verification_error, basic)
        << "Error: A circular execution dependency (which could result "
        "in a deadlock) exists for service '"
        << it->second->get_service_description() << "' on host '"
        << it->second->get_hostname() << "'!";
      errors++;
    }
  }

  /* check notification dependencies between all services */
  for (servicedependency_mmap::iterator
         it(servicedependency::servicedependencies.begin()),
         end(servicedependency::servicedependencies.end());
       it != end;
       ++it) {

    /* clear checked flag for all dependencies */
    for (servicedependency_mmap::iterator
           it2(servicedependency::servicedependencies.begin()),
           end2(servicedependency::servicedependencies.end());
         it2 != end2;
         ++it2)
      it2->second->set_circular_path_checked(false);

    found = it->second->check_for_circular_servicedependency_path(
              it->second.get(),
              hostdependency::notification);
    if (found) {
      logger(log_verification_error, basic)
        << "Error: A circular notification dependency (which could "
        "result in a deadlock) exists for service '"
        << it->second->get_service_description() << "' on host '"
        << it->second->get_hostname() << "'!";
      errors++;
    }
  }

  /* clear checked flag for all dependencies */
  for (servicedependency_mmap::iterator
         it(servicedependency::servicedependencies.begin()),
         end(servicedependency::servicedependencies.end());
       it != end;
       ++it)
    it->second->set_circular_path_checked(false);

  /* check execution dependencies between all hosts */
  for (hostdependency_mmap::iterator
         it(hostdependency::hostdependencies.begin()),
         end(hostdependency::hostdependencies.end());
       it != end;
       ++it) {

    /* clear checked flag for all dependencies */
    for (hostdependency_mmap::iterator
           it2(hostdependency::hostdependencies.begin()),
           end2(hostdependency::hostdependencies.end());
         it2 != end2;
         ++it2)
      it2->second->set_circular_path_checked(false);

    found = it->second->check_for_circular_hostdependency_path(
      it->second.get(), hostdependency::execution);
    if (found) {
      logger(log_verification_error, basic)
        << "Error: A circular execution dependency (which could "
        "result in a deadlock) exists for host '"
        << it->second->get_hostname() << "'!";
      errors++;
    }
  }

  /* check notification dependencies between all hosts */
  for (hostdependency_mmap::iterator
         it(hostdependency::hostdependencies.begin()),
         end(hostdependency::hostdependencies.end());
       it != end;
       ++it) {

    /* clear checked flag for all dependencies */
    for (hostdependency_mmap::iterator
           it2(hostdependency::hostdependencies.begin()),
           end2(hostdependency::hostdependencies.end());
         it2 != end2;
         ++it2)
      it2->second->set_circular_path_checked(false);

    found = it->second->check_for_circular_hostdependency_path(
      it->second.get(), hostdependency::notification);
    if (found) {
      logger(log_verification_error, basic)
        << "Error: A circular notification dependency (which could "
        "result in a deadlock) exists for host '"
        << it->first << "'!";
      errors++;
    }
  }

  /* clear checked flag for all dependencies */
  for (hostdependency_mmap::iterator
         it(hostdependency::hostdependencies.begin()),
         end(hostdependency::hostdependencies.end());
       it != end;
       ++it)
      it->second->set_circular_path_checked(false);

  /* update warning and error count */
  if (w != nullptr)
    *w += warnings;
  if (e != nullptr)
    *e += errors;

  return (errors > 0) ? ERROR : OK;
}

int check_contact(std::shared_ptr<contact> cntct, int* w, int* e) {
  int warnings(0);
  int errors(0);

  /* check service notification commands */
  if (cntct->get_service_notification_commands().empty()) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << cntct->get_name() << "' has no service "
      "notification commands defined!";
    errors++;
  }

  /* check host notification commands */
  if (cntct->get_host_notification_commands().empty()) {
    logger(log_verification_error, basic)
      << "Error: Contact '" << cntct->get_name() << "' has no host "
      "notification commands defined!";
    errors++;
  }

  /* check service notification timeperiod */
  if (cntct->get_service_notification_period().empty()) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << cntct->get_name() << "' has no service "
      "notification time period defined!";
    warnings++;
  }

  else {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(timeperiod::timeperiods.find(cntct->get_service_notification_period()));

    if (it != timeperiod::timeperiods.end())
      temp_timeperiod = it->second.get();

    if (temp_timeperiod == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Service notification period '"
        << cntct->get_service_notification_period()
        << "' specified for contact '" << cntct->get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the service notification timeperiod for later */
    cntct->service_notification_period_ptr = temp_timeperiod;
  }

  /* check host notification timeperiod */
  if (cntct->get_host_notification_period().empty()) {
    logger(log_verification_error, basic)
      << "Warning: Contact '" << cntct->get_name() << "' has no host "
      "notification time period defined!";
    warnings++;
  }

  else {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(timeperiod::timeperiods.find(cntct->get_host_notification_period()));

    if (it != timeperiod::timeperiods.end())
      temp_timeperiod = it->second.get();

    if (temp_timeperiod == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Host notification period '"
        << cntct->get_host_notification_period()
        << "' specified for contact '" << cntct->get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the host notification timeperiod for later */
    cntct->host_notification_period_ptr = temp_timeperiod;
  }

  /* check for sane host recovery options */
  if (cntct->notify_on(notifier::host_notification, notifier::recovery)
      && !cntct->notify_on(notifier::host_notification, notifier::down)
      && !cntct->notify_on(notifier::host_notification, notifier::unreachable)) {
    logger(log_verification_error, basic)
      << "Warning: Host recovery notification option for contact '"
      << cntct->get_name() << "' doesn't make any sense - specify down "
      "and/or unreachable options as well";
    warnings++;
  }

  /* check for sane service recovery options */
  if (cntct->notify_on(notifier::service_notification, notifier::recovery)
      && !cntct->notify_on(notifier::service_notification, notifier::critical)
      && !cntct->notify_on(notifier::service_notification, notifier::warning)) {
    logger(log_verification_error, basic)
      << "Warning: Service recovery notification option for contact '"
      << cntct->get_name() << "' doesn't make any sense - specify critical "
      "and/or warning options as well";
    warnings++;
  }

  /* check for illegal characters in contact name */
  if (contains_illegal_object_chars(const_cast<char*>(cntct->get_name().c_str()))) {
    logger(log_verification_error, basic)
      << "Error: The name of contact '" << cntct->get_name()
      << "' contains one or more illegal characters.";
    errors++;
  }

  if (w != nullptr)
    *w += warnings;
  if (e != nullptr)
    *e += errors;
  return errors == 0;
}

/**
 *  Check and resolve service groups.
 *
 *  @param[in,out] sg Service group object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_servicegroup(std::shared_ptr<servicegroup> sg, int* w, int* e) {
  (void)w;
  int errors(0);

  // Check all group members.
  for (service_map_unsafe::iterator
         it(sg->members.begin()),
         end(sg->members.end());
       it != end;
       ++it) {

   service_map::const_iterator found(service::services.find(
     it->first));

    if (found == service::services.end() || !found->second) {
      logger(log_verification_error, basic)
        << "Error: Service '"
        << it->first.second
        << "' on host '" << it->first.first
        << "' specified in service group '" << sg->get_group_name()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save a pointer to this servicegroup for faster service/group
    // membership lookups later.
    else {
      found->second->get_parent_groups().push_back(sg.get());

      // Save service pointer for later.
      sg->members[it->first] = found->second.get();
    }
  }

  // Check for illegal characters in servicegroup name.
  if (contains_illegal_object_chars(sg->get_group_name().c_str())) {
    logger(log_verification_error, basic)
      << "Error: The name of servicegroup '" << sg->get_group_name()
      << "' contains one or more illegal characters.";
    errors++;
  }

  // Add errors.
  if (e)
    *e += errors;

  return errors == 0;
}

/**
 *  Check and resolve host groups.
 *
 *  @param[in,out] hg Host group object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_hostgroup(std::shared_ptr<hostgroup> hg, int* w, int* e) {
  (void)w;
  int errors(0);

  // Check all group members.
  for (host_map_unsafe::iterator
         it(hg->members.begin()),
         end(hg->members.end());
       it != end;
       ++it) {

    host_map::const_iterator it_host(host::hosts.find(it->first.c_str()));
    if (it_host == host::hosts.end() || !it_host->second) {
      logger(log_verification_error, basic)
        << "Error: Host '" << it->first
        << "' specified in host group '" << hg->get_group_name()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save a pointer to this hostgroup for faster host/group
    // membership lookups later.
    else {
      it_host->second->get_parent_groups().push_back(hg.get());

      // Save host pointer for later.
      hg->members[it->first] = it_host->second.get();
    }
  }

  // Check for illegal characters in hostgroup name.
  if (contains_illegal_object_chars(hg->get_group_name().c_str())) {
    logger(log_verification_error, basic)
      << "Error: The name of hostgroup '" << hg->get_group_name()
      << "' contains one or more illegal characters.";
    errors++;
  }

  // Add errors.
  if (e)
    *e += errors;

  return errors == 0;
}

/**
 *  Check and resolve a contact group.
 *
 *  @param[in,out] cg Contact group object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_contactgroup(std::shared_ptr<contactgroup> cg, int* w, int* e) {
  (void)w;
  int errors(0);

  for (contact_map_unsafe::const_iterator
         it{cg->get_members().begin()},
         end{cg->get_members().end()};
       it != end;
       ++it) {
    contact* temp_contact{it->second};
    if (!temp_contact) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << it->first
        << "' specified in contact group '" << cg->get_name()
        << "' is not defined anywhere!";
      errors++;
    }
    // Save a pointer to this contact group for faster contact/group
    // membership lookups later.
    else {
      temp_contact->get_parent_groups().push_back(cg.get());

      // Save the contact pointer for later.
      // FIXME DBR: something strange here, we add again temp_contact to cg. But
      // we get it from cg.
      cg->add_member(temp_contact);
    }
  }

  // Check for illegal characters in contact group name.
  if (contains_illegal_object_chars(const_cast<char *>(cg->get_name().c_str()))) {
    logger(log_verification_error, basic)
      << "Error: The name of contact group '" << cg->get_name()
      << "' contains one or more illegal characters.";
    errors++;
  }

  // Add errors.
  if (e)
    *e += errors;

  return errors == 0;
}

/**
 *  Check and resolve a service dependency.
 *
 *  @param[in,out] sd Service dependency object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_servicedependency(std::shared_ptr<servicedependency> sd, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the dependent service.
  service_map::const_iterator found(service::services.find(
    {sd->get_dependent_hostname(), sd->get_dependent_service_description()}));

  if (found == service::services.end() || !found->second) {
    logger(log_verification_error, basic)
      << "Error: Dependent service '"
      << sd->get_dependent_service_description() << "' on host '"
      << sd->get_dependent_hostname()
      << "' specified in service dependency for service '"
      << sd->get_service_description() << "' on host '"
      << sd->get_hostname() << "' is not defined anywhere!";
    errors++;
  }

  sd->dependent_service_ptr = found->second.get();

  // Save pointer for later.
  found = service::services.find({sd->get_hostname(), sd->get_service_description()});

  // Find the service we're depending on.
  if (found == service::services.end() || !found->second) {
    logger(log_verification_error, basic)
      << "Error: Service '" << sd->get_service_description() << "' on host '"
      << sd->get_hostname()
      << "' specified in service dependency for service '"
      << sd->get_dependent_service_description() << "' on host '"
      << sd->get_dependent_hostname() << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  else
    sd->master_service_ptr = found->second.get();

  // Make sure they're not the same service.
  if (sd->dependent_service_ptr == sd->master_service_ptr) {
    logger(log_verification_error, basic)
      << "Error: Service dependency definition for service '"
      << sd->get_dependent_service_description() << "' on host '"
      << sd->get_dependent_hostname()
      << "' is circular (it depends on itself)!";
    errors++;
  }

  // Find the timeperiod.
  if (!sd->get_dependency_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(timeperiod::timeperiods.find(sd->get_dependency_period()));

    if (it != timeperiod::timeperiods.end())
      temp_timeperiod = it->second.get();

    if (!temp_timeperiod) {
      logger(log_verification_error, basic)
        << "Error: Dependency period '" << sd->get_dependency_period()
        << "' specified in service dependency for service '"
        << sd->get_dependent_service_description() << "' on host '"
        << sd->get_dependent_hostname() << "' is not defined anywhere!";
      errors++;
    }

    // Save the timeperiod pointer for later.
    sd->dependency_period_ptr = temp_timeperiod;
  }

  // Add errors.
  if (e)
    *e += errors;

  return errors == 0;
}

/**
 *  Check and resolve a host dependency.
 *
 *  @param[in,out] hd Host dependency object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_hostdependency(std::shared_ptr<hostdependency> hd, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the dependent host.
  host_map::const_iterator it(host::hosts.find(hd->get_dependent_hostname()));
  if (it == host::hosts.end() || !it->second) {
    logger(log_verification_error, basic)
      << "Error: Dependent host specified in host dependency for "
         "host '" << hd->get_dependent_hostname()
      << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  if (it == host::hosts.end())
    hd->dependent_host_ptr = nullptr;
  else
    hd->dependent_host_ptr = it->second.get();

  // Find the host we're depending on.
  it = host::hosts.find(hd->get_hostname());
  if (it == host::hosts.end() || !it->second) {
    logger(log_verification_error, basic)
      << "Error: Host specified in host dependency for host '"
      << hd->get_dependent_hostname() << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  if (it == host::hosts.end())
    hd->master_host_ptr = nullptr;
  else
    hd->master_host_ptr = it->second.get();

  // Make sure they're not the same host.
  if (hd->dependent_host_ptr == hd->master_host_ptr) {
    logger(log_verification_error, basic)
      << "Error: Host dependency definition for host '"
      << hd->get_dependent_hostname()
      << "' is circular (it depends on itself)!";
    errors++;
  }

  // Find the timeperiod.
  if (!hd->get_dependency_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(timeperiod::timeperiods.find(hd->get_dependency_period()));

    if (it != timeperiod::timeperiods.end())
      temp_timeperiod = it->second.get();

    if (!temp_timeperiod) {
      logger(log_verification_error, basic)
        << "Error: Dependency period '" << hd->get_dependency_period()
        << "' specified in host dependency for host '"
        << hd->get_dependent_hostname()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save the timeperiod pointer for later.
    if (it == timeperiod::timeperiods.end())
      hd->dependency_period_ptr = nullptr;
    else
      hd->dependency_period_ptr = temp_timeperiod;
  }

  // Add errors.
  if (e)
    *e += errors;

  return errors == 0;
}

/**
 *  Check and resolve a service escalation.
 *
 *  @param[in,out] se Service escalation object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_serviceescalation(std::shared_ptr<serviceescalation> se, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the service.
  service_map::const_iterator found = service::services.find(
    {se->get_hostname(), se->get_description()});

  if (found == service::services.end() || !found->second) {
    logger(log_verification_error, basic) << "Error: Service '"
        << se->get_description() << "' on host '" << se->get_hostname()
        << "' specified in service escalation is not defined anywhere!";
    errors++;
  }

  // Save the service pointer for later.
  se->notifier_ptr = found->second.get();

  // Find the timeperiod.
  if (!se->get_escalation_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(timeperiod::timeperiods.find(se->get_escalation_period()));

    if (it != timeperiod::timeperiods.end())
      temp_timeperiod = it->second.get();

    if (!temp_timeperiod) {
      logger(log_verification_error, basic)
        << "Error: Escalation period '" << se->get_escalation_period()
        << "' specified in service escalation for service '"
        << se->get_description() << "' on host '"
        << se->get_hostname() << "' is not defined anywhere!";
      errors++;
    }

    // Save the timeperiod pointer for later.
    se->escalation_period_ptr = temp_timeperiod;
  }

  // Check all contacts.
  for (std::pair<std::string, contact*> const& p : se->contacts()) {
    contact_map::const_iterator ct_it{contact::contacts.find(p.first)};
    if (ct_it == contact::contacts.end()) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << p.first
        << "' specified in service escalation for service '"
        << se->get_description() << "' on host '"
        << se->get_hostname() << "' is not defined anywhere!";
      errors++;
    }
  }

  // Check all contact groups.
  for (contactgroup_map_unsafe::iterator
         it(se->contact_groups().begin()),
         end(se->contact_groups().end());
       it != end;
       ++it) {
    // Find the contact group.
    contactgroup_map::iterator find_cg{
      contactgroup::contactgroups.find(it->first)};

    if (find_cg == contactgroup::contactgroups.end() || !find_cg->second) {
      logger(log_verification_error, basic)
        << "Error: Contact group '"
        << it->first
        << "' specified in service escalation for service '"
        << se->get_description() << "' on host '" << se->get_hostname()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save the contactgroup pointer for later.
    se->contact_groups()[it->first] = find_cg->second.get();
  }

  // Add errors.
  if (e)
    *e += errors;

  return errors == 0;
}

/**
 *  Check and resolve a host escalation.
 *
 *  @param[in,out] he Host escalation object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_hostescalation(std::shared_ptr<hostescalation> he, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the host.
  host_map::const_iterator it(host::hosts.find(he->get_hostname()));
  if (it == host::hosts.end() || !it->second) {
    logger(log_verification_error, basic)
      << "Error: Host '" << he->get_hostname()
      << "' specified in host escalation is not defined anywhere!";
    errors++;
  }

  // Save the host pointer for later.
  if (it == host::hosts.end())
    he->notifier_ptr = nullptr;
  else
    he->notifier_ptr = it->second.get();

  // Find the timeperiod.
  if (!he->get_escalation_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(timeperiod::timeperiods.find(he->get_escalation_period()));

    if (it != timeperiod::timeperiods.end())
      temp_timeperiod = it->second.get();

    if (!temp_timeperiod) {
      logger(log_verification_error, basic)
        << "Error: Escalation period '" << he->get_escalation_period()
        << "' specified in host escalation for host '"
        << he->get_hostname() << "' is not defined anywhere!";
      errors++;
    }

    // Save the timeperiod pointer for later.
    he->escalation_period_ptr = temp_timeperiod;
  }

  // Check all contacts.
  for (std::pair<std::string, contact*> const& p : he->contacts()) {
    // Find the contact.
    contact_map::const_iterator ct_it{contact::contacts.find(p.first)};
    if (ct_it == contact::contacts.end()) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << p.first
        << "' specified in host escalation for host '"
        << he->get_hostname() << "' is not defined anywhere!";
      errors++;
    }
  }

  // Check all contact groups.
  for (contactgroup_map_unsafe::iterator
         it(he->contact_groups().begin()),
         end(he->contact_groups().end());
       it != end;
       ++it) {
    // Find the contact group.
    contactgroup_map::iterator it_cg(contactgroup::contactgroups.find(it->first));

    if (it_cg == contactgroup::contactgroups.end() || !it_cg->second) {
      logger(log_verification_error, basic)
        << "Error: Contact group '"
        << it->first
        << "' specified in host escalation for host '"
        << he->get_hostname() << "' is not defined anywhere!";
      errors++;
    } else {
      // Save the contactgroup pointer for later.
      he->contact_groups()[it->first] = it_cg->second.get();
    }
  }

  // Add errors.
  if (e)
    *e += errors;
  return errors == 0;
}
