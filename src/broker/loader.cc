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

#include <cassert>
#include <cstdlib>
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/io/directory_entry.hh"
#include "com/centreon/io/file_stream.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::broker;
using namespace com::centreon::engine::logging;

// Class instance.
static loader* _instance = NULL;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Add a new module.
 *
 *  @param[in] filename The module filename.
 *  @param[in] args     The arguments module.
 *
 *  @return The new object module.
 */
shared_ptr<broker::handle> loader::add_module(
                                     std::string const& filename,
                                     std::string const& args) {
  shared_ptr<handle> module(new handle(filename, args));
  return (_modules.insert(std::make_pair(filename, module))->second);
}

/**
 *  Remove a module.
 *
 *  @param[in] mod Module to remove.
 */
void loader::del_module(shared_ptr<handle> const& module) {
  for (std::multimap<std::string, shared_ptr<handle> >::iterator
         it(_modules.find(module->get_name())), end(_modules.end());
       it != end;
       ++it)
    if (it->second.get() == module.get()) {
      _modules.erase(it);
      break;
    }
  return;
}

/**
 *  Get all modules.
 *
 *  @return All modules in a list.
 */
std::list<shared_ptr<broker::handle> > loader::get_modules() const {
  std::list<shared_ptr<handle> > lst;
  for (std::multimap<std::string, shared_ptr<handle> >::const_iterator
         it(_modules.begin()), end(_modules.end());
       it != end;
       ++it)
    lst.push_back(it->second);
  return (lst);
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
  if (!_instance)
    _instance = new loader;
  return;
}

/**
 *  Load modules in the specify directory.
 *
 *  @param[in] dir Directory to load.
 *
 *  @return Number of modules loaded.
 */
unsigned int loader::load_directory(std::string const& dir) {
  // Get directory entries.
  io::directory_entry directory(dir);
  std::list<io::file_entry> const& files(directory.entry_list("*.so"));

  // Load modules.
  unsigned int loaded(0);
  for (std::list<io::file_entry>::const_iterator
         it(files.begin()), end(files.end());
       it != end;
       ++it) {
    std::string config_file(dir + "/" + it->base_name() + ".cfg");
    if (io::file_stream::exists(config_file.c_str()) == false)
      config_file = "";
    shared_ptr<handle> module;
    try {
      module = add_module(dir + "/" + it->file_name(), config_file);
      module->open();
      logger(log_info_message, basic)
        << "Event broker module '" << it->file_name()
        << "' initialized successfully.";
      ++loaded;
    }
    catch (error const& e) {
      del_module(module);
      logger(log_runtime_error, basic)
        << "Error: Could not load module '"
        << it->file_name() << "' -> " << e.what();
    }
  }
  return (loaded);
}

/**
 *  Cleanup the loader singleton.
 */
void loader::unload() {
  delete _instance;
  _instance = NULL;
  return;
}

/**
 *  Unload all modules.
 */
void loader::unload_modules() {
  for (std::multimap<std::string, shared_ptr<handle> >::iterator
         it(_modules.begin()), end(_modules.end());
       it != end;
       ++it) {
    try {
      it->second->close();
    }
    catch (...) {}
    logger(dbg_eventbroker, basic)
      << "Module '" << it->second->get_filename()
      << "' unloaded successfully.";
  }
  _modules.clear();
  return;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
loader::loader() {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
loader::loader(loader const& right) {
  _internal_copy(right);
}

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
  return;
}
