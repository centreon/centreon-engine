/*
** Copyright 2011 Merethis
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

#include <QCoreApplication>
#include <QDebug>
#include <exception>
#include "commands/connector/execute_query.hh"

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
int main(int argc, char** argv) {
  try {
    QCoreApplication app(argc, argv);

    for (unsigned int i = 0; lines[i].command != NULL; ++i) {
      execute_query query(0, lines[i].command, QDateTime::currentDateTime(), 0);
      QStringList list = query.get_args();

      QString result;
      for (QStringList::const_iterator it = list.begin(), end = list.end();
	   it != end;
	   ++it) {
	result += *it;
	if (it + 1 != end) {
	  result += ' ';
	}
      }
      if (result != lines[i].result) {
	qDebug() << "error: command '" << lines[i].command << "' failed.";
	return (1);
      }
    }

  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}
