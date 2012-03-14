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

#include "com/centreon/engine/commands/connector/execute_query.hh"
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine::commands::connector;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
execute_query::execute_query(
                 unsigned long cmd_id,
                 QString const& cmd,
                 QDateTime const& start_time,
                 unsigned int timeout)
  : request(request::execute_q),
    _cmd(cmd),
    _cmd_id(cmd_id),
    _start_time(start_time),
    _timeout(timeout) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
execute_query::execute_query(execute_query const& right)
  : request(right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
execute_query::~execute_query() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
execute_query& execute_query::operator=(execute_query const& right) {
  if (this != &right) {
    request::operator=(right);
    _internal_copy(right);
  }
  return (*this);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if objects have the same values.
 */
bool execute_query::operator==(
                      execute_query const& right) const throw () {
  return (request::operator==(right)
          && (_cmd == right._cmd)
          && (_cmd_id == right._cmd_id)
          && (_start_time == right._start_time)
          && (_timeout == right._timeout));
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if objects have the different values.
 */
bool execute_query::operator!=(
                      execute_query const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Create the data with the request information.
 *
 *  @return The data request.
 */
QByteArray execute_query::build() {
  QByteArray query =
    QByteArray().setNum(_id) + '\0' +
    QByteArray().setNum(static_cast<qulonglong>(_cmd_id)) + '\0' +
    QByteArray().setNum(_timeout) + '\0' +
    QByteArray().setNum(_start_time.toTime_t()) + '\0';
  query += _cmd.toAscii();
  return (query + cmd_ending());
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
request* execute_query::clone() const {
  return (new execute_query(*this));
}

/**
 *  Get the argument list.
 *
 *  @return The argument list.
 */
QStringList execute_query::get_args() const throw () {
  static QString sep("\"'\t ");
  QString line = _cmd.trimmed();
  QStringList list;
  QString tmp;
  QChar c;
  int escape = 0;

  QString::const_iterator it = line.begin();
  QString::const_iterator end = line.end();

  if (c == 0 && it + 1 < end) {
    while (it->isSpace() && ((it + 1)->isSpace() || *(it + 1) == '\'' || *(it + 1) == '"'))
      ++it;
    if (!(c = (sep.contains(*it) ? *it : ' ')).isSpace())
      ++it;
  }

  for (; it != end; ++it) {
    if (c == 0 && it + 1 < end) {
      while (it->isSpace() && ((it + 1)->isSpace() || *(it + 1) == '\'' || *(it + 1) == '"'))
        ++it;
      c = (sep.contains(*it) ? *it : ' ');
      ++it;
    }

    if (*it == '\\') {
      if (++escape % 2 == 0)
        tmp += *it;
      continue ;
    }

    if (c == *it && escape % 2 == 0) {
      list << tmp;
      tmp = "";
      c = (c.isSpace() && !sep.contains(*(it + 1)) ? ' ' : 0);
    }
    else
      tmp += *it;
    escape = 0;
  }
  if (tmp != "")
    list << tmp;
  return (list);
}

/**
 *  Get the command line.
 *
 *  @return The command line.
 */
QString const& execute_query::get_command() const throw () {
  return (_cmd);
}

/**
 *  Get the command id.
 *
 *  @return The command id.
 */
unsigned long execute_query::get_command_id() const throw () {
  return (_cmd_id);
}

/**
 *  Get the start date time.
 *
 *  @return The start date time.
 */
QDateTime const& execute_query::get_start_time() const throw () {
 return (_start_time);
}

/**
 *  Get the timeout value.
 *
 *  @return The timeout value.
 */
unsigned int execute_query::get_timeout() const throw () {
 return (_timeout);
}

/**
 *  Restore object with the data information.
 *
 *  @param[in] data The data of the request information.
 */
void execute_query::restore(QByteArray const& data) {
  QList<QByteArray> list = data.split('\0');
  if (list.size() < 5)
    throw (engine_error() << "bad request argument");

  bool ok;
  int id = list[0].toInt(&ok);
  if (!ok || id < 0 || id != _id)
    throw (engine_error() << "bad request id");

  _cmd_id = list[1].toULong(&ok);
  if (!ok)
    throw (engine_error() << "bad request argument, invalid cmd_id");

  _timeout = list[2].toUInt(&ok);
  if (!ok)
    throw (engine_error() << "bad request argument, invalid timeout");

  unsigned int timestamp = list[3].toUInt(&ok);
  if (!ok)
    throw (engine_error()
           << "bad request argument, invalid start_time");

  _start_time.setTime_t(timestamp);

  _cmd = list[4];
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void execute_query::_internal_copy(execute_query const& right) {
  _cmd = right._cmd;
  _cmd_id = right._cmd_id;
  _start_time = right._start_time;
  _timeout = right._timeout;
  return ;
}
