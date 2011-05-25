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
#include <sys/time.h>
#include "commands/result.hh"

using namespace com::centreon::engine::commands;

#define DEFAULT_ID 42
#define DEFAULT_STDOUT "stdout string test"
#define DEFAULT_STDERR "stderr string test"
#define DEFAULT_RETURN 0
#define DEFAULT_TIMEOUT true
#define DEFAULT_EXIT_OK false

int main() {
  try {
    timeval time;
    gettimeofday(&time, NULL);

    result res;
    res.set_cmd_id(DEFAULT_ID);
    res.set_retval(DEFAULT_RETURN);
    res.set_start_time(time);
    res.set_end_time(time);
    res.set_stdout(DEFAULT_STDOUT);
    res.set_stderr(DEFAULT_STDERR);
    res.set_exited_ok(DEFAULT_EXIT_OK);
    res.set_is_timeout(DEFAULT_TIMEOUT);

    if (res.get_cmd_id() != DEFAULT_ID) {
      qDebug() << "error: cmd_id invalid value.";
      return (1);
    }

    if (res.get_retval() != DEFAULT_RETURN) {
      qDebug() << "error: retval invalid value.";
      return (1);
    }

    if (res.get_execution_time() != 0.0) {
      qDebug() << "error: execution_time invalid value.";
      return (1);
    }

    if (res.get_start_time().tv_sec != time.tv_sec
	|| res.get_start_time().tv_usec != time.tv_usec) {
      qDebug() << "error: start_time invalid value.";
      return (1);
    }

    if (res.get_end_time().tv_sec != time.tv_sec
	|| res.get_end_time().tv_usec != time.tv_usec) {
      qDebug() << "error: end_time invalid value.";
      return (1);
    }

    if (res.get_stdout() != DEFAULT_STDOUT) {
      qDebug() << "error: stdout invalid value.";
      return (1);
    }

    if (res.get_stderr() != DEFAULT_STDERR) {
      qDebug() << "error: stderr invalid value.";
      return (1);
    }

    if (res.get_exited_ok() != DEFAULT_EXIT_OK) {
      qDebug() << "error: exited_ok invalid value.";
      return (1);
    }

    if (res.get_is_timeout() != DEFAULT_TIMEOUT) {
      qDebug() << "error: timeout invalid value.";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }

  return (0);
}
