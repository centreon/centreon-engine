/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
** Copyright 2011      Merethis
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

#include "errno.h"
#include <exception>
#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "comments.hh"
#include "downtime.hh"
#include "statusdata.hh"
#include "sretention.hh"
#include "perfdata.hh"
#include "broker.hh"
#include "nebmods.hh"
#include "notifications.hh"
#include "config.hh"
#include "utils.hh"
#include "logging.hh"
#include "configuration/states.hh"
#include "modules/loader.hh"
#include "engine.hh"

using namespace com::centreon::engine;

// Error message when configuration parsing fail.
#define ERROR_CONFIGURATION "    Check your configuration file(s) to ensure that they contain valid\n" \
                            "    directives and data defintions. If you are upgrading from a\n" \
                            "    previous version of Centreon Engine, you should be aware that some\n" \
                            "    variables/definitions may have been removed or modified in this\n" \
                            "    version. Make sure to read the documentation regarding the config\n" \
                            "    files, as well as the version changelog to find out what has\n" \
                            "    changed.\n\n"

// Global variables.
configuration::states     config;

char*                     config_file = NULL;

command*                  global_host_event_handler_ptr = NULL;
command*                  global_service_event_handler_ptr = NULL;

command*                  ocsp_command_ptr = NULL;
command*                  ochp_command_ptr = NULL;

unsigned long             logging_options = 0;
unsigned long             syslog_options = 0;

time_t                    last_command_check = 0L;
time_t                    last_command_status_update = 0L;
time_t                    last_log_rotation = 0L;

unsigned long             modified_host_process_attributes = MODATTR_NONE;
unsigned long             modified_service_process_attributes = MODATTR_NONE;

unsigned long             next_comment_id = 0L;
unsigned long             next_downtime_id = 0L;
unsigned long             next_event_id = 0L;
unsigned long             next_problem_id = 0L;
unsigned long             next_notification_id = 0L;

int                       sigshutdown = FALSE;
int                       sigrestart = FALSE;

char const*               sigs[35] = {
  "EXIT", "HUP", "INT", "QUIT", "ILL",
  "TRAP", "ABRT", "BUS", "FPE", "KILL",
  "USR1", "SEGV", "USR2", "PIPE", "ALRM",
  "TERM", "STKFLT", "CHLD", "CONT", "STOP",
  "TSTP", "TTIN", "TTOU", "URG", "XCPU",
  "XFSZ", "VTALRM", "PROF", "WINCH", "IO",
  "PWR", "UNUSED", "ZERR", "DEBUG", NULL
};

int                       caught_signal = FALSE;
int                       sig_id = 0;

int                       restarting = FALSE;

int                       verify_config = FALSE;
int                       verify_circular_paths = TRUE;
int                       test_scheduling = FALSE;
int                       precache_objects = FALSE;
int                       use_precached_objects = FALSE;

unsigned int              currently_running_service_checks = 0;
unsigned int              currently_running_host_checks = 0;

time_t                    program_start = 0L;
time_t                    event_start = 0L;
int                       nagios_pid = 0;

int                       embedded_perl_initialized = FALSE;

int                       command_file_fd;
FILE*                     command_file_fp;
int                       command_file_created = FALSE;


extern contact*           contact_list;
extern contactgroup*      contactgroup_list;
extern hostgroup*         hostgroup_list;
extern command*           command_list;
extern timeperiod*        timeperiod_list;
extern serviceescalation* serviceescalation_list;

notification*             notification_list;

check_result              check_result_info;
check_result*             check_result_list = NULL;

dbuf                      check_result_dbuf;

circular_buffer           external_command_buffer;
circular_buffer           check_result_buffer;
pthread_t                 worker_threads[TOTAL_WORKER_THREADS];

check_stats               check_statistics[MAX_CHECK_STATS_TYPES];

