/*
** Copyright 2011      Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <QDir>
#include <QFile>
#include "error.hh"
#include "logging.hh"
#include "modules/loader.hh"

using namespace com::centreon::scheduler;
using namespace com::centreon::scheduler::modules;

loader& loader::instance() {
  static loader instance;
  return (instance);
}

void loader::load() {
  QDir dir(_directory);
  QStringList filters("*.so");
  QFileInfoList files = dir.entryInfoList(filters);

  for (QFileInfoList::const_iterator it = files.begin(), end = files.end(); it != end; ++it) {
    QString config_file(it->baseName() + ".cfg");
    if (dir.exists(config_file) == false)
      config_file = "";

    try {
      handle module(it->fileName(), config_file);
      module.open();
      add_module(module);
      logit(NSLOG_INFO_MESSAGE, false,
	    "Event broker module `%s' initialized syccessfully.\n",
	    it->fileName().toStdString().c_str());
    }
    catch (error const& e) {
      logit(NSLOG_RUNTIME_ERROR, false,
	    "Error: Could not load module `%s' -> %s\n",
	    it->fileName().toStdString().c_str(),
	    e.what());
    }
  }
}

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

void loader::add_module(handle const& module) {
  _modules.insert(module.get_filename(), module);
}

void loader::del_module(handle const& module) {
  _modules.remove(module.get_filename(), module);
}

QString const& loader::get_directory() const throw() {
  return (_directory);
}

QMultiHash<QString, handle> const& loader::get_modules() const throw() {
  return (_modules);
}

QMultiHash<QString, handle>& loader::get_modules() throw() {
  return (_modules);
}

void loader::set_directory(QString const& directory) {
  _directory = directory;
}

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
  throw (error() << "Module `" << filename << ":" << old_name << "' not found");
}

loader::loader() : QObject(0) {}

loader::~loader() throw() {}
