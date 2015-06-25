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

#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "config.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

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
