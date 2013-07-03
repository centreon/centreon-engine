/*
** Copyright 1999-2008 Ethan Galstad
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

#include "com/centreon/engine/configuration/applier/engine.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

extern "C" {

/**
 *  Read all host configuration data from external source.
 *
 *  @param[in] main_config_file Main configuration file.
 *  @param[in] options          Options.
 *  @param[in] cache            Cache.
 *  @param[in] precache         Precache.
 *
 *  @return 0 on success.
 */
int read_object_config_data(
      char const* main_config_file,
      int options,
      int cache,
      int precache) {
  // XXX: cache and precache are unused.
  (void)cache;
  (void)precache;

  int ret(ERROR);
  try {
    configuration::state config;
    configuration::parser p(options);
    p.parse(main_config_file, config);
    configuration::applier::engine::instance().apply(config);
    ret = OK;
  }
  catch (std::exception const& e) {
    logger(log_config_error, basic)
      << e.what();
  }
  return (ret);
}

}
