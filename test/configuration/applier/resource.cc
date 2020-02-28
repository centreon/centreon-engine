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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/io/file_stream.hh"
#include "test/unittest.hh"

#include <iostream>

using namespace com::centreon;
using namespace com::centreon::engine;

static std::string write_config_file(std::string const& resource_file) {
  std::string filename(io::file_stream::temp_path());

  io::file_stream fs;
  fs.open(filename, "w");
  std::string data("resource_file=" + resource_file + "\n");
  fs.write(data.c_str(), data.size());
  fs.close();
  return (filename);
}

static std::string write_resource_file() {
  std::ostringstream oss;
  for (unsigned int i(0); i < MAX_USER_MACROS - 1; ++i)
    oss << "$USER" << (i + 1) << "$=resource_" << (i + 1) << "\n";

  std::string filename(io::file_stream::temp_path());

  io::file_stream fs;
  fs.open(filename, "w");
  std::string const& data(oss.str());
  fs.write(data.c_str(), data.size());
  fs.close();
  return (filename);
}

/**
 *  Check if the configuration parser and applier works properly
 *  for the user macros.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  std::string resource_file(write_resource_file());
  std::string config_file(write_config_file(resource_file));

  try {
    configuration::state cfg;

    // tricks to bypass create log file.
    cfg.log_file("");

    configuration::parser p;
    p.parse(config_file, cfg);

    configuration::applier::state::instance().apply(cfg);

    umap<std::string, std::string> users(cfg.user());
    for (unsigned int i(0); i < MAX_USER_MACROS - 1; ++i) {
      std::string key = std::string("USER") + string::from(i + 1);
      if (!macro_user[i] || macro_user[i] != users[key])
        throw(engine_error() << "apply configuration resources "
                                "failed: global macro_user["
                             << i
                             << "] is not equal "
                                "to configuration::state::user["
                             << i << "]");
      std::ostringstream oss;
      oss << "resource_" << (i + 1);
      if (users[key] != oss.str())
        throw(engine_error()
              << "parse configuration resources "
                 "failed: invalid data into configuration::state::user["
              << i << "]: value = '" << users[key] << "'");
    }
  } catch (...) {
    io::file_stream::remove(config_file);
    io::file_stream::remove(resource_file);
    throw;
  }
  io::file_stream::remove(config_file);
  io::file_stream::remove(resource_file);
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