// Following main() declaration required by older versions of Perl ut 5.00503.
#ifdef EMBEDDEDPERL
int main(int argc, char** argv, char** env) {
#else
int main(int argc, char** argv) {
#endif // EMBEDDEDPERL
  int error = FALSE;
  int display_license = FALSE;
  int display_help = FALSE;
  int c = 0;
  struct tm* tm, tm_s;
  time_t now;
  char datestring[256];
  nagios_macros* mac;

  // Get global macros.
  mac = get_global_macros();

#ifdef HAVE_GETOPT_H
  int option_index = 0;
  static struct option long_options[] = {
    {"dont-verify-paths", no_argument, NULL, 'x'},
    {"help", no_argument, NULL, 'h'},
    {"license", no_argument, NULL, 'V'},
    {"precache-objects", no_argument, NULL, 'p'},
    {"test-scheduling", no_argument, NULL, 's'},
    {"use-precached-objects", no_argument, NULL, 'u'},
    {"verify-config", no_argument, NULL, 'v'},
    {"version", no_argument, NULL, 'V'},
    {NULL, no_argument, NULL, '\0'}
  };
#endif // HAVE_GETOPT_H

  // Make sure we have the correct number of command line arguments.
  if (argc < 2)
    error = TRUE;

  // Process all command line arguments.
#ifdef HAVE_GETOPT_H
  while ((c = getopt_long(argc, argv, "+hVvsxpu", long_options, &option_index)) != -1) {
#else
  while ((c = getopt(argc, argv, "+hVvsxpu")) != -1) {
#endif // HAVE_GETOPT_H

    // Process flag.
    switch (c) {
     case '?': // Usage.
     case 'h':
      display_help = TRUE;
      break ;
     case 'V': // Version.
      display_license = TRUE;
      break ;
     case 'v': // Verify config.
      verify_config = TRUE;
      break ;
     case 's': // Scheduling Check.
      test_scheduling = TRUE;
      break ;
     case 'x': // Don't verify circular paths.
      verify_circular_paths = FALSE;
      break ;
     case 'p': // Precache object config.
      precache_objects = TRUE;
      break ;
     case 'u': // Use precached object config.
      use_precached_objects = TRUE;
      break ;
     default:
      break ;
    }
  }

  // Make sure we have the right combination of arguments.
  if ((TRUE == precache_objects)
      && (FALSE == test_scheduling)
      && (FALSE == verify_config)) {
    error = TRUE;
    display_help = TRUE;
  }

  // Just display the license.
  if (TRUE == display_license) {
    std:: cout << "Centreon Engine is free software: you can redistribute it and/or modify\n"
                  "it under the terms of the GNU General Public License version 2 as\n"
                  "published by the Free Software Foundation.\n"
                  "\n"
                  "Centreon Engine is distributed in the hope that it will be useful, but\n"
                  "WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU\n"
                  "General Public License for more details.\n"
                  "\n"
                  "You should have received a copy of the GNU General Public License along\n"
                  "with Centreon Engine. If not, see <http://www.gnu.org/licenses/>."
               << std::endl;
    exit(EXIT_SUCCESS);
  }

  // Make sure we got the main config file on the command line.
  if (optind >= argc)
    error = TRUE;

  // If there are no command line options or if an error occured, print usage.
  if ((TRUE == error) || (TRUE == display_help)) {
    std::cout << "Usage: " << argv[0] << " [options] <main_config_file>\n"
                 "\nOptions:\n"
                 "  -v, --verify-config         Verify all configuration data.\n"
                 "  -s, --test-scheduling       Shows projected/recommended check\n"
                 "                              scheduling and other diagnostic info\n"
                 "                              based on the current configuration\n"
                 "                              files.\n"
                 "  -x, --dont-verify-paths     Don't check for circular object paths -\n"
                 "                              USE WITH CAUTION !\n"
                 "  -p, --precache-objects      Precache object configuration - use with\n"
                 "                              -v or -s options.\n"
                 "  -u, --use-precached-objects Use precached object config file."
              << std::endl;
    exit(ERROR);
  }

  // Config file is last argument specified.
  config_file = my_strdup(argv[optind]);

  // Make sure the config file uses an absolute path.
  if (config_file[0] != '/') {
    // Get absolute path of current working directory.
    std::string buffer;
    {
      char buff[PATH_MAX];
      if (getcwd(buff, sizeof(buff)) == NULL) {
        char const* msg(strerror(errno));
        std::cerr << "failure while retrieving current working directory: "
                  << msg << std::endl;
        exit(EXIT_FAILURE);
      }
      buffer.append(buff);
    }

    // Append a forward slash.
    buffer.append("/");

    // Append the config file to the path.
    buffer.append(config_file);
    delete [] config_file;
    config_file = my_strdup(buffer.c_str());
  }

  // We're just verifying the configuration.
  int result = ERROR;
  if (TRUE == verify_config) {
    // Reset program variables.
    reset_variables();

    // Read in the configuration files (main config file,
    // resource and object config files).
    try {
      // Read main config file.
      std::cout << "reading main config file" << std::endl;
      config.parse(config_file);

      // Read object config files.
      if ((result = read_all_object_data(config_file)) == OK)
        result = OK;
      else
        result = ERROR;
    }
    catch(std::exception const &e) {
      std::cerr << "error while processing a config file: "
                << e.what() << std::endl;
      result = ERROR;
    }

    // There was a problem reading the config files.
    if (result != OK)
      std::cout << "\n    One or more problems occurred while processing the config files.\n\n"
                   ERROR_CONFIGURATION;

    // The config files were okay, so run the pre-flight check.
    else {
      std::cout << "running pre-flight check on configuration data"
                << std::endl;
      result = pre_flight_check();
      if (result != OK)
        std::cout << "\n    One or more problem occurred during the pre-flight check.\n\n"
                     ERROR_CONFIGURATION;
    }

    // Clean up after ourselves.
    cleanup();

    // Free config_file.
    delete [] config_file;

    // Exit.
    exit(result ? EXIT_FAILURE : EXIT_SUCCESS);
  }

  // We're just testing scheduling.
  else if (TRUE == test_scheduling) {

    // Reset program variables.
    reset_variables();

    // Read in the configuration files (main config file and all host config files).
    try {
      config.parse(config_file);

      // Read object config files.
      result = read_all_object_data(config_file);
    }
    catch(std::exception const &e) {
      std::cerr << "error while processing a config file: "
                << e.what() << std::endl;
    }

    // Read initial service and host state information.
    if (result == OK) {
      initialize_retention_data(config_file);
      read_initial_state_information();
    }

    if (result != OK)
      std::cout << "\n    One or more problems occurred while processing the config files.\n\n";

    // Run the pre-flight check to make sure everything looks okay.
    if ((OK == result) && ((result = pre_flight_check()) != OK))
      std::cout << "\n    One or more problems occurred during the pre-flight check.\n\n";

    if (OK == result) {

      // Initialize the event timing loop.
      init_timing_loop();

      // Display scheduling information.
      display_scheduling_info();

      if (precache_objects == TRUE)
        std::cout << "\n"
                  << "OBJECT PRECACHING\n"
                  << "-----------------\n"
                  << "Object config files were precached.\n";
    }

#undef TEST_TIMEPERIODS
#ifdef TEST_TIMEPERIODS
    /* DO SOME TIMEPERIOD TESTING - ADDED 08/11/2009 */
    time_t now, pref_time, valid_time;
    timeperiod* tp;
    tp = find_timeperiod("247_exclusion");
    time(&now);
    pref_time = now;
    get_next_valid_time(pref_time, &valid_time, tp);
    printf("=====\n");
    printf("CURRENT:   %lu = %s", (unsigned long)now, ctime(&now));
    printf("PREFERRED: %lu = %s", (unsigned long)pref_time, ctime(&pref_time));
    printf("NEXT:      %lu = %s", (unsigned long)valid_time, ctime(&valid_time));
    printf("=====\n");
#endif

    // Clean up after ourselves.
    cleanup();

    // Exit.
    exit(result ? EXIT_FAILURE : EXIT_SUCCESS);
  }

  // Else start to monitor things.
  else {
    char* buffer;
    // Keep monitoring things until we get a shutdown command.
    do {
      // Reset program variables.
      reset_variables();

      // Get PID.
      nagios_pid = (int)getpid();

      // Read in the configuration files (main
      // and resource config files).
      try {
        config.parse(config_file);
        result = OK;
      }
      catch(std::exception const &e) {
        std::cerr << "error while processing a config file: "
                  << e.what() << std::endl;
      }

      /* NOTE 11/06/07 EG moved to after we read config files, as user may have overridden timezone offset */
      /* get program (re)start time and save as macro */
      program_start = time(NULL);
      delete [] mac->x[MACRO_PROCESSSTARTTIME];
      try {
        mac->x[MACRO_PROCESSSTARTTIME] = obj2pchar<unsigned long>(program_start);
      }
      catch (...) {
        cleanup();
        throw ;
      }

      // Open debug log.
      open_debug_log();

      // Initialize modules.
      neb_init_modules();
      neb_init_callback_list();
      try {
        modules::loader& loader = modules::loader::instance();
        loader.set_directory(config.get_broker_module_directory());
        loader.load();
      }
      catch (std::exception const& e) {
        logit(NSLOG_INFO_MESSAGE, false, "Event broker module initialize failed.\n");
        result = ERROR;
      }

      // This must be logged after we read config data, as user may have changed location of main log file.
      logit(NSLOG_PROCESS_INFO, TRUE,
        "Centreon Engine starting... (PID=%d)\n",
        (int)getpid());

      // Log the local time - may be different than clock time due to timezone offset.
      now = time(NULL);
      tm = localtime_r(&now, &tm_s);
      strftime(datestring, sizeof(datestring), "%a %b %d %H:%M:%S %Z %Y", tm);
      logit(NSLOG_PROCESS_INFO, TRUE, "Local time is %s", datestring);

      // Write log version/info.
      write_log_file_info(NULL);

      // Load modules.
      neb_load_all_modules();

      // Send program data to broker.
      broker_program_state(NEBTYPE_PROCESS_PRELAUNCH,
                           NEBFLAG_NONE,
                           NEBATTR_NONE,
                           NULL);

      // Read in all object config data.
      if (result == OK)
        result = read_all_object_data(config_file);

      // There was a problem reading the config files.
      if (result != OK)
        logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR | NSLOG_CONFIG_ERROR, TRUE,
              "Bailing out due to one or more errors encountered in the configuration files. Run Centreon Engine from the command line with the -v option to verify your config before restarting. (PID=%d)",
              (int)getpid());
      else {
        // Run the pre-flight check to make sure everything looks okay.
        if ((result = pre_flight_check()) != OK)
          logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR | NSLOG_VERIFICATION_ERROR, TRUE,
                "Bailing out due to errors encountered while running the pre-flight check.  Run Centreon Engine from the command line with the -v option to verify your config before restarting. (PID=%d)\n",
                (int)getpid());
      }

      // An error occurred that prevented us from (re)starting.
      if (result != OK) {
        // If we were restarting, we need to cleanup from the previous run.
        if (sigrestart == TRUE) {
          // Clean up the status data.
          cleanup_status_data(config_file, TRUE);

          // Shutdown the external command worker thread.
          shutdown_command_file_worker_thread();

          // Close and delete the external command file FIFO.
          close_command_file();

          // Cleanup embedded perl interpreter.
          if (embedded_perl_initialized == TRUE)
            deinit_embedded_perl();
        }

        // Send program data to broker.
        broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
                             NEBFLAG_PROCESS_INITIATED,
                             NEBATTR_SHUTDOWN_ABNORMAL,
                             NULL);
        cleanup();
        exit(ERROR);
      }

      /* initialize embedded Perl interpreter */
      /* NOTE 02/15/08 embedded Perl must be initialized if compiled in, regardless of whether or not its enabled in the config file */
      /* It compiled it, but not initialized, Centreon Engine will segfault in readdir() calls, as libperl takes this function over */
      if (embedded_perl_initialized == FALSE) {
        /*                                if(enable_embedded_perl==TRUE){*/
#ifdef EMBEDDEDPERL
        init_embedded_perl(env);
#else
        init_embedded_perl(NULL);
#endif
        embedded_perl_initialized = TRUE;
        /*                                        }*/
      }

      // Handle signals (interrupts).
      setup_sighandler();

      // Send program data to broker.
      broker_program_state(NEBTYPE_PROCESS_START,
                           NEBFLAG_NONE,
                           NEBATTR_NONE,
                           NULL);

      // Open the command file (named pipe) for reading.
      result = open_command_file();
      if (result != OK) {

        logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR, TRUE,
              "Bailing out due to errors encountered while trying to initialize the external command file... (PID=%d)\n",
              (int)getpid());

        // Send program data to broker.
        broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
                             NEBFLAG_PROCESS_INITIATED,
                             NEBATTR_SHUTDOWN_ABNORMAL,
                             NULL);
        cleanup();
        exit(ERROR);
      }

      // Initialize status data unless we're starting.
      if (sigrestart == FALSE)
        initialize_status_data(config_file);

      // Read initial service and host state information.
      initialize_retention_data(config_file);
      read_initial_state_information();

      // Initialize comment data.
      initialize_comment_data(config_file);

      // Initialize scheduled downtime data.
      initialize_downtime_data(config_file);

      // Initialize performance data.
      initialize_performance_data(config_file);

      // Initialize the event timing loop.
      init_timing_loop();

      // Initialize check statistics.
      init_check_stats();

      // Update all status data (with retained information).
      update_all_status_data();

      // Log initial host and service state.
      log_host_states(INITIAL_STATES, NULL);
      log_service_states(INITIAL_STATES, NULL);

      // Reset the restart flag.
      sigrestart = FALSE;

      // Send program data to broker.
      broker_program_state(NEBTYPE_PROCESS_EVENTLOOPSTART,
                           NEBFLAG_NONE,
                           NEBATTR_NONE,
                           NULL);

      // Get event start time and save as macro.
      event_start = time(NULL);
      delete[] mac->x[MACRO_EVENTSTARTTIME];
      try {
        mac->x[MACRO_EVENTSTARTTIME] = obj2pchar<unsigned long>(event_start);
      }
      catch(...) {
        // Send program data to broker.
        broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
                             NEBFLAG_PROCESS_INITIATED,
                             NEBATTR_SHUTDOWN_ABNORMAL,
                             NULL);
        cleanup();
      }

      /***** Start monitoring all services. *****/
      // (doesn't return until a restart or shutdown signal is encountered).
      event_execution_loop();

      /* 03/01/2007 EG Moved from sighandler() to prevent FUTEX locking problems under NPTL */
      // Did we catch a signal ?
      if (caught_signal == TRUE) {
        if (sig_id == SIGHUP) {
          try {
            buffer = my_strdup("Caught SIGHUP, restarting...\n");
          }
          catch(...) {
            // Send program data to broker.
            broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
                                 NEBFLAG_PROCESS_INITIATED,
                                 NEBATTR_SHUTDOWN_ABNORMAL,
                                 NULL);
            cleanup();
          }
        }
        else {
          try {
            std::ostringstream oss;
            oss << "Caught SIG" << sigs[sig_id] << ", shutting down...\n";
            buffer = my_strdup(oss.str().c_str());
          }
          catch(...) {
            // Send program data to broker.
            broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
                                 NEBFLAG_PROCESS_INITIATED,
                                 NEBATTR_SHUTDOWN_ABNORMAL,
                                 NULL);
            cleanup();
          }
        }

        write_to_all_logs(buffer, NSLOG_PROCESS_INFO);
        delete[] buffer;
      }

      // Send program data to broker.
      broker_program_state(NEBTYPE_PROCESS_EVENTLOOPEND,
                           NEBFLAG_NONE,
                           NEBATTR_NONE,
                           NULL);
      if (sigshutdown == TRUE)
        broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
                             NEBFLAG_USER_INITIATED,
                             NEBATTR_SHUTDOWN_NORMAL,
                             NULL);
      else if (sigrestart == TRUE)
        broker_program_state(NEBTYPE_PROCESS_RESTART,
                             NEBFLAG_USER_INITIATED,
                             NEBATTR_RESTART_NORMAL,
                             NULL);

      // Save service and host state information.
      save_state_information(FALSE);
      cleanup_retention_data(config_file);

      // Clean up performance data.
      cleanup_performance_data(config_file);

      // Clean up the scheduled downtime data.
      cleanup_downtime_data(config_file);

      // Clean up the comment data.
      cleanup_comment_data(config_file);

      // Clean up the status data unless we're restarting.
      if (sigrestart == FALSE)
        cleanup_status_data(config_file, TRUE);

      // Close and delete the external command file FIFO unless we're restarting.
      if (sigrestart == FALSE) {
        shutdown_command_file_worker_thread();
        close_command_file();
      }

      // Cleanup embedded perl interpreter.
      if (sigrestart == FALSE)
        deinit_embedded_perl();

      // Shutdown stuff.
      if (sigshutdown == TRUE) {
        // Log a shutdown message.
        logit(NSLOG_PROCESS_INFO, TRUE,
              "Successfully shutdown... (PID=%d)\n", (int)getpid());
      }

      // Clean up after ourselves.
      cleanup();

      // Close debug log.
      close_debug_log();

    } while (sigrestart == TRUE && sigshutdown == FALSE);

    // Free misc memory.
    delete [] config_file;
  }

  return (OK);
}
