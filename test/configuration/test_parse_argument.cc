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

#include "configuration/state.hh"

using namespace com::centreon::engine;

/**
 *  Check parse with directory.
 */
void check_directory() {
  try {
    configuration::state config;
    config.parse("./");
  }
  catch (std::exception const& e) {
    (void)e;
  }
  throw (engine_error() << "configure::parse didn't failed.");
}

/**
 *  Check the parsing argument.
 */
int main(void) {
  try {
    check_directory();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  catch (...) {
    qDebug() << "error: catch all.";
    return (1);
  }
  return (0);
}
