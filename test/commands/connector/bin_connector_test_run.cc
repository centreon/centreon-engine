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

#include <QCoreApplication>
#include <QList>
#include <QSharedPointer>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <stdlib.h>
#include "test/unittest.hh"
#include "engine.hh"
#include "error.hh"
#include "com/centreon/engine/version.hh"
#include "commands/connector/quit_response.hh"
#include "commands/connector/version_response.hh"
#include "commands/connector/execute_response.hh"
#include "commands/connector/execute_query.hh"
#include "commands/connector/request_builder.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Wait and get request.
 *
 *  @return The request was send by engine.
 */
static QSharedPointer<connector::request> wait() {
  static connector::request_builder& req_builder = connector::request_builder::instance();
  static QList<QSharedPointer<connector::request> > requests;
  static QByteArray data;

  while (requests.size() == 0) {
    char buf[4096];
    int ret = read(0, buf, sizeof(buf) - 1);
    data.append(buf, ret);

    while (data.size() > 0) {
      int pos = data.indexOf(connector::request::cmd_ending());
      if (pos < 0) {
	break;
      }

      QByteArray req_data = data.left(pos);
      data.remove(0, pos + connector::request::cmd_ending().size());

      try {
	requests.push_back(req_builder.build(req_data));
      }
      catch (std::exception const& e) {
	(void)e;
      }
    }
  }

  QSharedPointer<connector::request> req = requests.front();
  requests.pop_front();
  return (req);
}

/**
 *  Simulate a execution process.
 *
 *  @param[in] argv      The command arguments.
 *  @param[in] timeout   The command timeout.
 *  @param[in] exit_code The command exit code.
 *
 *  @return The raw query.
 */
static QString execute_process(QStringList const& argv, unsigned int timeout, int* exit_code) {
  QString output;

  for (QStringList::const_iterator it = argv.begin(), end = argv.end();
       it != end;
       ++it) {
    output += *it;
    if (it + 1 != end) {
      output += " ";
    }
  }

  if (argv.size() != 2) {
    *exit_code = STATE_WARNING;
    return (output);
  }

  if (argv[1] == "--timeout=on") {
    sleep(timeout  + 1);
  }
  else if (argv[1] == "--timeout=off") {
    *exit_code = STATE_OK;
  }
  else if (argv[1].indexOf("--kill=") == 0) {
    QString value = argv[1].right(argv[1].size() - 7);
    uint time = value.toUInt();
    uint now = QDateTime::currentDateTime().toTime_t();

    if (now < time + 1) {
      sleep(1);
      char* ptr = NULL;
      ptr[0] = 0;
    }
    else {
      *exit_code = STATE_OK;
    }
  }
  else {
    *exit_code = STATE_UNKNOWN;
  }
  return (output);
}

/**
 *  Simulate some behavior of connector.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    while (true) {
      QSharedPointer<connector::request> req = wait();

      switch (req->get_id()) {
      case connector::request::version_q: {
	connector::version_response version(
                                      CENTREON_ENGINE_VERSION_MAJOR,
                                      CENTREON_ENGINE_VERSION_MINOR);
	QByteArray data = version.build();
	if (write(1, data.constData(), data.size()) != data.size())
          throw (engine_error() << "write query version failed.");
	break;
      }

      case connector::request::execute_q: {
	connector::execute_query const* exec_query = static_cast<connector::execute_query const*>(&(*req));
	int exit_code = STATE_OK;
	QString output = execute_process(exec_query->get_args(),
					 exec_query->get_timeout(),
					 &exit_code);
	connector::execute_response execute(exec_query->get_command_id(),
					    true,
					    exit_code,
					    QDateTime::currentDateTime(),
					    "",
					    output);
	QByteArray data = execute.build();
	if (write(1, data.constData(), data.size()) != data.size())
          throw (engine_error() << "write query execute failed.");
	break;
      }

      case connector::request::quit_q: {
	connector::quit_response quit;
	QByteArray data = quit.build();
	if (write(1, data.constData(), data.size()) != data.size())
          throw (engine_error() << "write query quit failed.");
	return (EXIT_SUCCESS);
      }

      default:
	break;
      }
    }
  }
  catch (std::exception const& e) {
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}
