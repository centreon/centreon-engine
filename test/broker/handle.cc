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

#include "error.hh"
#include "broker/handle.hh"
#include "mod_load.hh"

using namespace com::centreon::engine::broker;

static const char* MOD_LIB_UNKNOW  = "./broker_mod_unknow.so";
static const char* MOD_LIB_NAME    = "./broker_mod_load.so";

bool mod_test_load_quit = false;

/**
 *  Check the open mothod with unknow module.
 */
void check_open_noexist_module() {
  try {
    handle hwd;
    hwd.open(MOD_LIB_UNKNOW, MOD_LIB_UNKNOW);
  }
  catch (std::exception const& e) {
    (void)e;
    return;
  }
  throw (engine_error() << __func__ << ": try to open noexist module.");
}

/**
 *  Check the open mothod with existing module.
 */
void check_open_exist_module() {
  handle hwd;
  hwd.open(MOD_LIB_NAME, MOD_LIB_NAME);
  if (hwd.is_loaded() == false) {
    throw (engine_error() << __func__ << ": open module failed.");
  }
  if (hwd.get_handle() == NULL) {
    throw (engine_error() << __func__ << ": invalide handle.");
  }
  if (hwd.get_author() != MOD_AUTHOR) {
    throw (engine_error() << __func__ << ": invalide author.");
  }
  if (hwd.get_copyright() != MOD_COPYRIGHT) {
    throw (engine_error() << __func__ << ": invalide copyright.");
  }
  if (hwd.get_description() != MOD_DESCRIPTION) {
    throw (engine_error() << __func__ << ": invalide description.");
  }
  if (hwd.get_name() != MOD_TITLE) {
    throw (engine_error() << __func__ << ": invalide name.");
  }
  if (hwd.get_version() != MOD_VERSION) {
    throw (engine_error() << __func__ << ": invalide version.");
  }
  if (hwd.get_license() != MOD_LICENSE) {
    throw (engine_error() << __func__ << ": invalide license.");
  }
  if (hwd.get_args() != MOD_LIB_NAME) {
    throw (engine_error() << __func__ << ": invalide args.");
  }

  hwd.close();
  if (mod_test_load_quit == false) {
    throw (engine_error() << __func__ << ": close failed.");
  }
}

/**
 *  Check the default copy constructor.
 */
void check_copy() {
  handle hwd(MOD_LIB_NAME, MOD_LIB_NAME);
  hwd.open();
  handle hwd_cpy(hwd);

  if (hwd != hwd_cpy || !(hwd == hwd_cpy)) {
    throw (engine_error() << __func__ << ": hwd and hwd_cpy are differente.");
  }

  hwd_cpy.close();
  if (mod_test_load_quit == false) {
    throw (engine_error() << __func__ << ": close failed.");
  }

  if (hwd.is_loaded() == true) {
    throw (engine_error() << __func__ << ": is_loaded is true.");
  }
}

/**
 *  Check the broker handle working.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    check_open_noexist_module();
    check_open_exist_module();
    check_copy();
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
