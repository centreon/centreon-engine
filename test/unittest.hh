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

#ifndef TEST_UNITTEST_HH
#  define TEST_UNITTEST_HH

#  include <iostream>
#  include "com/centreon/engine/broker/compatibility.hh"
#  include "com/centreon/engine/broker/loader.hh"
#  include "com/centreon/engine/checks/checker.hh"
#  include "com/centreon/engine/commands/set.hh"
#  include "com/centreon/engine/events/loop.hh"
#  include "com/centreon/engine/logging/engine.hh"

namespace     com {
  namespace   centreon {
    namespace engine {
      /**
       *  @class unittest unittest.hh
       *  @brief Class unittest init and destroy all
       *  engine needs to make unit test and run
       *  unit test.
       */
      class    unittest {
      public:
               unittest(int argc, char** argv, int (*func)(int, char**))
                 : _argc(argc), _argv(argv), _func(func) {}
               ~unittest() throw () {}
        int    run() {
          int ret(1);
          try {
            _init();
            ret = (*_func)(_argc, _argv);
            _deinit();
          }
          catch (std::exception const& e) {
            std::cerr << "error: " << e.what() << std::endl;
          }
          catch (...) {
            std::cerr << "error: catch all..." << std::endl;
          }
          return (ret);
        }

      private:
        void   _init() {
          logging::engine::load();
          commands::set::load();
          checks::checker::load();
          events::loop::load();
          broker::loader::load();
          broker::compatibility::load();
        }

        void   _deinit() {
          broker::compatibility::unload();
          broker::loader::unload();
          events::loop::unload();
          checks::checker::unload();
          commands::set::unload();
          logging::engine::unload();
        }

        int    _argc;
        char** _argv;
        int    (*_func)(int, char**);
      };
    }
  }
}

#endif // !TEST_UNITTEST_HH
