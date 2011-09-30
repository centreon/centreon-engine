/*
** Copyright 2011 Merethis
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

#include <QDebug>
#include <exception>
#include "error.hh"
#include "commands.hh"
#include "globals.hh"
#include "error.hh"

/**
 *  Run enable_failure_prediction test.
 */
static void check_enable_failure_prediction() {
  config.set_enable_failure_prediction(false);
  char const* cmd("[1317196300] ENABLE_FAILURE_PREDICTION");
  process_external_command(cmd);

  if (!config.get_enable_failure_prediction())
    throw (engine_error() << "enable_failure_prediction failed.");
}

/**
 *  Check processing of enable_failure_prediction works.
 */
int main(void) {
  try {
    check_enable_failure_prediction();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
