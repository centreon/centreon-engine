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
#include "logging/engine.hh"
#include "error.hh"
#include "commands.hh"
#include "globals.hh"

using namespace com::centreon::engine;

/**
 *  Run stop_accepting_passive_svc_checks test.
 */
static void check_stop_accepting_passive_svc_checks() {
  config.set_accept_passive_service_checks(true);
  char const* cmd("[1317196300] STOP_ACCEPTING_PASSIVE_SVC_CHECKS");
  process_external_command(cmd);

  if (config.get_accept_passive_service_checks())
    throw (engine_error() << "stop_accepting_passive_svc_checks failed.");
}

/**
 *  Check processing of stop_accepting_passive_svc_checks works.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    logging::engine& engine = logging::engine::instance();
    check_stop_accepting_passive_svc_checks();
    engine.cleanup();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
