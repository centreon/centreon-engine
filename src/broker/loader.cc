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
#include <unistd.h>

#include "error.hh"
#include "logging.hh"
#include "broker/compatibility.hh"
#include "broker/loader.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::broker;

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
  QSharedPointer<handle> module;

  for (QFileInfoList::const_iterator it = files.begin(), end = files.end(); it != end; ++it) {
    QString config_file(dir.path() + "/" + it->baseName() + ".cfg");
    if (dir.exists(config_file) == false)
      config_file = "";
    try {
      module = add_module(dir.path() + "/" + it->fileName(), config_file);
      module->open();
      logit(NSLOG_INFO_MESSAGE, false,
	    "Event broker module `%s' initialized syccessfully.\n",
	    it->fileName().toStdString().c_str());
    }
    catch (error const& e) {
      del_module(module);
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
  for (QMultiHash<QString, QSharedPointer<handle> >::iterator
	 it = _modules.begin(), end = _modules.end();
       it != end;
       ++it) {
    it.value()->close();
    log_debug_info(DEBUGL_EVENTBROKER, 0,
		   "Module `%s' unloaded successfully.\n",
		   it.value()->get_filename().toStdString().c_str());
  }
  _modules.clear();
}

/**
 *  Add a new module.
 *
 *  @param[in] filename The module filename.
 *  @param[in] args     The arguments module.
 *
 *  @return The new object module.
 */
QSharedPointer<handle> loader::add_module(QString const& filename,
					  QString const& args) {
  QSharedPointer<handle> module(new handle(filename, args));

  connect(&(*module), SIGNAL(name_changed(QString const&, QString const&)),
  	  this, SLOT(module_name_changed(QString const&, QString const&)));

  broker::compatibility& compatibility = broker::compatibility::instance();
  connect(&(*module), SIGNAL(event_create(broker::handle*)),
  	  &compatibility, SLOT(create_module(broker::handle*)));
  connect(&(*module), SIGNAL(event_destroy(broker::handle*)),
  	  &compatibility, SLOT(destroy_module(broker::handle*)));
  connect(&(*module), SIGNAL(event_name(broker::handle*)),
  	  &compatibility, SLOT(name_module(broker::handle*)));
  connect(&(*module), SIGNAL(event_author(broker::handle*)),
  	  &compatibility, SLOT(author_module(broker::handle*)));
  connect(&(*module), SIGNAL(event_copyright(broker::handle*)),
  	  &compatibility, SLOT(copyright_module(broker::handle*)));
  connect(&(*module), SIGNAL(event_version(broker::handle*)),
  	  &compatibility, SLOT(version_module(broker::handle*)));
  connect(&(*module), SIGNAL(event_license(broker::handle*)),
  	  &compatibility, SLOT(license_module(broker::handle*)));
  connect(&(*module), SIGNAL(event_description(broker::handle*)),
  	  &compatibility, SLOT(description_module(broker::handle*)));
  connect(&(*module), SIGNAL(event_loaded(broker::handle*)),
  	  &compatibility, SLOT(loaded_module(broker::handle*)));
  connect(&(*module), SIGNAL(event_unloaded(broker::handle*)),
  	  &compatibility, SLOT(unloaded_module(broker::handle*)));

  return (_modules.insert(filename, module).value());
}

/**
 *  Remove a module.
 *
 *  @param[in] module Modile to remove.
 */
void loader::del_module(QSharedPointer<handle> const& module) {
  _modules.remove(module->get_name(), module);
}

/**
 *  Get the directory.
 *
 *  @return The directory.
 */
QString const& loader::get_directory() const throw() {
  return (_directory);
}

/**
 *  Get all modules.
 *
 *  @return All modules in a const map.
 */
QList<QSharedPointer<handle> > loader::get_modules() const throw() {
  return (_modules.values());
}

/**
 *  Set the directory.
 *
 *  @param[in] directory The Directory path content modules.
 */
void loader::set_directory(QString const& directory) {
  _directory = directory;
}

/**
 *  Slot for notify when module name changed.
 *
 *  @param[in] old_name The old name of the module.
 *  @param[in] new_name The new name of the module.
 */
void loader::module_name_changed(QString const& old_name,
				 QString const& new_name) {
  for (QMultiHash<QString, QSharedPointer<handle> >::iterator
	 it = _modules.find(old_name), end = _modules.end();
       it != end && it.key() == old_name;
       ++it) {
    if (it.value() == this->sender()) {
      QSharedPointer<handle> module = it.value();
      _modules.insert(new_name, module);
      _modules.remove(old_name, module);
      return;
    }
  }
  throw (engine_error() << "Module `" << old_name << "' not found");
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
