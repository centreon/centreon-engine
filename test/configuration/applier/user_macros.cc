/*
** Copyright 2011-2013 Merethis
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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/io/file_stream.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**
 *  Write a configuration file that contains user-defined macros.
 *
 *  @return Path to the generated file.
 */
static std::string write_config_file() {
  std::string filename(io::file_stream::temp_path());
  io::file_stream fs;
  fs.open(filename, "w");
  std::string data;
  {
    std::ostringstream oss;
    for (int i(1); i <= MAX_USER_MACROS; ++i)
      oss << "$USER" << i << "$=resource_" << i << "\n";
    data = oss.str();
  }
  fs.write(data.c_str(), data.size());
  fs.close();
  return (filename);
}

/**
 *  Check if the configuration parser and applier works properly
 *  for the user macros.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Generate config file.
  std::string config_file(write_config_file());

  try {
    // Parse and apply configuration on objects.
    configuration::state cfg;
    cfg.log_file(""); // Trick to bypass log file creation.
    configuration::parser p;
    p.parse(config_file, cfg);
    configuration::applier::state::instance().apply(cfg);

    // Check that macros exist and have proper content.
    std::vector<std::string> const& users(cfg.user());
    for (int i(0); i < MAX_USER_MACROS; ++i) {
      if (!macro_user[i] || (macro_user[i] != users[i]))
        throw (engine_error()
               << "user-defined macros application failed: "
                  "global macro_user[" << i << "] is not equal "
                  "to configuration::state::user[" << i << "] "
               << "($USER" << i + 1 << "$)");
      std::ostringstream oss;
      oss << "resource_" << (i + 1);
      if (users[i] != oss.str())
        throw (engine_error() << "user-defined macros parsing failed: "
                  "invalid data into configuration::state::user["
               << i << "]: got '" << users[i] << "', expected '"
               << oss.str() << "' ($USER" << i + 1 << "$)");
    }
  }
  catch (...) {
    io::file_stream::remove(config_file);
    throw ;
  }
  io::file_stream::remove(config_file);
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char* argv[]) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
