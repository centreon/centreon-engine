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
 *  Run enable_flap_detection test.
 */
static void check_enable_flap_detection() {
  config.set_enable_flap_detection(false);
  char const* cmd("[1317196300] ENABLE_FLAP_DETECTION");
  process_external_command(cmd);

  if (!config.get_enable_flap_detection())
    throw (engine_error() << "enable_flap_detection failed.");
}

/**
 *  Check processing of enable_flap_detection works.
 */
int main(void) {
  try {
    check_enable_flap_detection();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
