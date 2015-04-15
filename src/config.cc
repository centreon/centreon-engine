/*
** Copyright 1999-2008 Ethan Galstad
** Copyright 2011-2015 Merethis
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
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/****************************************************************/
/**************** CONFIG VERIFICATION FUNCTIONS *****************/
/****************************************************************/

/* do a pre-flight check to make sure object relationships, etc. make sense */
int pre_flight_check() {
  host* temp_host(NULL);
  char* buf(NULL);
  service* temp_service(NULL);
  command* temp_command(NULL);
  char* temp_command_name(NULL);
  int warnings(0);
  int errors(0);
  struct timeval tv[4];
  double runtime[4];

  if (test_scheduling == true)
    gettimeofday(&tv[0], NULL);

  /********************************************/
  /* check object relationships               */
  /********************************************/
  pre_flight_object_check(&warnings, &errors);
  if (test_scheduling == true)
    gettimeofday(&tv[1], NULL);

  /********************************************/
  /* check for circular paths between hosts   */
  /********************************************/
  pre_flight_circular_check(&warnings, &errors);
  if (test_scheduling == true)
    gettimeofday(&tv[2], NULL);

  /********************************************/
  /* check global event handler commands...   */
  /********************************************/
  if (verify_config == true)
    printf("Checking global event handlers...\n");

  if (config->global_host_event_handler() != "") {

    /* check the event handler command */
    buf = string::dup(config->global_host_event_handler());

    /* get the command name, leave any arguments behind */
    temp_command_name = my_strtok(buf, "!");

    temp_command = find_command(temp_command_name);
    if (temp_command == NULL) {
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

    temp_command = find_command(temp_command_name);
    if (temp_command == NULL) {
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
  if (verify_config == true)
    printf("Checking obsessive compulsive processor commands...\n");

  if (!config->ocsp_command().empty()) {

    buf = string::dup(config->ocsp_command());

    /* get the command name, leave any arguments behind */
    temp_command_name = my_strtok(buf, "!");

    temp_command = find_command(temp_command_name);
    if (temp_command == NULL) {
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

    temp_command = find_command(temp_command_name);
    if (temp_command == NULL) {
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
       temp_service != NULL;
       temp_service = temp_service->next) {
    if ((temp_host = find_host(temp_service->host_name))) {
      temp_host->total_services++;
      temp_host->total_service_check_interval
        += static_cast<unsigned long>(temp_service->check_interval);
    }
  }

  if (verify_config == true) {
    printf("\n");
    printf("Total Warnings: %d\n", warnings);
    printf("Total Errors:   %d\n", errors);
  }

  if (test_scheduling == true)
    gettimeofday(&tv[3], NULL);

  if (test_scheduling == true) {

    runtime[0]
      = (double)((double)(tv[1].tv_sec - tv[0].tv_sec)
                 + (double)((tv[1].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);
    if (verify_circular_paths == true)
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
  if (verify_config == true)
    logger(log_info_message, basic) << "Checking services...";
  int total_objects(0);
  for (service* temp_service(service_list);
       temp_service;
       temp_service = temp_service->next, ++total_objects)
    check_service(temp_service, &warnings, &errors);
  if (verify_config == true)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " services.";

  // Check all hosts...
  if (verify_config == true)
    logger(log_info_message, basic) << "Checking hosts...";
  total_objects = 0;
  for (host* temp_host(host_list);
       temp_host;
       temp_host = temp_host->next, ++total_objects)
    check_host(temp_host, &warnings, &errors);
  if (verify_config == true)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " hosts.";

  // Check all service dependencies...
  if (verify_config == true)
    logger(log_info_message, basic)
      << "Checking service dependencies...";
  total_objects = 0;
  for (servicedependency* temp_sd(servicedependency_list);
       temp_sd;
       temp_sd = temp_sd->next, ++total_objects)
    check_servicedependency(temp_sd, &warnings, &errors);
  if (verify_config == true)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " service dependencies.";

  // Check all host dependencies...
  if (verify_config == true)
    logger(log_info_message, basic) << "Checking host dependencies...";
  total_objects = 0;
  for (hostdependency* temp_hd(hostdependency_list);
       temp_hd;
       temp_hd = temp_hd->next, ++total_objects)
    check_hostdependency(temp_hd, &warnings, &errors);
  if (verify_config == true)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " host dependencies.";

  // Check all commands...
  if (verify_config == true)
    logger(log_info_message, basic) << "Checking commands...";
  total_objects = 0;
  for (command* temp_command(command_list);
       temp_command;
       temp_command = temp_command->next, ++total_objects) {
    // Check for illegal characters in command name.
    if ((contains_illegal_object_chars(temp_command->name) == true)) {
      logger(log_verification_error, basic)
        << "Error: The name of command '" << temp_command->name
        << "' contains one or more illegal characters.";
      errors++;
    }
  }
  if (verify_config == true)
    logger(log_info_message, basic)
      << "\tChecked " << total_objects << " commands.\n";

  // Check all timeperiods...
  if (verify_config == true)
    logger(log_info_message, basic) << "Checking time periods...";
  total_objects = 0;
  for (timeperiod* temp_timeperiod(timeperiod_list);
       temp_timeperiod;
       temp_timeperiod = temp_timeperiod->next, ++total_objects)
    check_timeperiod(temp_timeperiod, &warnings, &errors);
  if (verify_config == true)
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

#define dfs_get_status(h) h->circular_path_checked
#define dfs_unset_status(h) h->circular_path_checked = 0
#define dfs_set_status(h, flag) h->circular_path_checked = (flag)
#define dfs_host_status(h) (h ? dfs_get_status(h) : DFS_OK)

/**
 * Modified version of Depth-first Search
 * http://en.wikipedia.org/wiki/Depth-first_search
 */
static int dfs_host_path(host* root) {
  hostsmember* child(NULL);

  if (!root)
    return (DFS_NEAR_LOOP);

  if (dfs_get_status(root) != DFS_UNCHECKED)
    return (dfs_get_status(root));

  /* Mark the root temporary checked */
  dfs_set_status(root, DFS_TEMP_CHECKED);

  /* We are scanning the children */
  for (child = root->child_hosts; child != NULL; child = child->next) {
    int child_status = dfs_get_status(child->host_ptr);

    /* If a child is not checked, check it */
    if (child_status == DFS_UNCHECKED)
      child_status = dfs_host_path(child->host_ptr);

    /* If a child already temporary checked, its a problem,
     * loop inside, and its a acked status */
    if (child_status == DFS_TEMP_CHECKED) {
      dfs_set_status(child->host_ptr, DFS_LOOPY);
      dfs_set_status(root, DFS_LOOPY);
    }

    /* If a child already temporary checked, its a problem, loop inside */
    if (child_status == DFS_NEAR_LOOP || child_status == DFS_LOOPY) {
      /* if a node is know to be part of a loop, do not let it be less */
      if (dfs_get_status(root) != DFS_LOOPY)
        dfs_set_status(root, DFS_NEAR_LOOP);

      /* we already saw this child, it's a problem */
      dfs_set_status(child->host_ptr, DFS_LOOPY);
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
  host* temp_host(NULL);
  servicedependency* temp_sd(NULL);
  servicedependency* temp_sd2(NULL);
  hostdependency* temp_hd(NULL);
  hostdependency* temp_hd2(NULL);
  int found(false);
  int warnings(0);
  int errors(0);

  /* bail out if we aren't supposed to verify circular paths */
  if (verify_circular_paths == false)
    return (OK);

  /********************************************/
  /* check for circular paths between hosts   */
  /********************************************/

  /* check routes between all hosts */
  found = false;

  /* We clean the dsf status from previous check */
  for (temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    dfs_set_status(temp_host, DFS_UNCHECKED);
  }

  for (temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    if (dfs_host_path(temp_host) == DFS_LOOPY)
      errors = 1;
  }

  for (temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    if (dfs_get_status(temp_host) == DFS_LOOPY)
      logger(log_verification_error, basic)
        << "Error: The host '" << temp_host->name
        << "' is part of a circular parent/child chain!";
    /* clean DFS status */
    dfs_set_status(temp_host, DFS_UNCHECKED);
  }

  /********************************************/
  /* check for circular dependencies         */
  /********************************************/

  /* check execution dependencies between all services */
  for (temp_sd = servicedependency_list;
       temp_sd != NULL;
       temp_sd = temp_sd->next) {

    /* clear checked flag for all dependencies */
    for (temp_sd2 = servicedependency_list;
	 temp_sd2 != NULL;
         temp_sd2 = temp_sd2->next)
      temp_sd2->circular_path_checked = false;

    found = check_for_circular_servicedependency_path(
              temp_sd,
              temp_sd);
    if (found == true) {
      logger(log_verification_error, basic)
        << "Error: A circular execution dependency (which could result "
        "in a deadlock) exists for service '"
        << temp_sd->service_description << "' on host '"
        << temp_sd->host_name << "'!";
      errors++;
    }
  }

  /* clear checked flag for all dependencies */
  for (temp_sd = servicedependency_list;
       temp_sd != NULL;
       temp_sd = temp_sd->next)
    temp_sd->circular_path_checked = false;

  /* check execution dependencies between all hosts */
  for (temp_hd = hostdependency_list;
       temp_hd != NULL;
       temp_hd = temp_hd->next) {

    /* clear checked flag for all dependencies */
    for (temp_hd2 = hostdependency_list;
	 temp_hd2 != NULL;
         temp_hd2 = temp_hd2->next)
      temp_hd2->circular_path_checked = false;

    found = check_for_circular_hostdependency_path(
              temp_hd,
              temp_hd);
    if (found == true) {
      logger(log_verification_error, basic)
        << "Error: A circular execution dependency (which could "
        "result in a deadlock) exists for host '"
        << temp_hd->host_name << "'!";
      errors++;
    }
  }

  /* clear checked flag for all dependencies */
  for (temp_hd = hostdependency_list;
       temp_hd != NULL;
       temp_hd = temp_hd->next)
    temp_hd->circular_path_checked = false;

  /* update warning and error count */
  if (w != NULL)
    *w += warnings;
  if (e != NULL)
    *e += errors;

  return ((errors > 0) ? ERROR : OK);
}

int check_service(service* svc, int* w, int* e) {
  int errors(0);
  int warnings(0);

  /* check for a valid host */
  host* temp_host = find_host(svc->host_name);

  /* we couldn't find an associated host! */

  if (!temp_host) {
    logger(log_verification_error, basic)
      << "Error: Host '" << svc->host_name << "' specified in service "
      "'" << svc->description << "' not defined anywhere!";
    errors++;
  }

  /* save the host pointer for later */
  svc->host_ptr = temp_host;

  /* add a reverse link from the host to the service for faster lookups later */
  add_service_link_to_host(temp_host, svc);

  /* check the event handler command */
  if (svc->event_handler != NULL) {

    /* check the event handler command */
    char* buf = string::dup(svc->event_handler);

    /* get the command name, leave any arguments behind */
    char* temp_command_name = my_strtok(buf, "!");

    command* temp_command = find_command(temp_command_name);
    if (temp_command == NULL) {
      logger(log_verification_error, basic)
        << "Error: Event handler command '" << temp_command_name
        << "' specified in service '" << svc->description
        << "' for host '" << svc->host_name << "' not defined anywhere";
      errors++;
    }

    delete[] buf;

    /* save the pointer to the event handler for later */
    svc->event_handler_ptr = temp_command;
  }

  /* check the service check_command */
  char* buf = string::dup(svc->service_check_command);

  /* get the command name, leave any arguments behind */
  char* temp_command_name = my_strtok(buf, "!");

  command* temp_command = find_command(temp_command_name);
  if (temp_command == NULL) {
    logger(log_verification_error, basic)
      << "Error: Service check command '" << temp_command_name
      << "' specified in service '" << svc->description
      << "' for host '" << svc->host_name << "' not defined anywhere!";
    errors++;
  }

  delete[] buf;

  /* save the pointer to the check command for later */
  svc->check_command_ptr = temp_command;

  /* verify service check timeperiod */
  if (svc->check_period == NULL) {
    logger(log_verification_error, basic)
      << "Warning: Service '" << svc->description << "' on host '"
      << svc->host_name << "' has no check time period defined!";
    warnings++;
  }
  else {
    timeperiod* temp_timeperiod = find_timeperiod(svc->check_period);
    if (temp_timeperiod == NULL) {
      logger(log_verification_error, basic)
        << "Error: Check period '" << svc->check_period
        << "' specified for service '" << svc->description
        << "' on host '" << svc->host_name
        << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the check timeperiod for later */
    svc->check_period_ptr = temp_timeperiod;
  }

  /* check for illegal characters in service description */
  if (contains_illegal_object_chars(svc->description) == true) {
    logger(log_verification_error, basic)
      << "Error: The description string for service '"
      << svc->description << "' on host '" << svc->host_name
      << "' contains one or more illegal characters.";
    errors++;
  }

  if (w != NULL)
    *w += warnings;
  if (e != NULL)
    *e += errors;
  return (errors == 0);
}

int check_host(host* hst, int* w, int* e) {
  int warnings(0);
  int errors(0);

  /* check the event handler command */
  if (hst->event_handler != NULL) {

    /* check the event handler command */
    char* buf = string::dup(hst->event_handler);

    /* get the command name, leave any arguments behind */
    char* temp_command_name = my_strtok(buf, "!");

    command* temp_command = find_command(temp_command_name);
    if (temp_command == NULL) {
      logger(log_verification_error, basic)
        << "Error: Event handler command '" << temp_command_name
        << "' specified for host '" << hst->name
        << "' not defined anywhere";
      errors++;
    }

    delete[] buf;

    /* save the pointer to the event handler command for later */
    hst->event_handler_ptr = temp_command;
  }

  /* hosts that don't have check commands defined shouldn't ever be checked... */
  if (hst->host_check_command != NULL) {

    /* check the host check_command */
    char* buf = string::dup(hst->host_check_command);

    /* get the command name, leave any arguments behind */
    char* temp_command_name = my_strtok(buf, "!");

    command* temp_command = find_command(temp_command_name);
    if (temp_command == NULL) {
      logger(log_verification_error, basic)
        << "Error: Host check command '" << temp_command_name
        << "' specified for host '" << hst->name
        << "' is not defined anywhere!",
      errors++;
    }

    /* save the pointer to the check command for later */
    hst->check_command_ptr = temp_command;

    delete[] buf;
  }

  /* check host check timeperiod */
  if (hst->check_period != NULL) {
    timeperiod* temp_timeperiod = find_timeperiod(hst->check_period);
    if (temp_timeperiod == NULL) {
      logger(log_verification_error, basic)
        << "Error: Check period '" << hst->check_period
        << "' specified for host '" << hst->name
        << "' is not defined anywhere!";
      errors++;
    }

    /* save the pointer to the check timeperiod for later */
    hst->check_period_ptr = temp_timeperiod;
  }

  /* check all parent parent host */
  for (hostsmember* temp_hostsmember = hst->parent_hosts;
       temp_hostsmember != NULL;
       temp_hostsmember = temp_hostsmember->next) {

    host* hst2 = NULL;
    if ((hst2 = find_host(temp_hostsmember->host_name)) == NULL) {
      logger(log_verification_error, basic)
        << "Error: '" << temp_hostsmember->host_name << "' is not a "
        "valid parent for host '" << hst->name << "'!";
      errors++;
    }

    /* save the parent host pointer for later */
    temp_hostsmember->host_ptr = hst2;

    /* add a reverse (child) link to make searches faster later on */
    add_child_link_to_host(hst2, hst);
  }

  /* check for illegal characters in host name */
  if (contains_illegal_object_chars(hst->name) == true) {
    logger(log_verification_error, basic)
      << "Error: The name of host '" << hst->name
      << "' contains one or more illegal characters.";
    errors++;
  }

  if (w != NULL)
    *w += warnings;
  if (e != NULL)
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
  service* temp_service(find_service(
                          sd->dependent_host_name,
                          sd->dependent_service_description));
  if (!temp_service) {
    logger(log_verification_error, basic)
      << "Error: Dependent service '"
      << sd->dependent_service_description << "' on host '"
      << sd->dependent_host_name
      << "' specified in service dependency for service '"
      << sd->service_description << "' on host '"
      << sd->host_name << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  sd->dependent_service_ptr = temp_service;

  // Find the service we're depending on.
  temp_service = find_service(
                   sd->host_name,
                   sd->service_description);
  if (!temp_service) {
    logger(log_verification_error, basic)
      << "Error: Service '" << sd->service_description << "' on host '"
      << sd->host_name
      << "' specified in service dependency for service '"
      << sd->dependent_service_description << "' on host '"
      << sd->dependent_host_name << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  sd->master_service_ptr = temp_service;

  // Make sure they're not the same service.
  if (sd->dependent_service_ptr == sd->master_service_ptr) {
    logger(log_verification_error, basic)
      << "Error: Service dependency definition for service '"
      << sd->dependent_service_description << "' on host '"
      << sd->dependent_host_name
      << "' is circular (it depends on itself)!";
    errors++;
  }

  // Find the timeperiod.
  if (sd->dependency_period) {
    timeperiod* temp_timeperiod(find_timeperiod(sd->dependency_period));
    if (!temp_timeperiod) {
      logger(log_verification_error, basic)
        << "Error: Dependency period '" << sd->dependency_period
        << "' specified in service dependency for service '"
        << sd->dependent_service_description << "' on host '"
        << sd->dependent_host_name << "' is not defined anywhere!";
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
  host* temp_host(find_host(hd->dependent_host_name));
  if (!temp_host) {
    logger(log_verification_error, basic)
      << "Error: Dependent host specified in host dependency for "
         "host '" << hd->dependent_host_name
      << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  hd->dependent_host_ptr = temp_host;

  // Find the host we're depending on.
  temp_host = find_host(hd->host_name);
  if (!temp_host) {
    logger(log_verification_error, basic)
      << "Error: Host specified in host dependency for host '"
      << hd->dependent_host_name << "' is not defined anywhere!";
    errors++;
  }

  // Save pointer for later.
  hd->master_host_ptr = temp_host;

  // Make sure they're not the same host.
  if (hd->dependent_host_ptr == hd->master_host_ptr) {
    logger(log_verification_error, basic)
      << "Error: Host dependency definition for host '"
      << hd->dependent_host_name
      << "' is circular (it depends on itself)!";
    errors++;
  }

  // Find the timeperiod.
  if (hd->dependency_period) {
    timeperiod* temp_timeperiod(find_timeperiod(hd->dependency_period));
    if (!temp_timeperiod) {
      logger(log_verification_error, basic)
        << "Error: Dependency period '" << hd->dependency_period
        << "' specified in host dependency for host '"
        << hd->dependent_host_name
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
  if (contains_illegal_object_chars(tp->name) == true) {
    logger(log_verification_error, basic)
      << "Error: The name of time period '" << tp->name
      << "' contains one or more illegal characters.";
    errors++;
  }

  // Check for valid timeperiod names in exclusion list.
  for (timeperiodexclusion*
         temp_timeperiodexclusion(tp->exclusions);
       temp_timeperiodexclusion;
       temp_timeperiodexclusion = temp_timeperiodexclusion->next) {
    timeperiod* temp_timeperiod2(
                  find_timeperiod(
                    temp_timeperiodexclusion->timeperiod_name));
    if (!temp_timeperiod2) {
      logger(log_verification_error, basic)
        << "Error: Excluded time period '"
        << temp_timeperiodexclusion->timeperiod_name
        << "' specified in timeperiod '" << tp->name
        << "' is not defined anywhere!";
      errors++;
    }

    // Save the timeperiod pointer for later.
    temp_timeperiodexclusion->timeperiod_ptr = temp_timeperiod2;
  }

  // Add errors.
  if (e)
    *e += errors;

  return (errors == 0);
}
