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
 *  Run stop_executing_host_checks test.
 */
static void check_stop_executing_host_checks() {
  execute_host_checks = true;
  char const* cmd("[1317196300] STOP_EXECUTING_HOST_CHECKS");
  process_external_command(cmd);

  if (execute_host_checks)
    throw (engine_error() << "stop_executing_host_checks failed.");
}

/**
 *  Check processing of stop_executing_host_checks works.
 */
int main(void) {
  try {
    check_stop_executing_host_checks();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
