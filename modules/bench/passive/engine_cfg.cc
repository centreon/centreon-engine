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

#include "engine_cfg.hh"
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "paths.hh"

/**
 *  Generate Centreon Engine configuration files.
 *
 *  @param[in] additional        Additional configuration directives.
 *  @param[in] expected_passive  Expected passive checks.
 *  @param[in] active_hosts      Number of active hosts to generate.
 *  @param[in] active_services   Number of active services to generate.
 *  @param[in] passive_hosts     Number of passive hosts to generate.
 *  @param[in] passive_services  Number of passive services to generate.
 *  @param[in] auto_delete       True if generated configuration should
 *                               be deleted at object destruction.
 */
engine_cfg::engine_cfg(std::string const& additional,
                       int expected_passive,
                       int active_hosts,
                       int active_services,
                       int passive_hosts,
                       int passive_services,
                       bool auto_delete)
    : _auto_delete(auto_delete) {
  // Create directory.
  _directory = tmpnam(NULL);
  ::mkdir(_directory.c_str(), 0777);

  // Write object files.
  std::string object_file(_directory);
  object_file.append("/objects.cfg");
  {
    // Command.
    std::ostringstream oss;
    oss << "define command{\n"
        << "  command_name default_command\n"
        << "  command_line /bin/true\n"
        << "}\n";

    // Hosts.
    for (int i(0), limit(active_hosts + passive_hosts); i < limit; ++i)
      oss << "define host{\n"
          << "  host_name " << i + 1 << "\n"
          << "  address localhost\n"
          << "  active_checks_enabled " << ((i < passive_hosts) ? "0" : "1")
          << "\n"
          << "  max_check_attempts 1\n"
          << "  check_command default_command\n"
          << "}\n";

    // Passive services.
    for (int i(0); i < passive_services; ++i)
      oss << "define service{\n"
          << "  service_description " << i + 1 << "\n"
          << "  host_name " << i / (passive_services / passive_hosts) + 1
          << "\n"
          << "  active_checks_enabled 0\n"
          << "  check_command default_command\n"
          << "  max_check_attempts 1\n"
          << "  check_interval 1\n"
          << "  retry_interval 1\n"
          << "}\n";

    // Active services.
    for (int i(0); i < active_services; ++i)
      oss << "define service {\n"
          << "  service_description " << i + passive_services + 1 << "\n"
          << "  host_name "
          << i / (active_services / active_hosts) + passive_hosts + 1 << "\n"
          << "  active_checks_enabled 1\n"
          << "  check_command default_command\n"
          << "  max_check_attempts 1\n"
          << "  check_interval 1\n"
          << "  retry_interval 1\n"
          << "}\n";

    // Write objects file.
    _write_file(object_file, oss.str());
    _generated.push_back(object_file);
  }

  // Command file will probably be generated.
  _generated.push_back(command_file());

  // Write main file.
  {
    std::ostringstream oss;
    oss << "log_file=/dev/null\n"
        // << "log_file=/tmp/centengine.log\n"
        // << "debug_file=/tmp/centengine.debug\n"
        // << "max_debug_file_size=10000000000\n"
        // << "debug_level=-1\n"
        // << "debug_verbosity=2\n"
        << "cached_host_check_horizon=0\n"
        << "command_check_interval=-1\n"
        << "command_file=" << command_file() << "\n"
        << "state_retention_file=\n"
        << "sleep_time=0.02\n"
        << "interval_length=10\n"
        << "cfg_file=" << object_file << "\n";
    if (expected_passive) {
      static char const* const potential_modules[] = {
          PROJECT_SOURCE_DIR "/build/test/bench/bench_passive_module.so",
          "/usr/lib64/centreon-engine/bench_passive_module.so",
          "/usr/lib/centreon-engine/bench_passive_module.so", NULL};
      for (int i(0); potential_modules[i]; ++i)
        if (!access(potential_modules[i], F_OK))
          oss << "broker_module=" << potential_modules[i] << " "
              << expected_passive << "\n";
    }
    oss << additional;
    _write_file(main_file(), oss.str());
    _generated.push_back(main_file());
  }
}

/**
 *  Remove generated configuration files.
 */
engine_cfg::~engine_cfg() {
  if (!_directory.empty() && _auto_delete) {
    for (std::list<std::string>::const_iterator it(_generated.begin()),
         end(_generated.end());
         it != end; ++it)
      ::remove(it->c_str());
    ::rmdir(_directory.c_str());
  }
}

/**
 *  Get the command file.
 *
 *  @return Command file.
 */
std::string engine_cfg::command_file() const {
  std::string retval(_directory);
  retval.append("/centengine.cmd");
  return (retval);
}

/**
 *  Get the directory in which configuration files were generated.
 *
 *  @return Directory in which configuration files were generated.
 */
std::string engine_cfg::directory() const {
  return (_directory);
}

/**
 *  Get the main configuration file.
 *
 *  @return Main configuration file.
 */
std::string engine_cfg::main_file() const {
  std::string retval(_directory);
  retval.append("/centengine.cfg");
  return (retval);
}

/**
 *  Write a file.
 *
 *  @param[in] target   Target file.
 *  @param[in] content  Content of target file.
 */
void engine_cfg::_write_file(std::string const& target,
                             std::string const& content) {
  std::ofstream ofs;
  ofs.open(target.c_str());
  if (ofs.good()) {
    ofs.write(content.data(), content.size());
    ofs.close();
  }
  return;
}
