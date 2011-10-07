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
 *  Run change_host_check_timeperiod test.
 */
static void check_change_host_check_timeperiod() {
  init_object_skiplists();

  host* hst = add_host("name", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42,
                       0, 0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0,
                       0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
                       NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0.0, 0.0,
                       0.0, 0, 0, 0, 0, 0);
  if (!hst)
    throw (engine_error() << "create host failed.");

  timeperiod* tperiod = add_timeperiod("tperiod", "alias");
  if (!tperiod)
    throw (engine_error() << "create timeperiod failed.");

  hst->check_period_ptr = NULL;
  char const* cmd("[1317196300] CHANGE_HOST_CHECK_TIMEPERIOD;name;tperiod");
  process_external_command(cmd);

  if (hst->check_period_ptr != tperiod)
    throw (engine_error() << "change_host_check_timeperiod failed.");

  delete[] hst->name;
  delete[] hst->display_name;
  delete[] hst->alias;
  delete[] hst->address;
  delete[] hst->check_period;
  delete hst;

  delete[] tperiod->name;
  delete[] tperiod->alias;
  delete tperiod;

  free_object_skiplists();
}

/**
 *  Check processing of change_host_check_timeperiod works.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    logging::engine& engine = logging::engine::instance();
    check_change_host_check_timeperiod();
    engine.cleanup();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
