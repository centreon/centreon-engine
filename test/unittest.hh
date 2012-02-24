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

#  include <QDebug>
#  include <QThread>
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
      class   unittest : public QThread {
      public:
              unittest(int (*func)())
          : QThread(), _func(func), _ret(1) {}

              ~unittest() throw () {
          wait();
        }

        int   ret() const throw () {
          return (_ret);
        }

      protected:
        void  run() {
          try {
            _init();
            _ret = (*_func)();
            _deinit();
          }
          catch (std::exception const& e) {
            qDebug() << "error: " << e.what();
          }
          catch (...) {
            qDebug() << "error: catch all...";
          }
        }

      private:
        void  _init() {
          logging::engine::load();
          commands::set::instance();
          checks::checker::instance();
          broker::loader::instance();
          broker::compatibility::instance();
          events::loop::instance();
        }

        void  _deinit() {
          events::loop::cleanup();
          broker::compatibility::cleanup();
          broker::loader::cleanup();
          checks::checker::cleanup();
          commands::set::cleanup();
          logging::engine::unload();
        }

        int  (*_func)();
        int   _ret;
      };
    }
  }
}

#endif // !TEST_UNITTEST_HH
