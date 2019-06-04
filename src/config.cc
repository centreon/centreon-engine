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
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;

/****************************************************************/
/**************** CONFIG VERIFICATION FUNCTIONS *****************/
/****************************************************************/

/* do a pre-flight check to make sure object relationships, etc. make sense */
int pre_flight_check() {
  host* temp_host(nullptr);
  char* buf(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  commands::command* temp_command(nullptr);
  char* temp_command_name(nullptr);
  int warnings(0);
  int errors(0);
  struct timeval tv[4];
  double runtime[4];

  if (test_scheduling)
    gettimeofday(&tv[0], nullptr);

  /********************************************/
  /* check object relationships               */
  /********************************************/
  pre_flight_object_check(&warnings, &errors);
  if (test_scheduling)
    gettimeofday(&tv[1], nullptr);

  /********************************************/
  /* check for circular paths between hosts   */
  /********************************************/
  pre_flight_circular_check(&warnings, &errors);
  if (test_scheduling)
    gettimeofday(&tv[2], nullptr);

  /********************************************/
  /* check global event handler commands...   */
  /********************************************/
  if (verify_config)
    printf("Checking global event handlers...\n");

  if (config->global_host_event_handler() != "") {

    /* check the event handler command */
    buf = string::dup(config->global_host_event_handler());

    /* get the command name, leave any arguments behind */
    temp_command_name = my_strtok(buf, "!");

    temp_command = configuration::applier::state::instance().find_command(
      temp_command_name);
    if (temp_command == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Global host event handler command '"
        << temp_command_name << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the command for later */
    global_host_event_handler_ptr = temp_command;

    delete[] buf;
  }

  if (config->global_service_event_handler() != "") {

    /* check the event handler command */
    buf = string::dup(config->global_service_event_handler());

    /* get the command name, leave any arguments behind */
    temp_command_name = my_strtok(buf, "!");

    temp_command = configuration::applier::state::instance().find_command(
      temp_command_name);
    if (temp_command == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Global service event handler command '"
        << temp_command_name << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the command for later */
    global_service_event_handler_ptr = temp_command;

    delete[] buf;
  }

  /**************************************************/
  /* check obsessive processor commands...          */
  /**************************************************/
  if (verify_config)
    printf("Checking obsessive compulsive processor commands...\n");

  if (!config->ocsp_command().empty()) {

    buf = string::dup(config->ocsp_command());

    /* get the command name, leave any arguments behind */
    temp_command_name = my_strtok(buf, "!");

    temp_command = configuration::applier::state::instance().find_command(
      temp_command_name);
    if (temp_command == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Obsessive compulsive service processor command '"
        << temp_command_name << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the command for later */
    ocsp_command_ptr = temp_command;

    delete[] buf;
  }

  if (!config->ochp_command().empty()) {

    buf = string::dup(config->ochp_command());

    /* get the command name, leave any arguments behind */
    temp_command_name = my_strtok(buf, "!");

    temp_command = configuration::applier::state::instance().find_command(
      temp_command_name);
    if (temp_command == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Obsessive compulsive host processor command '"
        << temp_command_name << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the command for later */
    ochp_command_ptr = temp_command;

    delete[] buf;
  }

  /* count number of services associated with each host (we need this for flap detection)... */
  for (temp_service = service_list;
       temp_service != nullptr;
       temp_service = temp_service->next) {

    umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it(state::instance().hosts().find(get_host_id(temp_service->get_hostname())));
    if (it != state::instance().hosts().end() && it->second != nullptr) {
      it->second->set_total_services(it->second->get_total_services() + 1);
      it->second->set_total_service_check_interval(
        it->second->get_total_service_check_interval()
        + static_cast<unsigned long>(temp_service->get_check_interval()));
    }
  }

  if (verify_config) {
    printf("\n");
    printf("Total Warnings: %d\n", warnings);
    printf("Total Errors:   %d\n", errors);
  }

  if (test_scheduling)
    gettimeofday(&tv[3], nullptr);

  if (test_scheduling) {

    runtime[0]
      = (double)((double)(tv[1].tv_sec - tv[0].tv_sec)
                 + (double)((tv[1].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);
    if (verify_circular_paths)
      runtime[1]
        = (double)((double)(tv[2].tv_sec - tv[1].tv_sec)
                   + (double)((tv[2].tv_usec - tv[1].tv_usec) / 1000.0) / 1000.0);
    else
      runtime[1] = 0.0;
    runtime[2]
      = (double)((double)(tv[3].tv_sec - tv[2].tv_sec)
                 + (double)((tv[3].tv_usec - tv[2].tv_usec) / 1000.0) / 1000.0);
    runtime[3] = runtime[0] + runtime[1] + runtime[2];

    printf("Timing information on configuration verification is listed below.\n\n");

    printf("CONFIG VERIFICATION TIMES          (* = Potential for speedup with -x option)\n");
    printf("----------------------------------\n");
    printf("Object Relationships: %.6f sec\n", runtime[0]);
    printf("Circular Paths:       %.6f sec  *\n", runtime[1]);
    printf("Misc:                 %.6f sec\n", runtime[2]);
    printf("                      ============\n");
    printf(
      "TOTAL:                %.6f sec  * = %.6f sec (%.1f%%) estimated savings\n",
      runtime[3],
      runtime[1],
      (runtime[1] / runtime[3]) * 100.0);
    printf("\n\n");
  }

  return ((errors > 0) ? ERROR : OK);
}

/**
 *  Do a pre-flight check to make sure object relationships make sense.
 *
 *  @param[out] w Warning counter.
 *  @param[out] e Error counter.
 *
 *  @return OK on success.
 */
int pre_flight_object_check(int* w, int* e) {
  // Counters.
  int warnings(0);
  int errors(0);

  // Check each service...
  if (verify_config)
    logger(log_info_message, basic) << "Checking services...";
  int total_objects(0);
  for (com::centreon::engine::service* temp_service(service_list);
       temp_service;
       temp_service = temp_service->next, ++total_objects)
    check_service(temp_service, &warnings, &errors);
  if (verify_config)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " services.";

  // Check all hosts...
  if (verify_config)
    logger(log_info_message, basic) << "Checking hosts...";
  total_objects = 0;
  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it, ++total_objects) {
    check_host(it->second.get(), &warnings, &errors);
    if (verify_config)
      logger(log_info_message, basic)
        << "\tChecked " << total_objects << " hosts.";
  }
  // Check each host group...
  if (verify_config)
    logger(log_info_message, basic) << "Checking host groups...";
  total_objects = 0;
  for (hostgroup_map::iterator
         it(hostgroup::hostgroups.begin()),
         end(hostgroup::hostgroups.end());
       it != end;
       ++it, ++total_objects)
    check_hostgroup(it->second.get(), &warnings, &errors);
  if (verify_config)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " host groups.";

  // Check each service group...
  if (verify_config)
    logger(log_info_message, basic) << "Checking service groups...";
  total_objects = 0;
  for (servicegroup_map::iterator
         it(servicegroup::servicegroups.begin()),
         end(servicegroup::servicegroups.end());
       it != end;
       ++it)
    check_servicegroup(it->second.get(), &warnings, &errors);
  if (verify_config)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " service groups.";

  // Check all contacts...
  if (verify_config)
    logger(log_info_message, basic) << "Checking contacts...";
  total_objects = 0;
  for (std::unordered_map<std::string,
            std::shared_ptr<com::centreon::engine::contact> >::iterator
         it(configuration::applier::state::instance().contacts().begin()),
         end(configuration::applier::state::instance().contacts().end());
       it != end;
       ++it) {
    check_contact(it->second.get(), &warnings, &errors);
  }
  if (verify_config)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " contacts.";

  // Check each contact group...
  if (verify_config)
    logger(log_info_message, basic) << "Checking contact groups...";
  total_objects = 0;
  for (std::unordered_map<std::string,
            std::shared_ptr<com::centreon::engine::contactgroup> >::iterator
         it(configuration::applier::state::instance().contactgroups().begin()),
         end(configuration::applier::state::instance().contactgroups().end());
       it != end;
       ++it) {
    check_contactgroup(it->second.get(), &warnings, &errors);
    if (verify_config)
      logger(log_info_message, basic)
        << "\tChecked " << total_objects << " contact groups.";
  }
  // Check all service escalations...
  if (verify_config)
    logger(log_info_message, basic)
      << "Checking service escalations...";
  total_objects = 0;
  for (serviceescalation_mmap::iterator
       it{serviceescalation::serviceescalations.begin()},
       end{serviceescalation::serviceescalations.end()};
       it != end;
       ++it, ++total_objects)
    check_serviceescalation(it->second.get(), &warnings, &errors);
  if (verify_config)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " service escalations.";

  // Check all service dependencies...
  if (verify_config)
    logger(log_info_message, basic)
      << "Checking service dependencies...";
  total_objects = 0;
  for (servicedependency_mmap::iterator
         it(servicedependency::servicedependencies.begin()),
         end(servicedependency::servicedependencies.end());
       it != end;
       ++it)
    check_servicedependency(it->second.get(), &warnings, &errors);
  if (verify_config)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " service dependencies.";

  // Check all host escalations...
  if (verify_config)
    logger(log_info_message, basic) << "Checking host escalations...";
  total_objects = 0;
  for (hostescalation_mmap::iterator
         it(hostescalation::hostescalations.begin()),
         end(hostescalation::hostescalations.end());
       it != end;
       ++it, ++total_objects)
    check_hostescalation(it->second.get(), &warnings, &errors);
  if (verify_config)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " host escalations.";

  // Check all host dependencies...
  if (verify_config)
    logger(log_info_message, basic) << "Checking host dependencies...";
  total_objects = 0;
  for (hostdependency_mmap::iterator
         it(hostdependency::hostdependencies.begin()),
         end(hostdependency::hostdependencies.end());
       it != end;
       ++it)
    check_hostdependency(it->second.get(), &warnings, &errors);
  if (verify_config)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " host dependencies.";

  // Check all commands...
  if (verify_config)
    logger(log_info_message, basic) << "Checking commands...";
  total_objects = 0;
  for (std::unordered_map<std::string, std::shared_ptr<commands::command>>::const_iterator
         it(configuration::applier::state::instance().commands().begin()),
         end(configuration::applier::state::instance().commands().end());
       it != end;
       ++it, ++total_objects) {
    std::shared_ptr<commands::command> const& temp_command(it->second);

    // Check for illegal characters in command name.
    if (contains_illegal_object_chars(temp_command->get_name().c_str())) {
      logger(log_verification_error, basic)
        << "Error: The name of command '" << temp_command->get_name()
        << "' contains one or more illegal characters.";
      errors++;
    }
  }
  if (verify_config)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " commands.\n";

  // Check all timeperiods...
  if (verify_config)
    logger(log_info_message, basic) << "Checking time periods...";
  total_objects = 0;
  for (timeperiod_map::iterator
         it(timeperiod::timeperiods.begin()),
         end(timeperiod::timeperiods.end());
       it != end;
       ++it)
    check_timeperiod(it->second.get(), &warnings, &errors);
  if (verify_config)
    logger(log_verification_error, basic)
      << "\tChecked " << total_objects << " time periods.";

  // Update warning and error count.
  *w += warnings;
  *e += errors;

  return ((errors > 0) ? ERROR : OK);
}

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
    return (DFS_NEAR_LOOP);

  if (dfs_get_status(root) != DFS_UNCHECKED)
    return (dfs_get_status(root));

  /* Mark the root temporary checked */
  dfs_set_status(root, DFS_TEMP_CHECKED);

  /* We are scanning the children */
  for (host_map::iterator
         it(root->child_hosts.begin()),
         end(root->child_hosts.end());
       it != end;
       it++) {
    int child_status = dfs_get_status(it->second.get());

    /* If a child is not checked, check it */
    if (child_status == DFS_UNCHECKED)
      child_status = dfs_host_path(it->second.get());

    /* If a child already temporary checked, its a problem,
     * loop inside, and its a acked status */
    if (child_status == DFS_TEMP_CHECKED) {
      dfs_set_status(it->second.get(), DFS_LOOPY);
      dfs_set_status(root, DFS_LOOPY);
    }

    /* If a child already temporary checked, its a problem, loop inside */
    if (child_status == DFS_NEAR_LOOP || child_status == DFS_LOOPY) {
      /* if a node is know to be part of a loop, do not let it be less */
      if (dfs_get_status(root) != DFS_LOOPY)
        dfs_set_status(root, DFS_NEAR_LOOP);

      /* we already saw this child, it's a problem */
      dfs_set_status(it->second.get(), DFS_LOOPY);
    }
  }

  /*
   * If root have been modified, do not set it OK
   * A node is OK if and only if all of his children are OK
   * If it does not have child, goes ok
   */
  if (dfs_get_status(root) == DFS_TEMP_CHECKED)
    dfs_set_status(root, DFS_OK);
  return (dfs_get_status(root));
}

/* check for circular paths and dependencies */
int pre_flight_circular_check(int* w, int* e) {
  servicedependency* temp_sd(nullptr);
  servicedependency* temp_sd2(nullptr);
  hostdependency* temp_hd(nullptr);
  hostdependency* temp_hd2(nullptr);
  int found(false);
  int warnings(0);
  int errors(0);

  /* bail out if we aren't supposed to verify circular paths */
  if (!verify_circular_paths)
    return (OK);

  /********************************************/
  /* check for circular paths between hosts   */
  /********************************************/

  /* check routes between all hosts */
  found = false;

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
         it2 != end;
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
         it2 != end;
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
        << temp_hd->get_hostname() << "'!";
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

  return ((errors > 0) ? ERROR : OK);
}

int check_service(com::centreon::engine::service* svc, int* w, int* e) {
  int errors(0);
  int warnings(0);

  /* check for a valid host */
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(state::instance().hosts().find(get_host_id(svc->get_hostname())));

  /* we couldn't find an associated host! */

  if (it == state::instance().hosts().end() || it->second == nullptr) {
    logger(log_verification_error, basic)
      << "Error: Host '" << svc->get_hostname() << "' specified in service "
      "'" << svc->get_description() << "' not defined anywhere!";
    errors++;
  }

  /* save the host pointer for later */
  if (it == state::instance().hosts().end())
    svc->host_ptr = nullptr;
  else
    svc->host_ptr = it->second.get();

  /* add a reverse link from the host to the service for faster lookups later */
  add_service_link_to_host(svc->host_ptr, svc);

  /* check the event handler command */
  if (!svc->get_event_handler().empty()) {

    /* check the event handler command */
    char* buf = string::dup(svc->get_event_handler());

    /* get the command name, leave any arguments behind */
    char* temp_command_name = my_strtok(buf, "!");

    commands::command* temp_command(
      configuration::applier::state::instance().find_command(temp_command_name));
    if (temp_command == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Event handler command '" << temp_command_name
        << "' specified in service '" << svc->get_description()
        << "' for host '" << svc->get_hostname() << "' not defined anywhere";
      errors++;
    }

    delete[] buf;

    /* save the pointer to the event handler for later */
    svc->event_handler_ptr = temp_command;
  }

  /* check the service check_command */
  char* buf = string::dup(svc->get_check_command());

  /* get the command name, leave any arguments behind */
  char* temp_command_name = my_strtok(buf, "!");

  commands::command* temp_command = configuration::applier::state::instance().find_command(temp_command_name);
  if (temp_command == nullptr) {
    logger(log_verification_error, basic)
      << "Error: Service check command '" << temp_command_name
      << "' specified in service '" << svc->get_description()
      << "' for host '" << svc->get_hostname() << "' not defined anywhere!";
    errors++;
  }

  delete[] buf;

  /* save the pointer to the check command for later */
  svc->check_command_ptr = temp_command;

  // Check for sane recovery options.
  if (svc->get_notifications_enabled()
      && svc->notify_on_recovery
      && !svc->notify_on_warning
      && !svc->notify_on_critical) {
    logger(log_verification_error, basic)
      << "Warning: Recovery notification option in service '"
      << svc->get_description() << "' for host '" << svc->get_hostname()
      << "' doesn't make any sense - specify warning and/or critical "
         "options as well";
    warnings++;
  }

  /* check for valid contacts */
  for (contact_map::iterator
         it(svc->contacts.begin()),
         end(svc->contacts.end());
       it != end;
       ++it) {
    com::centreon::engine::contact* temp_contact
      = configuration::applier::state::instance().find_contact(it->first);

    if (temp_contact == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << it->first
        << "' specified in service '" << svc->get_description() << "' for "
        "host '" << svc->get_hostname() << "' is not defined anywhere!";
      errors++;
    }
  }

  /* check all contact groupss */
  for (contactgroup_map::iterator
         it(svc->contact_groups.begin()),
         end(svc->contact_groups.end());
       it != end;
       ++it) {

    // Find the contact group.
    com::centreon::engine::contactgroup* temp_contactgroup(
      configuration::applier::state::instance().find_contactgroup(
        it->first));

    if (temp_contactgroup == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Contact group '" << it->first
        << "' specified in service '" << svc->get_description() << "' for "
        "host '" << svc->get_hostname() << "' is not defined anywhere!";
      errors++;
    }
  }

  /* verify service check timeperiod */
  if (svc->get_check_period().empty()) {
    logger(log_verification_error, basic)
      << "Warning: Service '" << svc->get_description() << "' on host '"
      << svc->get_hostname() << "' has no check time period defined!";
    warnings++;
  }
  else {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(state::instance().timeperiods().find(svc->get_check_period()));

    if (it != state::instance().timeperiods().end())
      temp_timeperiod = it->second.get();

    if (temp_timeperiod == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Check period '" << svc->get_check_period()
        << "' specified for service '" << svc->get_description()
        << "' on host '" << svc->get_hostname()
        << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the check timeperiod for later */
    svc->check_period_ptr = temp_timeperiod;
  }

  // Check service notification timeperiod.
  if (!svc->get_notification_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(state::instance().timeperiods().find(svc->get_notification_period()));

    if (it != state::instance().timeperiods().end())
      temp_timeperiod = it->second.get();

    if (!temp_timeperiod) {
      logger(log_verification_error, basic)
        << "Error: Notification period '" << svc->get_notification_period()
        << "' specified for service '" << svc->get_description() << "' on "
        "host '" << svc->get_hostname() << "' is not defined anywhere!";
      errors++;
    }

    // Save the pointer to the notification timeperiod for later.
    svc->notification_period_ptr = temp_timeperiod;
  }
  else if (svc->get_notifications_enabled()) {
    logger(log_verification_error, basic)
      << "Warning: Service '" << svc->get_description() << "' on host "
      "'" << svc->get_hostname() << "' has no notification time period "
      "defined!";
    warnings++;
  }

  // See if the notification interval is less than the check interval.
  if (svc->get_notifications_enabled()
      && svc->notification_interval
      && (svc->notification_interval < svc->get_check_interval())) {
    logger(log_verification_error, basic)
      << "Warning: Service '" << svc->get_description() << "' on host '"
      << svc->get_hostname() << "'  has a notification interval less than "
         "its check interval!  Notifications are only re-sent after "
         "checks are made, so the effective notification interval will "
         "be that of the check interval.";
    warnings++;
  }

  /* check for illegal characters in service description */
  if (contains_illegal_object_chars(svc->get_description().c_str())) {
    logger(log_verification_error, basic)
      << "Error: The description string for service '"
      << svc->get_description() << "' on host '" << svc->get_hostname()
      << "' contains one or more illegal characters.";
    errors++;
  }

  if (w != nullptr)
    *w += warnings;
  if (e != nullptr)
    *e += errors;
  return (errors == 0);
}

int check_host(host* hst, int* w, int* e) {
  int warnings(0);
  int errors(0);

  // Make sure each host has at least one service associated with it.
  // Note that this is extremely inefficient. Also note that we are
  // using the global variable /use_large_installation_tweaks/ instead
  // of the global /config/ object because we are in the middle of the
  // configuration application. Therefore the global variable already
  // has its correct value whereas the global object is not yet
  // modified.
  if (!use_large_installation_tweaks) {
    bool found(false);
    for (com::centreon::engine::service* temp_service(service_list);
	       temp_service;
	       temp_service = temp_service->next)
      if (hst->get_name() != temp_service->get_hostname()) {
        found = true;
        break ;
      }

    // We couldn't find a service associated with this host!
    if (!found) {
      logger(log_verification_error, basic)
        << "Warning: Host '" << hst->get_name()
        << "' has no services associated with it!";
      ++warnings;
    }
  }

  /* check the event handler command */
  if (!hst->get_event_handler().empty()) {

    /* check the event handler command */
    char* buf = string::dup(hst->get_event_handler());

    /* get the command name, leave any arguments behind */
    char* temp_command_name = my_strtok(buf, "!");

    commands::command* temp_command = configuration::applier::state::instance().find_command(temp_command_name);
    if (temp_command == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Event handler command '" << temp_command_name
        << "' specified for host '" << hst->get_name()
        << "' not defined anywhere";
      errors++;
    }

    delete[] buf;

    /* save the pointer to the event handler command for later */
    hst->event_handler_ptr = temp_command;
  }

  /* hosts that don't have check commands defined shouldn't ever be checked... */
  if (!hst->get_check_command().empty()) {

    /* check the host check_command */
    char* buf = string::dup(hst->get_check_command());

    /* get the command name, leave any arguments behind */
    char* temp_command_name = my_strtok(buf, "!");

    commands::command* temp_command = configuration::applier::state::instance().find_command(temp_command_name);
    if (temp_command == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Host check command '" << temp_command_name
        << "' specified for host '" << hst->get_name()
        << "' is not defined anywhere!",
      errors++;
    }

    /* save the pointer to the check command for later */
    hst->check_command_ptr = temp_command;

    delete[] buf;
  }

  if (!hst->get_check_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(state::instance().timeperiods().find(hst->get_check_period()));

    if (it != state::instance().timeperiods().end())
      temp_timeperiod = it->second.get();

    if (temp_timeperiod == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Check period '" << hst->get_check_period()
        << "' specified for host '" << hst->get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the check timeperiod for later */
    hst->check_period_ptr = temp_timeperiod;
  }

  /* check all contacts */
  for (contact_map::iterator
         it(hst->contacts.begin()),
         end(hst->contacts.end());
       it != end;
       ++it) {
    com::centreon::engine::contact* temp_contact
      = configuration::applier::state::instance().find_contact(it->first);

    if (temp_contact == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << it->first
        << "' specified in host '" << hst->get_name()
        << "' is not defined anywhere!";
      errors++;
    }
  }

  /* check all contact groups */
  for (contactgroup_map::iterator
         it(hst->contact_groups.begin()),
         end(hst->contact_groups.end());
       it != end;
       ++it) {
    // Find the contact group.
    com::centreon::engine::contactgroup* temp_contactgroup(
      configuration::applier::state::instance().find_contactgroup(
        it->first));

    if (temp_contactgroup == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Contact group '"
        << it->first
        << "' specified in host '" << hst->get_name()
        << "' is not defined anywhere!";
      errors++;
    }
  }

  // Check notification timeperiod.
  if (hst->get_notification_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(state::instance().timeperiods().find(hst->get_notification_period()));

    if (it != state::instance().timeperiods().end())
      temp_timeperiod = it->second.get();

    if (!temp_timeperiod) {
      logger(log_verification_error, basic)
        << "Error: Notification period '" << hst->get_notification_period()
        << "' specified for host '" << hst->get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save the pointer to the notification timeperiod for later.
    hst->notification_period_ptr = temp_timeperiod;
  }

  /* check all parent parent host */
  for (host_map::iterator
         it(hst->parent_hosts.begin()),
         end(hst->parent_hosts.end());
       it != end;
       it++) {

    host* hst2 = nullptr;
    umap<unsigned long, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it_host(state::instance().hosts().find(get_host_id(it->first)));

    if (it_host == state::instance().hosts().end() || it_host->second == nullptr) {
      logger(log_verification_error, basic)
        << "Error: '" << it->first << "' is not a "
        "valid parent for host '" << hst->get_name() << "'!";
      errors++;
    }
    else
      it_host->second->add_child_link(hst); //add a reverse (child) link to make searches faster later on
  }

  // Check for sane recovery options.
  if (hst->get_notifications_enabled()
      && hst->get_notify_on(notifier::recovery)
      && !hst->get_notify_on(notifier::down)
      && !hst->get_notify_on(notifier::unreachable)) {
    logger(log_verification_error, basic)
      << "Warning: Recovery notification option in host '" << hst->get_name()
      << "' definition doesn't make any sense - specify down and/or "
         "unreachable options as well";
    warnings++;
  }

  /* check for illegal characters in host name */
  if (contains_illegal_object_chars(hst->get_name().c_str())) {
    logger(log_verification_error, basic)
      << "Error: The name of host '" << hst->get_name()
      << "' contains one or more illegal characters.";
    errors++;
  }

  if (w != nullptr)
    *w += warnings;
  if (e != nullptr)
    *e += errors;
  return (errors == 0);
}

int check_contact(com::centreon::engine::contact* cntct, int* w, int* e) {
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
      it(state::instance().timeperiods().find(cntct->get_service_notification_period()));

    if (it != state::instance().timeperiods().end())
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
      it(state::instance().timeperiods().find(cntct->get_host_notification_period()));

    if (it != state::instance().timeperiods().end())
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
  if (cntct->notify_on_host_recovery()
      && !cntct->notify_on_host_down()
      && !cntct->notify_on_host_unreachable()) {
    logger(log_verification_error, basic)
      << "Warning: Host recovery notification option for contact '"
      << cntct->get_name() << "' doesn't make any sense - specify down "
      "and/or unreachable options as well";
    warnings++;
  }

  /* check for sane service recovery options */
  if (cntct->notify_on_service_recovery()
      && !cntct->notify_on_service_critical()
      && !cntct->notify_on_service_warning()) {
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
  return (errors == 0);
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
int check_servicegroup(servicegroup* sg, int* w, int* e) {
  (void)w;
  int errors(0);

  // Check all group members.
  for (servicesmember* temp_servicesmember(sg->members);
       temp_servicesmember;
       temp_servicesmember = temp_servicesmember->next) {
    com::centreon::engine::service* temp_service(find_service(
                            temp_servicesmember->host_name,
                            temp_servicesmember->service_description));
    if (!temp_service) {
      logger(log_verification_error, basic)
        << "Error: Service '"
        << temp_servicesmember->service_description
        << "' on host '" << temp_servicesmember->host_name
        << "' specified in service group '" << sg->get_group_name()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save a pointer to this servicegroup for faster service/group
    // membership lookups later.
    else
      add_object_to_objectlist(&temp_service->servicegroups_ptr, sg);

    // Save service pointer for later.
    temp_servicesmember->service_ptr = temp_service;
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

  return (errors == 0);
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
int check_hostgroup(hostgroup* hg, int* w, int* e) {
  (void)w;
  int errors(0);

  // Check all group members.
  for (host_map::iterator
         it(hg->members.begin()),
         end(hg->members.end());
       it != end;
       ++it) {

    umap<unsigned long, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it_host(state::instance().hosts().find(get_host_id(it->first.c_str())));
    if (it_host == state::instance().hosts().end() || it_host->second == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Host '" << it->first
        << "' specified in host group '" << hg->get_group_name()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save a pointer to this hostgroup for faster host/group
    // membership lookups later.
    else
      add_object_to_objectlist(&it_host->second->hostgroups_ptr, hg);
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

  return (errors == 0);
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
int check_contactgroup(contactgroup* cg, int* w, int* e) {
  (void)w;
  int errors(0);

  for (std::unordered_map<std::string, contact *>::const_iterator
         it(cg->get_members().begin()),
         end(cg->get_members().end());
       it != end;
       ++it) {
    contact *temp_contact(
      configuration::applier::state::instance().find_contact(
        it->first));
    if (temp_contact == nullptr) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << it->first
        << "' specified in contact group '" << cg->get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    // Save a pointer to this contact group for faster contact/group
    // membership lookups later.
    else
      add_object_to_objectlist(&temp_contact->contactgroups_ptr, cg);

    // Save the contact pointer for later.
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

  return (errors == 0);
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
int check_servicedependency(servicedependency* sd, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the dependent service.
  com::centreon::engine::service* temp_service(find_service(
                          sd->get_dependent_hostname().c_str(),
                          sd->get_dependent_service_description().c_str()));
  if (!temp_service) {
    logger(log_verification_error, basic)
      << "Error: Dependent service '"
      << sd->get_dependent_service_description() << "' on host '"
      << sd->get_dependent_hostname()
      << "' specified in service dependency for service '"
      << sd->get_service_description() << "' on host '"
      << sd->get_hostname() << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  sd->dependent_service_ptr = temp_service;

  // Find the service we're depending on.
  temp_service = find_service(
                   sd->get_hostname().c_str(),
                   sd->get_service_description().c_str());
  if (!temp_service) {
    logger(log_verification_error, basic)
      << "Error: Service '" << sd->get_service_description() << "' on host '"
      << sd->get_hostname()
      << "' specified in service dependency for service '"
      << sd->get_dependent_service_description() << "' on host '"
      << sd->get_dependent_hostname() << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  sd->master_service_ptr = temp_service;

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
      it(state::instance().timeperiods().find(sd->get_dependency_period()));

    if (it != state::instance().timeperiods().end())
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

  return (errors == 0);
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
int check_hostdependency(hostdependency* hd, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the dependent host.
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(state::instance().hosts().find(get_host_id(hd->get_dependent_hostname())));
  if (it == state::instance().hosts().end() || it->second == nullptr) {
    logger(log_verification_error, basic)
      << "Error: Dependent host specified in host dependency for "
         "host '" << hd->get_dependent_hostname()
      << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  if (it == state::instance().hosts().end())
    hd->dependent_host_ptr = nullptr;
  else
    hd->dependent_host_ptr = it->second.get();

  // Find the host we're depending on.
  it = state::instance().hosts().find(get_host_id(hd->get_hostname()));
  if (it == state::instance().hosts().end() || it->second == nullptr) {
    logger(log_verification_error, basic)
      << "Error: Host specified in host dependency for host '"
      << hd->get_dependent_hostname() << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  if (it == state::instance().hosts().end())
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
      it(state::instance().timeperiods().find(hd->get_dependency_period()));

    if (it != state::instance().timeperiods().end())
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
    hd->dependency_period_ptr = temp_timeperiod;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
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
int check_serviceescalation(serviceescalation* se, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the service.
  com::centreon::engine::service* temp_service(find_service(se->get_hostname().c_str(), se->get_description().c_str()));
  if (!temp_service) {
    logger(log_verification_error, basic) << "Error: Service '"
        << se->get_description() << "' on host '" << se->get_hostname()
        << "' specified in service escalation is not defined anywhere!";
    errors++;
  }

  // Save the service pointer for later.
  se->notifier_ptr = temp_service;

  // Find the timeperiod.
  if (!se->get_escalation_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(state::instance().timeperiods().find(se->get_escalation_period()));

    if (it != state::instance().timeperiods().end())
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
  for (std::pair<std::string, std::shared_ptr<contact>> const& p : se->contacts()) {
    contact* cntct{configuration::applier::state::instance().find_contact(
      p.first)};
    if (!cntct) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << p.first
        << "' specified in service escalation for service '"
        << se->get_description() << "' on host '"
        << se->get_hostname() << "' is not defined anywhere!";
      errors++;
    }
  }

  // Check all contact groups.
  for (contactgroup_map::iterator
         it(se->contact_groups.begin()),
         end(se->contact_groups.end());
       it != end;
       ++it) {
    // Find the contact group.
    com::centreon::engine::contactgroup* temp_contactgroup(
      configuration::applier::state::instance().find_contactgroup(
        it->first));

    if (!temp_contactgroup) {
      logger(log_verification_error, basic)
        << "Error: Contact group '"
        << it->first
        << "' specified in service escalation for service '"
        << se->get_description() << "' on host '" << se->get_hostname()
        << "' is not defined anywhere!";
      errors++;
    }
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
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
int check_hostescalation(hostescalation* he, int* w, int* e) {
  (void)w;
  int errors(0);

  // Find the host.
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(state::instance().hosts().find(get_host_id(he->get_hostname())));
  if (it == state::instance().hosts().end() || it->second == nullptr) {
    logger(log_verification_error, basic)
      << "Error: Host '" << he->get_hostname()
      << "' specified in host escalation is not defined anywhere!";
    errors++;
  }

  // Save the host pointer for later.
  if (it == state::instance().hosts().end())
    he->notifier_ptr = nullptr;
  else
    he->notifier_ptr = it->second.get();

  // Find the timeperiod.
  if (!he->get_escalation_period().empty()) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      it(state::instance().timeperiods().find(he->get_escalation_period()));

    if (it != state::instance().timeperiods().end())
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
  for (std::pair<std::string, std::shared_ptr<contact>> const& p : he->contacts()) {
    // Find the contact.
    contact* cntct{configuration::applier::state::instance().find_contact(
                            p.first)};
    if (!cntct) {
      logger(log_verification_error, basic)
        << "Error: Contact '" << p.first
        << "' specified in host escalation for host '"
        << he->get_hostname() << "' is not defined anywhere!";
      errors++;
    }
  }

  // Check all contact groups.
  for (contactgroup_map::iterator
         it(he->contact_groups.begin()),
         end(he->contact_groups.end());
       it != end;
       ++it) {
    // Find the contact group.
    com::centreon::engine::contactgroup* temp_contactgroup(
      configuration::applier::state::instance().find_contactgroup(
        it->first));

    if (!temp_contactgroup) {
      logger(log_verification_error, basic)
        << "Error: Contact group '"
        << it->first
        << "' specified in host escalation for host '"
        << he->get_hostname() << "' is not defined anywhere!";
      errors++;
    }
  }

  // Add errors.
  if (e)
    *e += errors;
  return (errors == 0);
}

/**
 *  Check and resolve a time period.
 *
 *  @param[in,out] tp Time period object.
 *  @param[out]    w  Warnings.
 *  @param[out]    e  Errors.
 *
 *  @return Non-zero on success.
 */
int check_timeperiod(timeperiod* tp, int* w, int* e) {
  (void)w;
  int errors(0);

  // Check for illegal characters in timeperiod name.
  if (contains_illegal_object_chars(tp->get_name().c_str())) {
    logger(log_verification_error, basic)
      << "Error: The name of time period '" << tp->get_name()
      << "' contains one or more illegal characters.";
    errors++;
  }

  // Check for valid timeperiod names in exclusion list.
  for (timeperiodexclusion::iterator
         it(tp->exclusions.begin()),
         end(tp->exclusions.end());
       it != end;
       ++it) {
    timeperiod* temp_timeperiod(nullptr);
    timeperiod_map::const_iterator
      found(state::instance().timeperiods().find(it->first));

    if (found == state::instance().timeperiods().end()) {
      logger(log_verification_error, basic)
        << "Error: Excluded time period '"
        << it->first
        << "' specified in timeperiod '" << tp->get_name()
        << "' is not defined anywhere!";
      errors++;
    }

    it->second = found->second;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}
