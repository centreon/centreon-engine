/*
** Copyright 2011-2012 Merethis
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

#include <ctime>
#include <exception>
#include <QCoreApplication>
#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Calculate the execution time.
 *
 *  @param[in] start The start time.
 *  @param[in] end   The end time.
 *
 *  @return The execution time, diff between end and start time.
 */
static uint execution_time(QDateTime const& start, QDateTime const& end) {
  if (end.toTime_t() < start.toTime_t())
    return (0);
  return (end.toTime_t() - start.toTime_t());
}

/**
 *  Check if the result execution time works.
 */
int main_test() {
  QDateTime start = QDateTime::currentDateTime();

  QDateTime end = start;
  end.addSecs(10);
  end.addMSecs(20);

  result res1(0, "", "", start, end);
  if (res1.get_execution_time() != execution_time(start, end))
    throw (engine_error() << "error: execution_time invalid value.");

  result res2(0, "", "", end, start);
  if (res2.get_execution_time() != 0)
    throw (engine_error() << "error: execution_time != 0");

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&main_test);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}
