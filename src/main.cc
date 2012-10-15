/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
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

#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif // HAVE_GETOPT_H
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "com/centreon/clib.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/broker/compatibility.hh"
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/comments.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/configuration/applier/logging.hh"
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/events/loop.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/broker.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/nebmods.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/perfdata.hh"
#include "com/centreon/engine/sretention.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/utils.hh"
#include "com/centreon/engine/version.hh"
#include "com/centreon/io/directory_entry.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

// Error message when configuration parsing fail.
#define ERROR_CONFIGURATION \
  "    Check your configuration file(s) to ensure that they contain valid\n" \
  "    directives and data defintions. If you are upgrading from a\n" \
  "    previous version of Centreon Engine, you should be aware that some\n" \
  "    variables/definitions may have been removed or modified in this\n" \
  "    version. Make sure to read the documentation regarding the config\n" \
  "    files, as well as the version changelog to find out what has\n" \
  "    changed.\n"

/**
 *  Centreon Engine entry point.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main(int argc, char* argv[]) {
  struct tm* tm, tm_s;
  time_t now;
  char datestring[256];
  nagios_macros* mac;

  // Get global macros.
  mac = get_global_macros();

#ifdef HAVE_GETOPT_H
  int option_index = 0;
  static struct option const long_options[] = {
    { "dont-verify-paths", no_argument, NULL, 'x' },
    { "help", no_argument, NULL, 'h' },
    { "license", no_argument, NULL, 'V' },
    { "precache-objects", no_argument, NULL, 'p' },
    { "test-scheduling", no_argument, NULL, 's' },
    { "use-precached-objects", no_argument, NULL, 'u' },
    { "verify-config", no_argument, NULL, 'v' },
    { "version", no_argument, NULL, 'V' },
    { NULL, no_argument, NULL, '\0' }
  };
#endif // HAVE_GETOPT_H

  // Load singletons.
  com::centreon::clib::load();
  com::centreon::engine::configuration::state::load();
  com::centreon::engine::logging::engine::load();
  com::centreon::engine::configuration::applier::logging::load();
  com::centreon::engine::commands::set::load();
  com::centreon::engine::checks::checker::load();
  com::centreon::engine::events::loop::load();
  com::centreon::engine::broker::loader::load();
  com::centreon::engine::broker::compatibility::load();

  int retval(EXIT_FAILURE);
  try {
    // Options.
    bool display_help(false);
    bool display_license(false);
    bool error(false);

    // Process all command line arguments.
    int c;
#ifdef HAVE_GETOPT_H
    while ((c = getopt_long(
                  argc,
                  argv,
                  "+hVvsxpu",
                  long_options,
                  &option_index)) != -1) {
#else
    while ((c = getopt(argc, argv, "+hVvsxpu")) != -1) {
#endif // HAVE_GETOPT_H

      // Process flag.
      switch (c) {
      case '?': // Usage.
      case 'h':
        display_help = true;
        break;
      case 'V': // Version.
        display_license = true;
        break;
      case 'v': // Verify config->
        verify_config = TRUE;
        break;
      case 's': // Scheduling Check.
        test_scheduling = TRUE;
        break;
      case 'x': // Don't verify circular paths.
        verify_circular_paths = FALSE;
        break;
      case 'p': // Precache object config->
        precache_objects = TRUE;
        break;
      case 'u': // Use precached object config->
        use_precached_objects = TRUE;
        break;
      default:
        error = true;
      }
    }

    // Invalid argument count.
    if ((argc < 2)
        // Invalid argument combination.
        || (precache_objects && !test_scheduling && !verify_config)
        // Main configuration file not on command line.
        || (optind >= argc))
      error = true;
    else {
      // Config file is last argument specified.
      config_file = my_strdup(argv[optind]);

      // Make sure the config file uses an absolute path.
      if (config_file[0] != '/') {
        // Get absolute path of current working directory.
        std::string
          buffer(com::centreon::io::directory_entry::current_path());
        buffer.append("/");
        buffer.append(config_file);
        delete[] config_file;
        config_file = NULL;
        config_file = my_strdup(buffer.c_str());
      }
    }

    // Just display the license.
    if (display_license) {
      logger(log_info_message, basic)
        << "Centreon Engine " << CENTREON_ENGINE_VERSION_STRING << "\n"
        << "\n"
        << "Copyright 1999-2009 Ethan Galstad\n"
        << "Copyright 2009-2010 Nagios Core Development Team and Community Contributors\n"
        << "Copyright 2011-2012 Merethis\n"
        << "\n"
        << "This program is free software: you can redistribute it and/or\n"
        << "modify it under the terms of the GNU General Public License version 2\n"
        << "as published by the Free Software Foundation.\n"
        << "\n"
        << "Centreon Engine is distributed in the hope that it will be useful,\n"
        << "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU\n"
        << "General Public License for more details.\n"
        << "\n"
        << "You should have received a copy of the GNU General Public License\n"
        << "along with this program. If not, see\n"
        << "<http://www.gnu.org/licenses/>.";
      retval = EXIT_SUCCESS;
    }
    // If requested or if an error occured, print usage.
    else if (error || display_help) {
      logger(log_info_message, basic)
        << "Usage: " << argv[0] << " [options] <main_config_file>\n"
        << "\n"
        << "Basics:\n"
        << "  -h, --help                  Print help.\n"
        << "  -V, --license, --version    Print software version and license.\n"
        << "\n"
        << "Configuration:\n"
        << "  -v, --verify-config         Verify all configuration data.\n"
        << "  -s, --test-scheduling       Shows projected/recommended check\n"
        << "                              scheduling and other diagnostic info\n"
        << "                              based on the current configuration\n"
        << "                              files.\n"
        << "  -x, --dont-verify-paths     Don't check for circular object paths -\n"
        << "                              USE WITH CAUTION !\n"
        << "  -p, --precache-objects      Precache object configuration - use with\n"
        << "                              -v or -s options.\n"
        << "  -u, --use-precached-objects Use precached object config file.";
      retval = (display_help ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    // We're just verifying the configuration.
    else if (verify_config) {
      int result(ERROR);

      // Reset program variables.
      reset_variables();

      // Read in the configuration files (main config file,
      // resource and object config files).
      try {
        // Read main config file.
        logger(log_info_message, basic)
          << "reading main config file";
        config->parse(config_file);

        // Read object config files.
        if ((result = read_all_object_data(config_file)) == OK)
          result = OK;
        else
          result = ERROR;
      }
      catch (std::exception const &e) {
        logger(log_config_error, basic)
          << "error while processing a config file: " << e.what();
        result = ERROR;
      }

      // There was a problem reading the config files.
      if (result != OK)
        logger(log_config_error, basic)
          << "\n    One or more problems occurred while processing the config files.\n\n"
          << ERROR_CONFIGURATION;
      // The config files were okay, so run the pre-flight check.
      else {
        logger(log_info_message, basic)
          << "running pre-flight check on configuration data";
        result = pre_flight_check();
        if (result != OK)
          logger(log_config_error, basic)
            << "\n    One or more problem occurred during the pre-flight check.\n\n"
            << ERROR_CONFIGURATION;
      }

      // Return value.
      retval = (result ? EXIT_FAILURE : EXIT_SUCCESS);
    }
    // We're just testing scheduling.
    else if (test_scheduling) {
      int result(ERROR);

      // Reset program variables.
      reset_variables();

      // Read in the configuration files (main config file and all host config files).
      try {
        config->parse(config_file);
        configuration::applier::logging::instance().apply(*config);
        engine::obj_info
          obj(
            com::centreon::shared_ptr<logging::object>(new logging::broker),
            log_all,
            basic);
        engine::instance().add_object(obj);

        // Read object config files.
        result = read_all_object_data(config_file);
      }
      catch (std::exception const &e) {
        logger(log_config_error, basic)
          << "error while processing a config file: " << e.what();
      }

      // Read initial service and host state information.
      if (result == OK) {
        initialize_retention_data(config_file);
        read_initial_state_information();
      }

      if (result != OK) {
        logger(log_config_error, basic)
          << "\n    One or more problems occurred while processing the config files.\n";
      }

      // Run the pre-flight check to make sure everything looks okay.
      if ((OK == result) && ((result = pre_flight_check()) != OK)) {
        logger(log_config_error, basic)
          << "\n    One or more problems occurred during the pre-flight check.\n";
      }

      if (OK == result) {
        // Initialize the event timing loop.
        init_timing_loop();

        // Display scheduling information.
        display_scheduling_info();

        if (precache_objects == TRUE)
          logger(log_info_message, basic)
            << "\n"
            << "OBJECT PRECACHING\n"
            << "-----------------\n"
            << "Object config files were precached.";
      }

      // Return value.
      retval = (result ? EXIT_FAILURE : EXIT_SUCCESS);
    }
    // Else start to monitor things.
    else {
      char* buffer(NULL);
      int result(ERROR);

      // Keep monitoring things until we get a shutdown command.
      do {
        // Reset program variables.
        reset_variables();

        // Read in the configuration files (main
        // and resource config files).
        try {
          config->parse(config_file);
          configuration::applier::logging::instance().apply(*config);
          engine::obj_info obj(
                             com::centreon::shared_ptr<logging::object>(
                                              new logging::broker),
                             log_all,
                             basic);
          engine::instance().add_object(obj);
          result = OK;
        }
        catch (std::exception const &e) {
          logger(log_config_error, basic)
            << "error while processing a config file: " << e.what();
        }

        // Get program (re)start time and save as macro. Needs to be done
        // after we read config files, as user may have overridden
        // timezone offset.
        program_start = time(NULL);
        delete[] mac->x[MACRO_PROCESSSTARTTIME];
        mac->x[MACRO_PROCESSSTARTTIME]
          = obj2pchar<unsigned long>(program_start);

        // Initialize modules.
        neb_init_modules();
        neb_init_callback_list();
        try {
          com::centreon::engine::broker::loader& loader(
            com::centreon::engine::broker::loader::instance());
          std::string const& mod_dir(config->get_broker_module_directory());
          if (!mod_dir.empty())
            loader.load_directory(mod_dir);
        }
        catch (std::exception const& e) {
          logger(log_info_message, basic)
            << "error: event broker module initialization failed: "
            << e.what();
          result = ERROR;
        }

        // This must be logged after we read config data, as user may have changed location of main log file.
        logger(log_process_info, basic)
          << "Centreon Engine " << CENTREON_ENGINE_VERSION_STRING
          << " starting ... (PID=" << getpid() << ")";

        // Log the local time - may be different than clock time due to timezone offset.
        now = time(NULL);
        tm = localtime_r(&now, &tm_s);
        strftime(
          datestring,
          sizeof(datestring),
          "%a %b %d %H:%M:%S %Z %Y",
          tm);
        logger(log_process_info, basic)
          << "Local time is " << datestring;

        // Write log version/info.
        logger(log_process_info, basic)
          <<  "LOG VERSION: " << LOG_VERSION_2;

        // Load modules.
        neb_load_all_modules();

        // Send program data to broker.
        broker_program_state(
          NEBTYPE_PROCESS_PRELAUNCH,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          NULL);

        // Read in all object config data.
        if (result == OK)
          result = read_all_object_data(config_file);

        // There was a problem reading the config files.
        if (result != OK)
          logger(log_process_info | log_runtime_error | log_config_error, basic)
            << "Bailing out due to one or more errors encountered in the configuration files. "
            << "Run Centreon Engine from the command line with the -v option to verify your config before restarting. (PID=" << getpid() << ")";
        // Run the pre-flight check to make sure everything looks okay.
        else if ((result = pre_flight_check()) != OK)
          logger(log_process_info | log_runtime_error | log_verification_error, basic)
            << "Bailing out due to errors encountered while running the pre-flight check.  "
            << "Run Centreon Engine from the command line with the -v option to verify your config before restarting. (PID=" << getpid() << ")";

        // An error occurred that prevented us from (re)starting.
        if (result != OK) {
          // Send program data to broker.
          broker_program_state(
            NEBTYPE_PROCESS_SHUTDOWN,
            NEBFLAG_PROCESS_INITIATED,
            NEBATTR_SHUTDOWN_ABNORMAL,
            NULL);
          throw (engine_error ()
                 << "Shutting down because of an early failure");
        }

        // Handle signals (interrupts).
        setup_sighandler();

        // Send program data to broker.
        broker_program_state(
          NEBTYPE_PROCESS_START,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          NULL);

        // Initialize status data.
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

        // Send program data to broker.
        broker_program_state(
          NEBTYPE_PROCESS_EVENTLOOPSTART,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          NULL);

        // Get event start time and save as macro.
        event_start = time(NULL);
        delete[] mac->x[MACRO_EVENTSTARTTIME];
        try {
          mac->x[MACRO_EVENTSTARTTIME]
            = obj2pchar<unsigned long>(event_start);
        }
        catch (...) {
          // Send program data to broker.
          broker_program_state(
            NEBTYPE_PROCESS_SHUTDOWN,
            NEBFLAG_PROCESS_INITIATED,
            NEBATTR_SHUTDOWN_ABNORMAL,
            NULL);
        }

        /***** Start monitoring all services. *****/
        // (doesn't return until a restart or shutdown signal is encountered).
        events::loop& loop(events::loop::instance());
        loop.run();

        /* 03/01/2007 EG Moved from sighandler() to prevent FUTEX locking problems under NPTL */
        // Did we catch a signal ?
        if (sigshutdown == TRUE) {
          try {
            std::ostringstream oss;
            oss << "Caught SIG" << sigs[sig_id] << ", shutting down ...";
            buffer = my_strdup(oss.str().c_str());
          }
          catch (...) {
            // Send program data to broker.
            broker_program_state(
              NEBTYPE_PROCESS_SHUTDOWN,
              NEBFLAG_PROCESS_INITIATED,
              NEBATTR_SHUTDOWN_ABNORMAL,
              NULL);
          }

          logger(log_process_info, basic)
            << buffer;
          delete[] buffer;
        }

        // Send program data to broker.
        broker_program_state(
          NEBTYPE_PROCESS_EVENTLOOPEND,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          NULL);
        if (sigshutdown == TRUE)
          broker_program_state(
            NEBTYPE_PROCESS_SHUTDOWN,
            NEBFLAG_USER_INITIATED,
            NEBATTR_SHUTDOWN_NORMAL,
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

        // Clean up the status data.
        cleanup_status_data(config_file, TRUE);

        // Shutdown stuff.
        if (sigshutdown == TRUE) {
          // Log a shutdown message.
          logger(log_process_info, basic)
            << "Successfully shutdown ... (PID=" << getpid() << ")";
        }

        // Clean up after ourselves.
        cleanup();

      } while (sigshutdown == FALSE);

      // Successful execution.
      retval = EXIT_SUCCESS;
    }
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "error: " << e.what();
  }

  // Memory cleanup.
  cleanup();
  delete[] config_file;
  delete[] mac->x[MACRO_OBJECTCACHEFILE];
  delete[] mac->x[MACRO_PROCESSSTARTTIME];
  delete[] mac->x[MACRO_EVENTSTARTTIME];
  delete[] mac->x[MACRO_RETENTIONDATAFILE];
  delete[] mac->x[MACRO_STATUSDATAFILE];
  com::centreon::engine::broker::compatibility::unload();
  com::centreon::engine::broker::loader::unload();
  com::centreon::engine::events::loop::unload();
  com::centreon::engine::checks::checker::unload();
  com::centreon::engine::commands::set::unload();
  com::centreon::engine::configuration::applier::logging::unload();
  com::centreon::engine::configuration::state::unload();
  com::centreon::engine::logging::engine::unload();
  com::centreon::clib::unload();

  return (retval);
}
