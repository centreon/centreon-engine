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

#include <sstream>
#include <vector>
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
                 std::string const& cmd,
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
std::string execute_query::build() {
  std::ostringstream oss;

  oss << _id << '\0'
      << static_cast<qulonglong>(_cmd_id) << '\0'
      << _timeout << '\0'
      << _start_time.toTime_t() << '\0'
      << _cmd.c_str();
  oss.write(cmd_ending().c_str(), cmd_ending().size());
  return (oss.str());
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
std::list<std::string> execute_query::get_args() const throw () {
  static std::string sep("\"'\t ");
  std::string line(_cmd);
  while (!line.empty() && isspace(line[0]))
    line.erase(line.begin());
  while (!line.empty() && isspace(line[line.size() - 1]))
    line.resize(line.size() - 1);
  std::list<std::string> list;
  std::string tmp;
  char c(0);
  int escape(0);

  std::string::const_iterator it = line.begin();
  std::string::const_iterator end = line.end();

  if (it + 1 < end) {
    while (isspace(*it) && (isspace(*(it + 1))
                            || (*(it + 1) == '\'')
                            || (*(it + 1) == '"')))
      ++it;
    if (!isspace(c = ((sep.find(*it) != std::string::npos) ? *it : ' ')))
      ++it;
  }

  for (; it != end; ++it) {
    if (c == 0 && it + 1 < end) {
      while (isspace(*it) && (isspace(*(it + 1))
                              || (*(it + 1) == '\'')
                              || (*(it + 1) == '"')))
        ++it;
      c = ((sep.find(*it) != std::string::npos) ? *it : ' ');
      ++it;
    }

    if (*it == '\\') {
      if (++escape % 2 == 0)
        tmp += *it;
      continue ;
    }

    if (c == *it && escape % 2 == 0) {
      list.push_back(tmp);
      tmp = "";
      c = ((isspace(c) && (sep.find(*(it + 1)) == std::string::npos))
           ? ' '
           : 0);
    }
    else
      tmp += *it;
    escape = 0;
  }
  if (tmp != "")
    list.push_back(tmp);
  return (list);
}

/**
 *  Get the command line.
 *
 *  @return The command line.
 */
std::string const& execute_query::get_command() const throw () {
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
void execute_query::restore(std::string const& data) {
  // Split arguments.
  std::vector<std::string> list;
  size_t prev(0);
  size_t current;
  while ((current = data.find('\0', prev)) != std::string::npos) {
    list.push_back(data.substr(prev, current - prev));
    prev = current + 1;
  }
  list.push_back(data.substr(prev));
  if (list.size() < 5)
    throw (engine_error() << "bad request argument");

  // Request ID.
  char* nok;
  int id(strtol(list[0].c_str(), &nok, 0));
  if (*nok || (id < 0) || (id != _id))
    throw (engine_error() << "bad request id");

  // Command ID.
  _cmd_id = strtoul(list[1].c_str(), &nok, 0);
  if (*nok)
    throw (engine_error() << "bad request argument, invalid cmd_id");

  // Timeout.
  _timeout = strtoul(list[2].c_str(), &nok, 0);
  if (*nok)
    throw (engine_error() << "bad request argument, invalid timeout");

  // Start time.
  unsigned int timestamp(strtoul(list[3].c_str(), &nok, 0));
  if (*nok)
    throw (engine_error()
           << "bad request argument, invalid start_time");
  _start_time.setTime_t(timestamp);

  // Command.
  _cmd = list[4];

  return ;
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
