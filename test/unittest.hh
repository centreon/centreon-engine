/*
** Copyright 2011-2013,2015 Merethis
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

#ifndef TEST_UNITTEST_HH
#define TEST_UNITTEST_HH

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "com/centreon/clib.hh"
#include "com/centreon/engine/broker/compatibility.hh"
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/events/loop.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "com/centreon/logging/backend.hh"
#include "com/centreon/logging/engine.hh"
#include "com/centreon/logging/file.hh"

CCE_BEGIN()

/**
 *  @class unittest unittest.hh
 *  @brief Class unittest init and destroy all
 *  engine needs to make unit test and run
 *  unit test.
 */
class unittest {
 public:
  /**
   *  Constructor.
   *
   *  @param[in] argc Argument count.
   *  @param[in] argv Argument values.
   *  @param[in] func Unit test routine.
   *
   *  @return Return value of func.
   */
  unittest(int argc, char** argv, int (*func)(int, char**))
      : _argc(argc), _argv(argv), _func(func), _log(stdout) {}

  /**
   *  Destructor.
   */
  ~unittest() throw() {}

  /**
   *  Entry point.
   *
   *  @return Return value of unit test routine.
   */
  int run() {
    int ret(EXIT_FAILURE);
    if (!_init())
      return (ret);
    try {
      ret = (*_func)(_argc, _argv);
    } catch (std::exception const& e) {
      std::cerr << "error: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "error: unknown exception" << std::endl;
    }
    if (!_deinit())
      ret = EXIT_FAILURE;
    return (ret);
  }

 private:
  class nothing : public com::centreon::logging::backend {
   public:
    nothing(FILE* file = NULL) { (void)file; }
    nothing(nothing const& right) : com::centreon::logging::backend(right) {
      (void)right;
    }
    ~nothing() throw() {}
    nothing& operator=(nothing const& right) {
      (void)right;
      return (*this);
    }
    void close() throw() {}
    void log(uint64_t types,
             uint32_t verbose,
             char const* msg,
             uint32_t size) throw() {
      (void)types;
      (void)verbose;
      (void)msg;
      (void)size;
    }
    void open() {}
    void reopen() {}
  };

  bool _init() {
    try {
      com::centreon::clib::load();
      com::centreon::logging::engine::instance().add(
          &_log, com::centreon::engine::logging::log_all,
          com::centreon::engine::logging::most);
      config = new configuration::state;
      timezone_manager::load();
      commands::set::load();
      configuration::applier::state::load();
      checks::checker::load();
      events::loop::load();
      broker::loader::load();
      broker::compatibility::load();
    } catch (std::exception const& e) {
      std::cerr << "unit test init failed: " << e.what() << std::endl;
      return (false);
    } catch (...) {
      std::cerr << "unit test init failed: "
                   "unknown exception"
                << std::endl;
      return (false);
    }
    return (true);
  }

  bool _deinit() {
    try {
      broker::compatibility::unload();
      broker::loader::unload();
      events::loop::unload();
      checks::checker::unload();
      configuration::applier::state::unload();
      commands::set::unload();
      delete config;
      config = NULL;
      com::centreon::clib::unload();
    } catch (std::exception const& e) {
      std::cerr << "unit test deinit failed: " << e.what() << std::endl;
      return (false);
    } catch (...) {
      std::cerr << "unit test deinit failed: "
                   "unknown exception"
                << std::endl;
      return (false);
    }
    return (true);
  }

  int _argc;
  char** _argv;
  int (*_func)(int, char**);
#ifdef NDEBUG
  nothing _log;
#else
  com::centreon::logging::file _log;
#endif  // !NDEBUG
};

CCE_END()

#endif  // !TEST_UNITTEST_HH
