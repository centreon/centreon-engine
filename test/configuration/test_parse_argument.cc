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
#include <QFile>
#include <exception>

#include "configuration/state.hh"

using namespace com::centreon::engine;

/**
 *  Check parse with directory.
 */
static void check_directory() {
  try {
    configuration::state config;
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
    configuration::state config;
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
  QString filename("./test_exist_file.cfg");
  QFile file(filename);
  file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered);
  if (file.error() != QFile::NoError) {
    throw (engine_error() << filename << ": " << file.errorString());
  }
  file.close();
  configuration::state config;
  config.parse(filename);
}

/**
 *  Check the parsing argument.
 */
int main(void) {
  try {
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
