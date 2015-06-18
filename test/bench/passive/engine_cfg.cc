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

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include "engine_cfg.hh"
#include "test/paths.hh"

/**
 *  Generate Centreon Engine configuration files.
 *
 *  @param[in] additional        Additional configuration directives.
 *  @param[in] expected_passive  Expected passive checks.
 */
engine_cfg::engine_cfg(
              std::string const& additional,
              int expected_passive) {
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

    // Host.
    oss << "define host{\n"
        << "  host_name 1\n"
        << "  address localhost\n"
        << "  active_checks_enabled 0\n"
        << "  max_check_attempts 1\n"
        << "  check_command default_command\n"
        << "}\n";

    // Services.
    for (int i(0); i < 100; ++i)
      oss << "define service{\n"
          << "  service_description " << i + 1 << "\n"
          << "  host_name 1\n"
          << "  active_checks_enabled 0\n"
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
    oss
      << "log_file=/dev/null\n"
      // << "log_file=/tmp/centengine.log\n"
      << "cached_host_check_horizon=1\n"
      << "command_check_interval=-1\n"
      << "command_file=" << command_file() << "\n"
      << "state_retention_file=\n"
      << "cfg_file=" << object_file << "\n"
      << "broker_module=" << PROJECT_SOURCE_DIR << "/build/test/bench/bench_passive_module.so "
      << expected_passive << "\n"
      << additional;
    _write_file(main_file(), oss.str());
    _generated.push_back(main_file());
  }
}

/**
 *  Remove generated configuration files.
 */
engine_cfg::~engine_cfg() {
  if (!_directory.empty()) {
    for (std::list<std::string>::const_iterator
           it(_generated.begin()),
           end(_generated.end());
         it != end;
         ++it)
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
void engine_cfg::_write_file(
                   std::string const& target,
                   std::string const& content) {
  std::ofstream ofs;
  ofs.open(target.c_str());
  if (ofs.good()) {
    ofs.write(content.data(), content.size());
    ofs.close();
  }
  return ;
}
