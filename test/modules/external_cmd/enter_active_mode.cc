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
 *  Run enter_active_mode test.
 */
static void check_enter_active_mode() {
  config.set_enable_notifications(false);
  char const* cmd("[1317196300] ENTER_ACTIVE_MODE");
  process_external_command(cmd);

  if (!config.get_enable_notifications())
    throw (engine_error() << "enter_active_mode failed.");
}

/**
 *  Check processing of enter_active_mode works.
 */
int main(void) {
  try {
    check_enter_active_mode();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
