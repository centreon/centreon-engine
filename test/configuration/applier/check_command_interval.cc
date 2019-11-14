/*
** Copyright 2011-2014 Merethis
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
#include <string>
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects/comment.hh"
#include "com/centreon/engine/objects/downtime.hh"
#include "com/centreon/engine/retention/parser.hh"
#include "com/centreon/engine/retention/state.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**
 *  Read configuration with new parser.
 *
 *  @parser[out] g        Fill global variable.
 *  @param[in]  filename  The file path to parse.
 *  @parse[in]  options   The options to use.
 *
 *  @return True on succes, otherwise false.
 */
static bool newparser_read_config(std::string const& filename,
                                  unsigned int options) {
  bool ret(false);
  try {
    init_macros();
    // Parse configuration.
    configuration::state config;

    // tricks to bypass create log file.
    config.log_file("");

    {
      configuration::parser p(options);
      p.parse(filename, config);
    }

    // Parse retention.
    retention::state state;
    try {
      retention::parser p;
      p.parse(config.state_retention_file(), state);
    } catch (...) {
    }

    configuration::applier::state::instance().apply(config, state);
    ret = true;
  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
  }
  return (ret);
}

/**
 *  Check if the configuration parser works properly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  if (argc != 3)
    throw(engine_error() << "usage: " << argv[0] << " file.cfg"
                         << " expected_value");

  unsigned int options(configuration::parser::read_all);
  int expected_value = atoi(argv[2]);

  if (!newparser_read_config(argv[1], options))
    throw(engine_error() << "new parser can't parse " << argv[1]);

  if (::command_check_interval != expected_value)
    throw(engine_error() << "value found " << ::command_check_interval
                         << "was not the expected value");

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
