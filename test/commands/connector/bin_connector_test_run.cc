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

#include <QDebug>

#include <QCoreApplication>
#include <QList>
#include <QSharedPointer>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <stdlib.h>
#include "engine.hh"
#include "commands/connector/quit_response.hh"
#include "commands/connector/version_response.hh"
#include "commands/connector/execute_response.hh"
#include "commands/connector/execute_query.hh"
#include "commands/connector/request_builder.hh"

using namespace com::centreon::engine::commands;

QSharedPointer<connector::request> wait() {
  static connector::request_builder& req_builder = connector::request_builder::instance();
  static QList<QSharedPointer<connector::request> > requests;
  static QByteArray data;

  while (requests.size() == 0) {
    char buf[4096];
    int ret = read(0, buf, sizeof(buf) - 1);
    data.append(buf, ret);

    write(2, buf, ret);

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
	qDebug() << e.what();
      }
    }
  }

  QSharedPointer<connector::request> req = requests.front();
  requests.pop_front();
  return (req);
}

QString execute_process(QStringList const& argv, int timeout, int* exit_code) {
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
    sleep(timeout / 1000  + 1);
  }

  if (argv[1] == "--timeout=off") {
    *exit_code = STATE_OK;
  }
  else {
    *exit_code = STATE_UNKNOWN;
  }
  return (output);
}

int main(int argc, char** argv) {
  try {
    QCoreApplication app(argc, argv);

    while (true) {
      qDebug() << __func__ << "[" << __LINE__ << "]: start loop.";
      QSharedPointer<connector::request> req = wait();
      qDebug() << __func__ << "[" << __LINE__ << "]: get request.";

      switch (req->get_id()) {
      case connector::request::version_q: {
	qDebug() << __func__ << "[" << __LINE__ << "]: recv version_q";
	connector::version_response version(ENGINE_MAJOR, ENGINE_MINOR);
	QByteArray data = version.build();
	write(1, data.constData(), data.size());
	break;
      }

      case connector::request::execute_q: {
	qDebug() << __func__ << "[" << __LINE__ << "]: recv execute_q";
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
	write(1, data.constData(), data.size());
	break;
      }

      case connector::request::quit_q: {
	qDebug() << __func__ << "[" << __LINE__ << "]: recv quit_q";
	connector::quit_response quit;
	QByteArray data = quit.build();
	write(1, data.constData(), data.size());
	return (EXIT_SUCCESS);
      }

      default:
	qDebug() << __func__ << "[" << __LINE__ << "]: bad request id.";
	break;
      }
      qDebug() << __func__ << "[" << __LINE__ << "]: end loop.";
    }
  }
  catch (std::exception const& e) {
    qDebug() << __func__ << "[" << __LINE__ << "]: error: " << e.what();
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}
