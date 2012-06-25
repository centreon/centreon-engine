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

#include <exception>
#include <QCoreApplication>
#include "com/centreon/engine/commands/connector/execute_query.hh"
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands::connector;

struct line {
  char const* command;
  char const* result;
};

line lines[] = {
  { "", "" },
  { "./bin", "./bin" },
  { "  ./bin   arg1  arg2    arg3  arg4", "./bin arg1 arg2 arg3 arg4" },
  { "./bin  arg1   'arg2'   ' arg3 '", "./bin arg1 arg2  arg3 " },
  { "  ./bin ", "./bin" },
  { " ./bin arg1 \"arg2\" \" arg3 \"", "./bin arg1 arg2  arg3 " },
  { "./bin 'mix \"arg1\"'", "./bin mix \"arg1\"" },
  { "./bin 'multi \\'level\\'", "./bin multi 'level'" },
  { "\t./bin \t \t \t \t arg1", "./bin arg1" },
  { NULL, NULL }
};

/**
 *  Check the execute request.
 */
int main_test() {
  for (unsigned int i(0); lines[i].command != NULL; ++i) {
    execute_query query(0, lines[i].command, QDateTime::currentDateTime(), 0);
    std::list<std::string> list(query.get_args());
    std::string result;
    for (std::list<std::string>::const_iterator
           it(list.begin()),
           end(list.end());
         it != end;
         ++it) {
      result += *it;
      std::list<std::string>::const_iterator next(it);
      ++next;
      if (next != end)
        result += ' ';
    }
    if (result != lines[i].result)
      throw (engine_error() << "error: command '"
             << lines[i].command << "' failed");
  }

  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&main_test);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}
