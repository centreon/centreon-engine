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
 *  Check the constructor and copy object.
 */
int main_test() {
  result res1;
  if (res1.get_command_id() != 0
      || res1.get_stdout() != ""
      || res1.get_stderr() != ""
      || res1.get_exit_code() != 0
      || res1.get_is_timeout() != false
      || res1.get_is_executed () != true
      || res1.get_start_time().tv_sec != 0
      || res1.get_start_time().tv_usec != 0
      || res1.get_end_time().tv_sec != 0
      || res1.get_end_time().tv_usec != 0)
    throw (engine_error() << "error: Default constructor failed.");

  QDateTime time = QDateTime::currentDateTime();

  result res2(DEFAULT_ID,
              DEFAULT_STDOUT,
              DEFAULT_STDERR,
              time,
              time,
              DEFAULT_RETURN,
              DEFAULT_TIMEOUT,
              DEFAULT_EXIT_OK);
  if (res2.get_command_id() != DEFAULT_ID
      || res2.get_stdout() != DEFAULT_STDOUT
      || res2.get_stderr() != DEFAULT_STDERR
      || res2.get_exit_code() != DEFAULT_RETURN
      || res2.get_is_timeout() != DEFAULT_TIMEOUT
      || res2.get_is_executed() != DEFAULT_EXIT_OK
      || res2.get_start_time().tv_sec != static_cast<time_t>(time.toTime_t())
      || res2.get_start_time().tv_usec != 0
      || res2.get_end_time().tv_sec != static_cast<time_t>(time.toTime_t())
      || res2.get_end_time().tv_usec != 0)
    throw (engine_error() << "error: Constructor failed.");

  result res3(res2);
  if (res2 != res3)
    throw (engine_error() << "error: Default copy constructor failed.");

  result res4 = res3;
  if (res2 != res4)
    throw (engine_error() << "error: Default copy operator failed.");

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
