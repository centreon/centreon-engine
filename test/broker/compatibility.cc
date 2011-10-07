/*
** Copyright 2011      Merethis
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
#include <limits.h>

#include "error.hh"
#include "mod_load.hh"
#include "broker/compatibility.hh"
#include "broker/loader.hh"

using namespace com::centreon::engine::broker;

static const char* MOD_LIB_COMPT_NAME = "./broker_mod_compatibility.so";

extern nebmodule* neb_module_list;

/**
 *  Check broker compatibility.
 */
void check_compatibility() {
  loader& loader = loader::instance();

  loader.add_module(MOD_LIB_COMPT_NAME, MOD_LIB_COMPT_NAME)->open();

  if (loader.get_modules().size() != 1) {
    throw (engine_error() << __func__ << ": broker loader failed.");
  }

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle != NULL) {

      if (strcmp(tmp->info[NEBMODULE_MODINFO_TITLE], MOD_TITLE)) {
	throw (engine_error() << __func__ << ": invalid name.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_AUTHOR], MOD_AUTHOR)) {
	throw (engine_error() << __func__ << ": invalide author.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_COPYRIGHT], MOD_COPYRIGHT)) {
	throw (engine_error() << __func__ << ": invalide copyright.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_DESC], MOD_DESCRIPTION)) {
	throw (engine_error() << __func__ << ": invalide description.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_VERSION], MOD_VERSION)) {
	throw (engine_error() << __func__ << ": invalide version.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_LICENSE], MOD_LICENSE)) {
	throw (engine_error() << __func__ << ": invalide license.");
      }
    }
  }

  loader.unload();
}

/**
 *  Check the broker compatibility working.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    check_compatibility();
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
