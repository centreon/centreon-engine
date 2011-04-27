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

#include <QDir>
#include <QFile>
#include "error.hh"
#include "logging.hh"
#include "modules/loader.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::modules;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Get instance of loader singleton.
 */
loader& loader::instance() {
  static loader instance;
  return (instance);
}

/**
 *  Load modules in the specify directory.
 */
void loader::load() {
  QDir dir(_directory);
  QStringList filters("*.so");
  QFileInfoList files = dir.entryInfoList(filters);
  QHash<QString, handle>::iterator it_module;

  for (QFileInfoList::const_iterator it = files.begin(), end = files.end(); it != end; ++it) {
    QString config_file(it->baseName() + ".cfg");
    if (dir.exists(config_file) == false)
      config_file = "";
    try {
      handle module(it->fileName(), config_file);
      it_module = _modules.insert(module.get_filename(), module);
      it_module->open();
      logit(NSLOG_INFO_MESSAGE, false,
	    "Event broker module `%s' initialized syccessfully.\n",
	    it->fileName().toStdString().c_str());
    }
    catch (error const& e) {
      _modules.erase(it_module);
      logit(NSLOG_RUNTIME_ERROR, false,
	    "Error: Could not load module `%s' -> %s\n",
	    it->fileName().toStdString().c_str(),
	    e.what());
    }
  }
}

/**
 *  Unload all modules.
 */
void loader::unload() {
  for (QMultiHash<QString, handle>::iterator it = _modules.begin(), end = _modules.end();
       it != end;
       ++it) {
    it.value().close();
    log_debug_info(DEBUGL_EVENTBROKER, 0,
		   "Module `%s' unloaded successfully.\n",
		   it.value().get_filename().toStdString().c_str());
  }
  _modules.clear();
}

/**
 *  Add a new module.
 *  @param[in] module Module to add.
 */
void loader::add_module(handle const& module) {
  _modules.insert(module.get_filename(), module);
}

/**
 *  Remove a module.
 *  @param[in] module Modile to remove.
 */
void loader::del_module(handle const& module) {
  _modules.remove(module.get_filename(), module);
}

/**
 *  Get the directory.
 *  @return The directory.
 */
QString const& loader::get_directory() const throw() {
  return (_directory);
}

/**
 *  Get all modules.
 *  @return All modules in a const map.
 */
QMultiHash<QString, handle> const& loader::get_modules() const throw() {
  return (_modules);
}

/**
 *  Get all modules.
 *  @return All modules in a map.
 */
QMultiHash<QString, handle>& loader::get_modules() throw() {
  return (_modules);
}

/**
 *  Set the directory.
 *  @param[in] directory The Directory path content modules.
 */
void loader::set_directory(QString const& directory) {
  _directory = directory;
}

/**
 *  Slot for notify when module name changed.
 *  @param[in] filename The filename of the module.
 *  @param[in] old_name The old name of the module.
 *  @param[in] new_name The new name of the module.
 */
void loader::module_name_changed(QString const& filename,
				 QString const& old_name,
				 QString const& new_name) {
  for (QHash<QString, handle>::iterator it = _modules.find(old_name), end = _modules.end();
       it != end;
       ++it) {
    if (it->get_filename() == filename) {
      handle module(*it);
      _modules.insert(new_name, module);
      _modules.remove(old_name, module);
      return;
    }
  }
  throw (engine_error() << "Module `" << filename << ":" << old_name << "' not found");
}

/**************************************
 *                                     *
 *           Private Methods           *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 */
loader::loader() : QObject(0) {}

/**
 *  Default destructor.
 */
loader::~loader() throw() {}
