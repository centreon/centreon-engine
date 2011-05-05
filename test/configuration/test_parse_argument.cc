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
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <exception>

#include "globals.hh"
#include "configuration/state.hh"

using namespace com::centreon::engine;

/**
 *  Check parse with directory.
 */
static void check_directory() {
  try {
    config.parse("./");
  }
  catch (std::exception const& e) {
    (void)e;
    return;
  }
  throw (engine_error() << "try to parse directory.");
}

/**
 *  Check pase with noexist file.
 */
static void check_noexist_file() {
  try {
    config.parse("./test_noexist_file.cfg");
  }
  catch (std::exception const& e) {
    (void)e;
    return;
  }
  throw (engine_error() << "try to parse noexisting file.");
}

/**
 *  Check parse with exist file.
 */
static void check_exist_file() {
  QTemporaryFile tmp("./centengine_test_exist_file.cfg");
  if (tmp.open() == false) {
    throw (engine_error() << "open temporary file failed.");
  }
  config.parse(tmp.fileName());
  tmp.close();
}

/**
 *  Check the parsing argument.
 */
int main(void) {
  try {
    config.set_check_result_path(QDir::tempPath());
    config.set_log_archive_path(QDir::tempPath());
    config.set_temp_path(QDir::tempPath());

    check_directory();
    check_noexist_file();
    check_exist_file();
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
