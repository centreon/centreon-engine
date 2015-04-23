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

#define MACRO_VALUE "I love Centreon"

/**
 *  Write a configuration file that contains a user-defined macro.
 *
 *  @return Path to the generated file.
 */
static std::string write_included_file() {
  std::string filename(io::file_stream::temp_path());
  io::file_stream fs;
  fs.open(filename, "w");
  std::string data("$USER1$=" MACRO_VALUE "\n");
  fs.write(data.c_str(), data.size());
  fs.close();
  return (filename);
}

/**
 *  Write a configuration file that includes another file.
 *
 *  @param[in] include_file  File to include.
 *
 *  @return Path to the generated file.
 */
static std::string write_main_file(std::string const& include_file) {
  std::string filename(io::file_stream::temp_path());
  io::file_stream fs;
  fs.open(filename, "w");
  std::string data;
  data.append("cfg_include=");
  data.append(include_file);
  data.append("\n");
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
  std::string included_file(write_included_file());
  std::string main_file(write_main_file(included_file));

  try {
    // Parse and apply configuration on objects.
    configuration::state cfg;
    cfg.log_file(""); // Trick to bypass log file creation.
    configuration::parser p;
    p.parse(main_file, cfg);
    configuration::applier::state::instance().apply(cfg);

    // Check that macros exist and have proper content.
    std::vector<std::string> const& users(cfg.user());
    if (!macro_user[0]
        || users.empty()
        || (macro_user[0] != users[0]))
      throw (engine_error()
             << "application of included file failed: "
                "test macro was not applied");
    if (users[0] != MACRO_VALUE)
      throw (engine_error() << "parsing of included file failed: "
                "invalid test macro value: got '" << users[0]
             << "', expected '" MACRO_VALUE "'");
  }
  catch (...) {
    io::file_stream::remove(included_file);
    io::file_stream::remove(main_file);
    throw ;
  }
  io::file_stream::remove(included_file);
  io::file_stream::remove(main_file);
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char* argv[]) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
