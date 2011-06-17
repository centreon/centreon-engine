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

/**
 *  Calculate the execution time.
 *
 *  @param[in] start The start time.
 *  @param[in] end   The end time.
 *
 *  @return The execution time, diff between end and start time.
 */
static double execution_time(QDateTime const& start, QDateTime const& end) {
  double res =
    (double)(end.toMSecsSinceEpoch() / 1000 - start.toMSecsSinceEpoch() / 1000)
    + (double)(end.toMSecsSinceEpoch() % 1000 - start.toMSecsSinceEpoch() % 1000);
  return (res < 0.0 ? 0.0 : res);
}

/**
 *  Check if the result execution time works.
 */
int main() {
  try {
    QDateTime start = QDateTime::currentDateTime();

    QDateTime end = start;
    end.addSecs(10);
    end.addMSecs(20);

    result res1(0, "", "", start, end);
    if (res1.get_execution_time() != execution_time(start, end)) {
      qDebug() << "error: execution_time invalid value.";
      return (1);
    }

    result res2(0, "", "", end, start);
    if (res2.get_execution_time() != 0) {
      qDebug() << "error: execution_time != 0";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }

  return (0);
}
