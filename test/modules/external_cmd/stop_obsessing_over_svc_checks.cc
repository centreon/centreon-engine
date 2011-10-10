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

#include <QCoreApplication>
#include <QDebug>
#include <exception>
#include "test/testing.hh"
#include "logging/engine.hh"
#include "error.hh"
#include "commands.hh"
#include "globals.hh"

using namespace com::centreon::engine;

/**
 *  Run stop_obsessing_over_svc_checks test.
 */
static void check_stop_obsessing_over_svc_checks() {
  config.set_obsess_over_services(true);
  char const* cmd("[1317196300] STOP_OBSESSING_OVER_SVC_CHECKS");
  process_external_command(cmd);

  if (config.get_obsess_over_services())
    throw (engine_error() << "stop_obsessing_over_svc_checks failed.");
}

/**
 *  Check processing of stop_obsessing_over_svc_checks works.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    testing init;

    logging::engine& engine = logging::engine::instance();
    check_stop_obsessing_over_svc_checks();
    engine.cleanup();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
