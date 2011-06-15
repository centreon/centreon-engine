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

#include <QStringList>
#include "error.hh"
#include "commands/connector/execute_response.hh"

using namespace com::centreon::engine::commands::connector;

execute_response::execute_response(unsigned long cmd_id,
		 bool is_executed,
		 int exit_code,
		 QDateTime const& end_time,
		 QString const& err,
		 QString const& out)
  : request(request::execute_r),
    _stderr(err),
    _stdout(out),
    _end_time(end_time),
    _cmd_id(cmd_id),
    _exit_code(exit_code),
    _is_executed(is_executed) {

}

execute_response::execute_response(execute_response const& right)
  : request(right) {
  operator=(right);
}

execute_response::~execute_response() throw() {

}

execute_response& execute_response::operator=(execute_response const& right) {
  if (this != &right) {
    request::operator=(right);

    _stderr = right._stderr;
    _stdout = right._stdout;
    _end_time = right._end_time;
    _id = right._id;
    _exit_code = right._exit_code;
    _is_executed = right._is_executed;
  }
  return (*this);
}

request* execute_response::clone() const {
  return (new execute_response(*this));
}

QByteArray execute_response::build() {
  QByteArray query =
    QByteArray().setNum(_id) + '\0' +
    QByteArray().setNum(static_cast<qulonglong>(_cmd_id)) + '\0' +
    QByteArray().setNum(_is_executed) + '\0' +
    QByteArray().setNum(_exit_code) + '\0' +
    QByteArray().setNum(_end_time.toMSecsSinceEpoch()) + '\0';
  query += _stderr + '\0' + _stdout;
  return (query + cmd_ending());
}

void execute_response::restore(QByteArray const& data) {
  QList<QByteArray> list = data.split('\0');
  if (list.size() != 7) {
    throw (engine_error() << "bad request argument.");
  }

  bool ok;
  unsigned int id = list[0].toUInt(&ok);
  if (ok == false || id != _id) {
    throw (engine_error() << "bad request id.");
  }

  _cmd_id = list[1].toULong(&ok);
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid cmd_id.");
  }

  _is_executed = list[2].toInt(&ok);
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid is_executed.");
  }

  _exit_code = list[3].toInt(&ok);
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid exit_code.");
  }

  qint64 timestamp = list[4].toLongLong(&ok);
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid end_time.");
  }
  _end_time.setMSecsSinceEpoch(timestamp);

  _stderr = list[5];
  _stdout = list[6];
}

QString const& execute_response::get_stderr() const throw() {
 return (_stderr);
}

QString const& execute_response::get_stdout() const throw() {
 return (_stdout);
}

QDateTime const& execute_response::get_end_time() const throw() {
 return (_end_time);
}

unsigned long execute_response::get_command_id() const throw() {
 return (_cmd_id);
}

int execute_response::get_exit_code() const throw() {
 return (_exit_code);
}

bool execute_response::get_is_executed() const throw() {
 return (_is_executed);
}
