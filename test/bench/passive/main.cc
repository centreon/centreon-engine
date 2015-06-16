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

#include <cstdlib>
#include <fstream>
#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif // HAVE_GETOPT_H
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include "com/centreon/clib.hh"
#include "com/centreon/process.hh"
#include "engine_cfg.hh"

/**
 *  Bench how long Centreon Engine needs to process some passive check
 *  results.
 *
 *  @return EXIT_SUCCESS.
 */
int main(int argc, char* argv[]) {
  // Initialization.
  com::centreon::clib::load();
  srandom(getpid());

  // Options.
#ifdef HAVE_GETOPT_H
  int option_index(0);
  static struct option const long_options[] = {
    { "help", no_argument, NULL, 'h' },
    { "engine", required_argument, NULL, 'e' },
    { "module", required_argument, NULL, 'm' },
    { "count", required_argument, NULL, 'c' },
    { NULL, no_argument, NULL, '\0' }
  };
#endif // HAVE_GETOPT_H
  bool help(false);
  std::string engine("/usr/sbin/centengine");
  std::string module("/usr/lib64/centreon-engine/externalcmd.so");
  int count(1000);

  // Process command line arguments.
  int c;
#ifdef HAVE_GETOPT_H
  while ((c = getopt_long(
                argc,
                argv,
                "+he:m:c:",
                long_options,
                &option_index)) != -1) {
#else
  while ((c = getopt(argc, argv, "+he:m:c:")) != -1) {
#endif // HAVE_GETOPT_H
    switch (c) {
    case 'h':
      help = true;
      break ;
    case 'e':
      engine = optarg;
      break ;
    case 'm':
      module = optarg;
      break ;
    case 'c':
      count = strtol(optarg, NULL, 0);
      break ;
    }
  }

  // Banner.
  std::cout << "---------------------------------------------\n"
            << "Centreon Engine passive checks benchmark tool\n"
            << "---------------------------------------------\n"
            << "\n";

  // Print help.
  if (help) {
    std::cout
      << "  -h --help    Print this help.\n"
      << "  -e --engine  Centreon Engine binary (default is " << engine << ")\n"
      << "  -m --module  Centreon Engine external command module (default is " << module << ")\n"
      << "  -c --count   Number of passive check results to send (default is " << count << ")\n"
      << "\n"
      << "This benchmarking tool aims to mesure the time needed by\n"
      << "Centreon Engine to process some amount of passive service\n"
      << "checks provided through external commands. This tool will\n"
      << "create a temporary configuration folder containing 1 host\n"
      << "and 100 services. It will then run Centreon Engine and\n"
      << "provide randomly check results for the services. When\n"
      << "finished, it will wait for Centreon Engine termination and\n"
      << "print the total number of external command processed along\n"
      << "with the average number of passive check results processed\n"
      << "per second.\n";
  }
  // Perform benchmark.
  else {
    // Generate configuration files.
    std::cout << "Generating configuration files...               ";
    std::cout.flush();
    std::string additional("broker_module=");
    additional.append(module);
    additional.append("\n");
    engine_cfg cfg_files(additional, count);
    std::cout << "Done\n";

    // Launch Centreon Engine.
    std::cout << "Starting Centreon Engine...                     ";
    std::cout.flush();
    std::string cmdline(engine);
    cmdline.append(" ");
    cmdline.append(cfg_files.main_file());
    com::centreon::process centengine;
    centengine.enable_stream(com::centreon::process::in, false);
    centengine.enable_stream(com::centreon::process::out, false);
    centengine.enable_stream(com::centreon::process::err, false);
    centengine.exec(cmdline);
    sleep(3);
    time_t start_time(time(NULL));
    std::cout << "Done\n";

    // Send external commands.
    {
      time_t now(time(NULL));
      // Send a little bit more external commands as writing and reading
      // to the same pipe is not thread safe.
      for (int i(0), limit(count * 105 / 100); i < limit; ++i) {
        if (!(i % 1000)) {
          std::cout << "\rSending passive check results...                "
                    << i << "/" << count;
          std::cout.flush();
        }
        std::ostringstream externalcmd;
        externalcmd << "[" << now << "] PROCESS_SERVICE_CHECK_RESULT;1;"
                    << (random() % 100) + i << ";" << random() % 4
                    << ";output\n";
        std::ofstream ofs;
        ofs.open(
              cfg_files.command_file().c_str(),
              std::ios_base::out | std::ios_base::app);
        if (ofs.good()) {
          ofs << "[" << now << "] PROCESS_SERVICE_CHECK_RESULT;1;"
              << (random() % 100) + 1 << ";" << random() % 4
              << ";output\n";
          ofs.close();
        }
      }
    }
    std::cout << "\rSending passive check results...                Done               \n";

    // Wait for Centreon Engine.
    std::cout << "Waiting for Centreon Engine...                  ";
    std::cout.flush();
    centengine.wait();
    time_t end_time(time(NULL));
    std::cout << "Done\n";

    // Print results.
    std::cout << "\n"
              << "  Total passive check results processed         "
              << count << "\n"
              << "  Total time in seconds                         "
              << end_time - start_time << "\n"
              << "  Average check results processed per second    "
              << static_cast<double>(count) / ((end_time != start_time)
                                               ? (end_time - start_time)
                                               : 1)
              << "\n";
  }

  // Unload Clib.
  com::centreon::clib::unload();

  return (EXIT_SUCCESS);
}
