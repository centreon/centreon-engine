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
    result res1;
    if (res1.get_cmd_id() != 0
	|| res1.get_stdout() != ""
	|| res1.get_stderr() != ""
	|| res1.get_retval() != 0
	|| res1.get_is_timeout() != false
	|| res1.get_exited_ok () != true
	|| res1.get_start_time().tv_sec != 0
	|| res1.get_start_time().tv_usec != 0
	|| res1.get_end_time().tv_sec != 0
	|| res1.get_end_time().tv_usec != 0) {
      qDebug() << "error: Default constructor failed.";
      return (1);
    }

    timeval time;
    gettimeofday(&time, NULL);

    result res2(DEFAULT_ID,
		DEFAULT_STDOUT,
		DEFAULT_STDERR,
		time,
		time,
		DEFAULT_RETURN,
		DEFAULT_TIMEOUT,
		DEFAULT_EXIT_OK);
    if (res2.get_cmd_id() != DEFAULT_ID
	|| res2.get_stdout() != DEFAULT_STDOUT
	|| res2.get_stderr() != DEFAULT_STDERR
	|| res2.get_retval() != DEFAULT_RETURN
	|| res2.get_is_timeout() != DEFAULT_TIMEOUT
	|| res2.get_exited_ok () != DEFAULT_EXIT_OK
	|| res2.get_start_time().tv_sec != time.tv_sec
	|| res2.get_start_time().tv_usec != time.tv_usec
	|| res2.get_end_time().tv_sec != time.tv_sec
	|| res2.get_end_time().tv_usec != time.tv_usec) {
      qDebug() << "error: Constructor failed.";
      return (1);
    }

    result res3(res2);
    if (res2 != res3) {
      qDebug() << "error: Default copy constructor failed.";
      return (1);
    }

    result res4 = res3;
    if (res2 != res4) {
      qDebug() << "error: Default copy operator failed.";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
