/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
** Copyright 2011-2017 Centreon
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

#include <cstdlib>
#include <cstring>
#include <exception>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif  // HAVE_GETOPT_H
#include <unistd.h>

#include <random>
#include <string>

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/logging.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/diagnostic.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/events/loop.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/broker.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros/misc.hh"
#include "com/centreon/engine/nebmods.hh"
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/retention/parser.hh"
#include "com/centreon/engine/retention/state.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/utils.hh"
#include "com/centreon/engine/version.hh"
#include "com/centreon/io/directory_entry.hh"
#include "com/centreon/logging/engine.hh"
#include "enginerpc.hh"

using namespace com::centreon::engine;

// Error message when configuration parsing fail.
#define ERROR_CONFIGURATION                                                  \
  "    Check your configuration file(s) to ensure that they contain valid\n" \
  "    directives and data definitions. If you are upgrading from a\n"       \
  "    previous version of Centreon Engine, you should be aware that some\n" \
  "    variables/definitions may have been removed or modified in this\n"    \
  "    version. Make sure to read the documentation regarding the config\n"  \
  "    files, as well as the version changelog to find out what has\n"       \
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
  // Get global macros.
  nagios_macros* mac(get_global_macros());

#ifdef HAVE_GETOPT_H
  int option_index = 0;
  static struct option const long_options[] = {
      {"diagnose", no_argument, NULL, 'D'},
      {"dont-verify-paths", no_argument, NULL, 'x'},
      {"help", no_argument, NULL, 'h'},
      {"license", no_argument, NULL, 'V'},
      {"test-scheduling", no_argument, NULL, 's'},
      {"verify-config", no_argument, NULL, 'v'},
      {"version", no_argument, NULL, 'V'},
      {NULL, no_argument, NULL, '\0'}};
