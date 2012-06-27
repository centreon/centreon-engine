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

#define DEFAULT_ID 42
#define DEFAULT_STDOUT "stdout string test"
#define DEFAULT_STDERR "stderr string test"
#define DEFAULT_RETURN 0
#define DEFAULT_TIMEOUT true
#define DEFAULT_EXIT_OK false

/**
 *  Check setter and getter return.
 */
int main_test() {
  QDateTime time = QDateTime::currentDateTime();

  result res;
  res.set_command_id(DEFAULT_ID);
  res.set_exit_code(DEFAULT_RETURN);
  res.set_start_time(time);
  res.set_end_time(time);
  res.set_stdout(DEFAULT_STDOUT);
  res.set_stderr(DEFAULT_STDERR);
  res.set_is_executed(DEFAULT_EXIT_OK);
  res.set_is_timeout(DEFAULT_TIMEOUT);

  if (res.get_command_id() != DEFAULT_ID)
    throw (engine_error() << "error: command_id invalid value.");

  if (res.get_exit_code() != DEFAULT_RETURN)
    throw (engine_error() << "error: exit_code invalid value.");

  if (res.get_execution_time() != 0.0)
    throw (engine_error() << "error: execution_time invalid value.");

  if (res.get_start_time().tv_sec != static_cast<time_t>(time.toTime_t())
      || res.get_start_time().tv_usec != 0)
    throw (engine_error() << "error: start_time invalid value.");

  if (res.get_end_time().tv_sec != static_cast<time_t>(time.toTime_t())
      || res.get_end_time().tv_usec != 0)
    throw (engine_error() << "error: end_time invalid value.");

  if (res.get_stdout() != DEFAULT_STDOUT)
    throw (engine_error() << "error: stdout invalid value.");

  if (res.get_stderr() != DEFAULT_STDERR)
    throw (engine_error() << "error: stderr invalid value.");

  if (res.get_is_executed() != DEFAULT_EXIT_OK)
    throw (engine_error() << "error: is_executed invalid value.");

  if (res.get_is_timeout() != DEFAULT_TIMEOUT)
    throw (engine_error() << "error: timeout invalid value.");

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
