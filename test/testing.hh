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

#ifndef TEST_TESTING_HH
# define TEST_TESTING_HH

# include "broker/compatibility.hh"
# include "broker/loader.hh"
# include "checks/checker.hh"
# include "commands/set.hh"
# include "logging/engine.hh"
# include "events/loop.hh"

namespace com {
  namespace centreon {
    namespace engine {
      /**
       *  @class testing testing.hh
       *  @brief Class testing init and destroy all
       *  engine needs to make unit test.
       */
      class testing {
      public:
        testing() {
          logging::engine::instance();
          commands::set::instance();
          checks::checker::instance();
          broker::loader::instance();
          broker::compatibility::instance();
          events::loop::instance();
        }

        ~testing() throw() {
          events::loop::cleanup();
          broker::compatibility::cleanup();
          broker::loader::cleanup();
          checks::checker::cleanup();
          commands::set::cleanup();
          logging::engine::cleanup();
        }
      };
    }
  }
}

#endif // !TEST_TESTING_HH