#endif  // HAVE_GETOPT_H

  // Load singletons and global variable.
  config = new configuration::state;

  // Hack to instanciate the logger.
  configuration::applier::logging::instance();

  logging::broker backend_broker_log;

  int retval(EXIT_FAILURE);
  try {
    // Options.
    bool display_help(false);
    bool display_license(false);
    bool error(false);
    bool diagnose(false);

    // Process all command line arguments.
    int c;
#ifdef HAVE_GETOPT_H
    while ((c = getopt_long(argc, argv, "+hVvsxD", long_options,
                            &option_index)) != -1) {
#else
    while ((c = getopt(argc, argv, "+hVvsxD")) != -1) {
#endif  // HAVE_GETOPT_H

      // Process flag.
      switch (c) {
        case '?':  // Usage.
        case 'h':
          display_help = true;
          break;
        case 'V':  // Version.
          display_license = true;
          break;
        case 'v':  // Verify config.
          verify_config = true;
          break;
        case 's':  // Scheduling check.
          test_scheduling = true;
          break;
        case 'x':  // Don't verify circular paths.
          verify_circular_paths = false;
          break;
        case 'D':  // Diagnostic.
          diagnose = true;
          break;
        default:
          error = true;
      }
    }

    // Invalid argument count.
    if ((argc < 2)
        // Main configuration file not on command line.
        || (optind >= argc))
      error = true;
    else {
      // Config file is last argument specified.
      config_file = string::dup(argv[optind]);

      // Make sure the config file uses an absolute path.
      if (config_file[0] != '/') {
        // Get absolute path of current working directory.
        std::string buffer(com::centreon::io::directory_entry::current_path());
        buffer.append("/");
        buffer.append(config_file);
        string::setstr(config_file, buffer);
      }
    }

    // Reset umask.
    umask(S_IWGRP | S_IWOTH);

    // Just display the license.
    if (display_license) {
      logger(logging::log_info_message, logging::basic)
          << "Centreon Engine " << CENTREON_ENGINE_VERSION_STRING << "\n"
          << "\n"
          << "Copyright 1999-2009 Ethan Galstad\n"
          << "Copyright 2009-2010 Nagios Core Development Team and Community "
             "Contributors\n"
          << "Copyright 2011-2018 Centreon\n"
          << "\n"
          << "This program is free software: you can redistribute it and/or\n"
          << "modify it under the terms of the GNU General Public License "
             "version 2\n"
          << "as published by the Free Software Foundation.\n"
          << "\n"
          << "Centreon Engine is distributed in the hope that it will be "
             "useful,\n"
          << "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
          << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
             "GNU\n"
          << "General Public License for more details.\n"
          << "\n"
          << "You should have received a copy of the GNU General Public "
             "License\n"
          << "along with this program. If not, see\n"
          << "<http://www.gnu.org/licenses/>.";
      retval = EXIT_SUCCESS;
    }
    // If requested or if an error occured, print usage.
    else if (error || display_help) {
      logger(logging::log_info_message, logging::basic)
          << "Usage: " << argv[0] << " [options] <main_config_file>\n"
          << "\n"
          << "Basics:\n"
          << "  -h, --help                  Print help.\n"
          << "  -V, --license, --version    Print software version and "
             "license.\n"
          << "\n"
          << "Configuration:\n"
          << "  -v, --verify-config         Verify all configuration data.\n"
          << "  -s, --test-scheduling       Shows projected/recommended check\n"
          << "                              scheduling and other diagnostic "
             "info\n"
          << "                              based on the current "
             "configuration\n"
          << "                              files.\n"
          << "  -x, --dont-verify-paths     Don't check for circular object "
             "paths -\n"
          << "                              USE WITH CAUTION !\n"
          << "  -D, --diagnose              Generate a diagnostic file.\n"
          << "\n"
          << "Online:\n"
          << "  Website                     https://www.centreon.com\n"
          << "  Reference documentation     "
             "https://documentation.centreon.com/docs/centreon-engine/en/"
             "latest/\n"
          << "  Sources                     "
             "https://github.com/centreon/centreon-engine";
      retval = (display_help ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    // We're just verifying the configuration.
    else if (verify_config) {
      try {
        // Read in the configuration files (main config file,
        // resource and object config files).
        configuration::state config;
        {
          configuration::parser p;
          p.parse(config_file, config);
        }

        configuration::applier::state::instance().apply(config);

        logger(logging::log_info_message, logging::basic)
            << "\n"
            << "Checked " << commands::command::commands.size()
            << " commands.\n"
            << "Checked " << commands::connector::connectors.size()
            << " connectors.\n"
            << "Checked " << contact::contacts.size() << " contacts.\n"
            << "Checked " << hostdependency::hostdependencies.size()
            << " host dependencies.\n"
            << "Checked " << hostescalation::hostescalations.size()
            << " host escalations.\n"
            << "Checked " << hostgroup::hostgroups.size() << " host groups.\n"
            << "Checked " << host::hosts.size() << " hosts.\n"
            << "Checked " << servicedependency::servicedependencies.size()
            << " service dependencies.\n"
            << "Checked " << serviceescalation::serviceescalations.size()
            << " service escalations.\n"
            << "Checked " << servicegroup::servicegroups.size()
            << " service groups.\n"
            << "Checked " << service::services.size() << " services.\n"
            << "Checked " << timeperiod::timeperiods.size()
            << " time periods.\n"
            << "\n"
            << "Total Warnings: " << config_warnings << "\n"
            << "Total Errors:   " << config_errors;

        retval = (config_errors ? EXIT_FAILURE : EXIT_SUCCESS);
      } catch (std::exception const& e) {
        logger(logging::log_config_error, logging::basic)
            << "Error while processing a config file: " << e.what();
        logger(logging::log_config_error, logging::basic)
            << "One or more problems occurred while processing "
               "the config files.\n\n" ERROR_CONFIGURATION;
      }
    }
    // We're just testing scheduling.
    else if (test_scheduling) {
      try {
        // Parse configuration.
        configuration::state config;
        {
          configuration::parser p;
          p.parse(config_file, config);
        }

        // Parse retention.
        retention::state state;
        if (!config.state_retention_file().empty()) {
          retention::parser p;
          try {
            p.parse(config.state_retention_file(), state);
          } catch (std::exception const& e) {
            logger(logging::log_config_error, logging::basic) << e.what();
          }
        }

        // Apply configuration.
        configuration::applier::state::instance().apply(config, state);

        display_scheduling_info();
        retval = EXIT_SUCCESS;
      } catch (std::exception const& e) {
        logger(logging::log_config_error, logging::basic) << e.what();
      }
    }
    // Diagnostic.
    else if (diagnose) {
      diagnostic diag;
      diag.generate(config_file);
    }
    // Else start to monitor things.
    else {
      try {
        // Parse configuration.
        configuration::state config;
        {
          configuration::parser p;
          p.parse(config_file, config);
        }

        uint16_t port = config.rpc_port();

        if (!port) {
          std::random_device
              rd;  // Will be used to obtain a seed for the random number engine
          std::mt19937 gen(
              rd());  // Standard mersenne_twister_engine seeded with rd()
          std::uniform_int_distribution<uint16_t> dis(50000, 50999);

          port = dis(gen);
        }

        std::unique_ptr<enginerpc, std::function<void(enginerpc*)>> rpc(
            new enginerpc("0.0.0.0", port), [](enginerpc* rpc) {
              rpc->shutdown();
              delete rpc;
            });

        // Parse retention.
        retention::state state;
        {
          retention::parser p;
          try {
            p.parse(config.state_retention_file(), state);
          } catch (std::exception const& e) {
            logger(logging::log_config_error, logging::basic) << e.what();
          }
        }

        // Get program (re)start time and save as macro. Needs to be
        // done after we read config files, as user may have overridden
        // timezone offset.
        program_start = std::time(nullptr);
        mac->x[MACRO_PROCESSSTARTTIME] = std::to_string(program_start);

        // Load broker modules.
        for (std::list<std::string>::const_iterator
                 it(config.broker_module().begin()),
             end(config.broker_module().end());
             it != end; ++it) {
          std::string filename;
          std::string args;
          if (!string::split(*it, filename, args, ' '))
            filename = *it;
          broker::loader::instance().add_module(filename, args);
        }
        neb_init_callback_list();

        // Add broker backend.
        com::centreon::logging::engine::instance().add(
            &backend_broker_log, logging::log_all, logging::basic);

        // Apply configuration.
        configuration::applier::state::instance().apply(config, state);

        // Handle signals (interrupts).
        setup_sighandler();

        // Initialize status data.
        initialize_status_data();

        // Initialize scheduled downtime data.
        downtimes::downtime_manager::instance().initialize_downtime_data();

        // Initialize check statistics.
        init_check_stats();

        // Update all status data (with retained information).
        update_all_status_data();

        // Send program data to broker.
        broker_program_state(NEBTYPE_PROCESS_EVENTLOOPSTART, NEBFLAG_NONE,
                             NEBATTR_NONE, NULL);

        // Get event start time and save as macro.
        event_start = time(NULL);
        mac->x[MACRO_EVENTSTARTTIME] = std::to_string(event_start);

        logger(logging::log_info_message, logging::basic)
            << "Event loop start at " << string::ctime(event_start);

        // Start monitoring all services (doesn't return until a
        // restart or shutdown signal is encountered).
        com::centreon::engine::events::loop::instance().run();

        if (sigshutdown)
          logger(logging::log_process_info, logging::basic)
              << "Caught SIG" << sigs[sig_id] << ", shutting down ...";

        // Send program data to broker.
        broker_program_state(NEBTYPE_PROCESS_EVENTLOOPEND, NEBFLAG_NONE,
                             NEBATTR_NONE, NULL);
        if (sigshutdown)
          broker_program_state(NEBTYPE_PROCESS_SHUTDOWN, NEBFLAG_USER_INITIATED,
                               NEBATTR_SHUTDOWN_NORMAL, NULL);

        // Save service and host state information.
        retention::dump::save(::config->state_retention_file());

        // Clean up the status data.
        cleanup_status_data(true);

        // Shutdown stuff.
        if (sigshutdown)
          logger(logging::log_process_info, logging::basic)
              << "Successfully shutdown ... (PID=" << getpid() << ")";

        retval = EXIT_SUCCESS;

      } catch (std::exception const& e) {
        // Log.
        logger(logging::log_runtime_error, logging::basic)
            << "Error: " << e.what();

        // Send program data to broker.
        broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
                             NEBFLAG_PROCESS_INITIATED,
                             NEBATTR_SHUTDOWN_ABNORMAL, NULL);
      }
    }

    // Memory cleanup.
    cleanup();
    delete[] config_file;
    config_file = NULL;
  } catch (std::exception const& e) {
    logger(logging::log_runtime_error, logging::basic) << "Error: " << e.what();
  }

  // Unload singletons and global objects.
  delete config;
  config = nullptr;

  return retval;
}
