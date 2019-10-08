/*
** Copyright 2015 Merethis
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

#include <csignal>
#include <cstdlib>
#include <fstream>
#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif // HAVE_GETOPT_H
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include "com/centreon/engine/process.hh"
#include "engine_cfg.hh"

/**
 *  Bench how long Centreon Engine needs to process some passive check
 *  results.
 *
 *  @return EXIT_SUCCESS.
 */
int main(int argc, char* argv[]) {
  // Initialization.
  srandom(getpid());
  signal(SIGPIPE, SIG_IGN);

  // Options.
#ifdef HAVE_GETOPT_H
  int option_index(0);
  static struct option const long_options[] = {
    // Common options.
    { "help", no_argument, NULL, '?' },
    { "mode", required_argument, NULL, 'M' },
    { "activehosts", required_argument, NULL, 'h' },
    { "activeservices", required_argument, NULL, 's' },
    { "passivehosts", required_argument, NULL, 'H' },
    { "passiveservices", required_argument, NULL, 'S' },
    { "count", required_argument, NULL, 'c' },
    // Benchmark options.
    { "engine", required_argument, NULL, 'e' },
    { "module", required_argument, NULL, 'm' },
    { NULL, no_argument, NULL, '\0' }
  };
#endif // HAVE_GETOPT_H
  int activehosts(0);
  int activeservices(0);
  int passivehosts(1);
  int passiveservices(100);
  std::string mode;
  int count(1000);
  std::string engine("/usr/sbin/centengine");
  std::string module("/usr/lib64/centreon-engine/externalcmd.so");

  // Process command line arguments.
  int c;
#ifdef HAVE_GETOPT_H
  while ((c = getopt_long(
                argc,
                argv,
                "+?h:M:s:H:S:c:e:m:",
                long_options,
                &option_index)) != -1) {
#else
  while ((c = getopt(argc, argv, "+?h:M:s:H:S:c:e:m:")) != -1) {
#endif // HAVE_GETOPT_H
    switch (c) {
    case '?':
      mode.clear();
      break ;
    case 'M':
      mode = optarg;
      break ;
    case 'h':
      activehosts = strtol(optarg, NULL, 0);
      break ;
    case 's':
      activeservices = strtol(optarg, NULL, 0);
      break ;
    case 'H':
      passivehosts = strtol(optarg, NULL, 0);
      break ;
    case 'S':
      passiveservices = strtol(optarg, NULL, 0);
      break ;
    case 'c':
      count = strtol(optarg, NULL, 0);
      break ;
    case 'e':
      engine = optarg;
      break ;
    case 'm':
      module = optarg;
      break ;
    }
  }

  // Banner.
  if (mode != "commands") {
    std::cout << "---------------------------------------------\n"
              << "Centreon Engine passive checks benchmark tool\n"
              << "---------------------------------------------\n"
              << "\n";
  }

  // Perform benchmark.
  if (mode == "benchmark") {
    // Generate configuration files.
    std::cout << "Generating configuration files...               ";
    std::cout.flush();
    std::string additional("broker_module=");
    additional.append(module);
    additional.append("\n");
    engine_cfg cfg_files(
                 additional,
                 count,
                 activehosts,
                 activeservices,
                 passivehosts,
                 passiveservices);
    std::cout << "Done\n";

    // Launch Centreon Engine.
    std::cout << "Starting Centreon Engine...                     ";
    std::cout.flush();
    std::string cmdline(engine);
    cmdline.append(" ");
    cmdline.append(cfg_files.main_file());
    com::centreon::engine::process centengine;
    centengine.enable_stream(com::centreon::engine::process::in, false);
    centengine.enable_stream(com::centreon::engine::process::out, false);
    centengine.enable_stream(com::centreon::engine::process::err, false);
    centengine.exec(cmdline);
    while (access(cfg_files.command_file().c_str(), F_OK))
      sleep(1);
    time_t start_time(time(NULL));  // Perform benchmark.
    std::cout << "Done\n";

    // Send external commands.
    {
      time_t now(time(NULL));
      // Send a little bit more external commands as writing and reading
      // to the same pipe is not thread safe.
      std::ofstream ofs;
      ofs.open(cfg_files.command_file().c_str());
      if (ofs.good()) {
        int slice(count / 100 + 1);
        for (int i(0), limit(count * 105 / 100); i < limit; ++i) {
          if (!(i % 10000)) {
            std::cout << "\rSending passive check results...                "
                      << i << "/" << count;
            std::cout.flush();
          }
          if (!(i % slice))
            sleep(1);
          if (centengine.wait(0))
            break ;
          int service_id(random() % passiveservices + 1);
          ofs << "[" << now << "] PROCESS_SERVICE_CHECK_RESULT;"
              << (service_id - 1) / (passiveservices / passivehosts) + 1 << ";"
              << service_id << ";" << random() % 4 << ";output\n";
        }
        ofs.close();
      }
    }
    time_t send_time(time(NULL));
    std::cout << "\rSending passive check results...                Done               \n";

    // Wait for Centreon Engine.
    std::cout << "Waiting for Centreon Engine...                  ";
    std::cout.flush();
    centengine.wait();
    time_t end_time(time(NULL));
    std::cout << "Done\n";

    // Print results.
    std::cout << "\n"
              << "  Total passive check results                   "
              << count << "\n"
	      << "  Total send time in seconds                    "
	      << send_time - start_time << "\n"
              << "  Total processing time in seconds              "
              << end_time - start_time << "\n"
	      << "  Average check results sent per second         "
	      << static_cast<double>(count) / ((send_time != start_time)
					       ? (send_time - start_time)
					       : 1)
	      << "\n"
              << "  Average check results processed per second    "
              << static_cast<double>(count) / ((end_time != start_time)
                                               ? (end_time - start_time)
                                               : 1)
              << "\n";
  }
  // Generate configuration files.
  else if (mode == "configuration") {
    std::string additional("broker_module=");
    additional.append(module);
    additional.append("\n");
    engine_cfg cfg_files(
                 additional,
                 count,
                 activehosts,
                 activeservices,
                 passivehosts,
                 passiveservices,
                 false);
    std::cout << "Configuration files generated in "
              << cfg_files.directory() << "\n";
  }
  // Generate external commands.
  else if (mode == "commands") {
    time_t now(time(NULL));
    for (int i(0), limit(count * 105 / 100); i < limit; ++i) {
      int service_id(random() % passiveservices + 1);
      std::cout << "[" << now << "] PROCESS_SERVICE_CHECK_RESULT;"
                << (service_id - 1) / (passiveservices / passivehosts) + 1 << ";"
                << service_id << ";" << random() % 4 << ";output\n";
    }
  }
  // Print help.
  else {
    std::cout
      << "Common options\n"
      << "  -? --help             Print this help.\n"
      << "  -M --mode             This tool has multiple modes : 'benchmark'\n"
      << "                        which compute the time needed to process some\n"
      << "                        count of passive check results, 'configuration'\n"
      << "                        which generate configuration files compatible\n"
      << "                        with the latest mode 'commands' which print on\n"
      << "                        standard output passive check results.\n"
      << "  -h --activehosts      Number of active hosts in the configuration (default is "
      << activehosts << ").\n"
      << "  -s --activeservices   Number of active services in the configuration (default is "
      << activeservices << ").\n"
      << "  -H --passivehosts     Number of passive hosts in the configuration. (default is "
      << passivehosts << ").\n"
      << "  -S --passiveservices  Number of passive services in the configuration (default is "
      << passiveservices << ").\n"
      << "  -c --count            Number of passive check results to send (default is "
      << count << ")\n"
      << "Benchmark options\n"
      << "  -e --engine           Centreon Engine binary (default is "
      << engine << ")\n"
      << "  -m --module           Centreon Engine external command module (default is "
      << module << ")\n"
      << "\n"
      << "This benchmarking tool aims to mesure the time needed by\n"
      << "Centreon Engine to process some amount of passive service\n"
      << "checks provided through external commands. This tool has\n"
      << "multiple mode. First one is 'benchmark' which computes time\n"
      << "needed to process some count of passive check results. This\n"
      << "mode automatically generate configuration files and write\n"
      << "external commands. The 'configuration' mode will only generate\n"
      << "configuration files on a temporary folder which path will be\n"
      << "printed. The latest mode 'command' print on the standard output\n"
      << "some count of external commands compatible with a configuration\n"
      << "file generated with the same parameters.\n";
  }

  return (EXIT_SUCCESS);
}
