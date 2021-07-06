/*
** Copyright 2011-2013 Merethis
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

#include "com/centreon/engine/broker/loader.hh"
#include <cassert>
#include <cstdlib>
#include <map>
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/io/directory_entry.hh"
#include "com/centreon/io/file_stream.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::exceptions;
using namespace com::centreon::engine::broker;
using namespace com::centreon::engine::logging;

loader* loader::_instance = nullptr;

/**
 *  Add a new module.
 *
 *  @param[in] filename The module filename.
 *  @param[in] args     The arguments module.
 *
 *  @return The new object module.
 */
broker::handle* loader::add_module(const std::string& filename,
                                                   const std::string& args) {
  _modules.emplace_back(std::make_unique<handle>(filename, args));
  return _modules.back().get();
}

/**
 *  Remove a module.
 *
 *  @param[in] mod Module to remove.
 */
void loader::del_module(handle* module) {
  for (auto it = _modules.begin(); it != _modules.end(); ++it)
    if (it->get() == module) {
      _modules.erase(it);
      break;
    }
}

/**
 *  Get all modules.
 *
 *  @return All modules in a list.
 */
const std::list<std::unique_ptr<broker::handle>>& loader::get_modules() const {
  return _modules;
}

/**
 *  Get instance of loader singleton.
 *
 *  @return Class instance.
 */
loader& loader::instance() {
  assert(_instance);
  return *_instance;
}

void loader::load() {
  if (_instance == nullptr)
    _instance = new loader();
}

void loader::unload() {
  if (_instance) {
    delete _instance;
    _instance = nullptr;
  }
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

  // Sort by file name.
  std::multimap<std::string, io::file_entry> sort_files;
  for (std::list<io::file_entry>::const_iterator it(files.begin()),
       end(files.end());
       it != end; ++it)
    sort_files.insert(std::make_pair(it->file_name(), *it));

  // Load modules.
  unsigned int loaded(0);
  for (std::multimap<std::string, io::file_entry>::const_iterator
           it(sort_files.begin()),
       end(sort_files.end());
       it != end; ++it) {
    io::file_entry const& f(it->second);
    std::string config_file(dir + "/" + f.base_name() + ".cfg");
    if (io::file_stream::exists(config_file.c_str()) == false)
      config_file = "";
    handle* module;
    try {
      module = add_module(dir + "/" + f.file_name(), config_file);
      module->open();
      logger(log_info_message, basic)
          << "Event broker module '" << f.file_name()
          << "' initialized successfully.";
      ++loaded;
    } catch (error const& e) {
      del_module(module);
      logger(log_runtime_error, basic) << "Error: Could not load module '"
                                       << f.file_name() << "' -> " << e.what();
    }
  }
  return loaded;
}

/**
 *  Unload all modules.
 */
void loader::unload_modules() {
  for (auto& m : _modules) {
    try {
      m->close();
    } catch (...) {
    }
  logger(log_info_message, basic)
      << "Event broker module '" << m->get_filename()
      << "' deinitialized successfully";

    logger(dbg_eventbroker, basic)
        << "Module '" << m->get_filename() << "' unloaded successfully.";
  }
  _modules.clear();
}

/**
 *  Default destructor.
 */
loader::~loader() noexcept {
  try {
    unload_modules();
  } catch (...) {
  }
}
