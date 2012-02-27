/*
** Copyright 2011-2012 Merethis
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

#include <assert.h>
#include <QDir>
#include <QFile>
#include <stdlib.h>
#include "com/centreon/engine/broker/compatibility.hh"
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::broker;
using namespace com::centreon::engine::logging;

// Class instance.
std::auto_ptr<loader> loader::_instance;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default destructor.
 */
loader::~loader() throw () {
  try {
    unload_modules();
  }
  catch (...) {}
}

/**
 *  Add a new module.
 *
 *  @param[in] filename The module filename.
 *  @param[in] args     The arguments module.
 *
 *  @return The new object module.
 */
QSharedPointer<handle> loader::add_module(
                                 QString const& filename,
                                 QString const& args) {
  QSharedPointer<handle> module(new handle(filename, args));
  broker::compatibility& compatibility(broker::compatibility::instance());

  if (connect(&(*module),
              SIGNAL(name_changed(QString const&, QString const&)),
              this,
              SLOT(module_name_changed(QString const&, QString const&))) == false
      || connect(&(*module),
                 SIGNAL(event_create(broker::handle*)),
                 &compatibility,
                 SLOT(create_module(broker::handle*))) == false
      || connect(&(*module),
                 SIGNAL(event_destroy(broker::handle*)),
                 &compatibility,
                 SLOT(destroy_module(broker::handle*))) == false
      || connect(&(*module),
                 SIGNAL(event_name(broker::handle*)),
                 &compatibility,
                 SLOT(name_module(broker::handle*))) == false
      || connect(&(*module),
                 SIGNAL(event_author(broker::handle*)),
                 &compatibility,
                 SLOT(author_module(broker::handle*))) == false
      || connect(&(*module),
                 SIGNAL(event_copyright(broker::handle*)),
                 &compatibility,
                 SLOT(copyright_module(broker::handle*))) == false
      || connect(&(*module),
                 SIGNAL(event_version(broker::handle*)),
                 &compatibility,
                 SLOT(version_module(broker::handle*))) == false
      || connect(&(*module),
                 SIGNAL(event_license(broker::handle*)),
                 &compatibility,
                 SLOT(license_module(broker::handle*))) == false
      || connect(&(*module),
                 SIGNAL(event_description(broker::handle*)),
                 &compatibility,
                 SLOT(description_module(broker::handle*))) == false
      || connect(&(*module),
                 SIGNAL(event_loaded(broker::handle*)),
                 &compatibility,
                 SLOT(loaded_module(broker::handle*))) == false
      || connect(&(*module),
                 SIGNAL(event_unloaded(broker::handle*)),
                 &compatibility,
                 SLOT(unloaded_module(broker::handle*))) == false) {
    throw (engine_error() << "connect module to broker::compatibility failed.");
  }
  return (_modules.insert(filename, module).value());
}

/**
 *  Remove a module.
 *
 *  @param[in] mod Module to remove.
 */
void loader::del_module(QSharedPointer<handle> const& mod) {
  _modules.remove(mod->get_name(), mod);
  return ;
}

/**
 *  Get all modules.
 *
 *  @return All modules in a list.
 */
QList<QSharedPointer<handle> > loader::get_modules() const {
  return (_modules.values());
}

/**
 *  Get instance of loader singleton.
 *
 *  @return Class instance.
 */
loader& loader::instance() {
  return (*_instance);
}

/**
 *  Load class singleton.
 */
void loader::load() {
  if (!_instance.get())
    _instance.reset(new loader);
  return ;
}

/**
 *  Load modules in the specify directory.
 *
 *  @param[in] dir Directory to load.
 *
 *  @return Number of modules loaded.
 */
unsigned int loader::load_directory(QString const& dir) {
  // Get directory entries.
  QDir directory(dir);
  QStringList filters("*.so");
  QFileInfoList files(directory.entryInfoList(filters));

  // Load modules.
  unsigned int loaded(0);
  for (QFileInfoList::const_iterator
         it = files.begin(),
         end = files.end();
       it != end;
       ++it) {
    QString config_file(dir + "/" + it->baseName() + ".cfg");
    if (directory.exists(config_file) == false)
      config_file = "";
    QSharedPointer<handle> module;
    try {
      module = add_module(dir + "/" + it->fileName(), config_file);
      module->open();
      logger(log_info_message, basic)
        << "Event broker module '" << it->fileName()
        << "' initialized successfully.";
      ++loaded;
    }
    catch (error const& e) {
      del_module(module);
      logger(log_runtime_error, basic)
        << "Error: Could not load module '" << it->fileName()
        << "' -> " << e.what();
    }
  }
  return (loaded);
}

/**
 *  Cleanup the loader singleton.
 */
void loader::unload() {
  _instance.reset();
  return ;
}

/**
 *  Unload all modules.
 */
void loader::unload_modules() {
  for (QMultiHash<QString, QSharedPointer<handle> >::iterator
         it = _modules.begin(), end = _modules.end();
       it != end;
       ++it) {
    try {
      it.value()->close();
    }
    catch (...) {}
    logger(dbg_eventbroker, basic)
      << "Module '" << it.value()->get_filename()
      << "' unloaded successfully.";
  }
  _modules.clear();
  return ;
}

/**
 *  Slot for notify when module name changed.
 *
 *  @param[in] old_name The old name of the module.
 *  @param[in] new_name The new name of the module.
 */
void loader::module_name_changed(
               QString const& old_name,
               QString const& new_name) {
  for (QMultiHash<QString, QSharedPointer<handle> >::iterator
         it = _modules.find(old_name), end = _modules.end();
       it != end && it.key() == old_name;
       ++it) {
    if (it.value() == this->sender()) {
      QSharedPointer<handle> module = it.value();
      _modules.insert(new_name, module);
      _modules.remove(old_name, module);
      return ;
    }
  }
  throw (engine_error() << "Module '" << old_name << "' not found");
  return ;
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
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
loader::loader(loader const& right) : QObject(0) {
  _internal_copy(right);
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
loader& loader::operator=(loader const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void loader::_internal_copy(loader const& right) {
  (void)right;
  assert(!"broker module loader is not copyable");
  abort();
  return ;
}
