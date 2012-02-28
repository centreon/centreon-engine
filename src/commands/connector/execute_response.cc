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

#include <sstream>
#include <vector>
#include "error.hh"
#include "commands/connector/execute_response.hh"

using namespace com::centreon::engine::commands::connector;

/**
 *  Default constructor.
 */
execute_response::execute_response(unsigned long cmd_id,
		 bool is_executed,
		 int exit_code,
		 QDateTime const& end_time,
		 std::string const& err,
		 std::string const& out)
  : request(request::execute_r),
    _stderr(err),
    _stdout(out),
    _end_time(end_time),
    _cmd_id(cmd_id),
    _exit_code(exit_code),
    _is_executed(is_executed) {

}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
execute_response::execute_response(execute_response const& right)
  : request(right) {
  operator=(right);
}

/**
 *  Default destructor.
 */
execute_response::~execute_response() throw() {

}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 *
 *  @return This object.
 */
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

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool execute_response::operator==(execute_response const& right) const throw() {
  return (request::operator==(right) == true
	  && _stderr == right._stderr
	  && _stdout == right._stdout
	  && _end_time == right._end_time
	  && _id == right._id
	  && _exit_code == right._exit_code
	  && _is_executed == right._is_executed);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool execute_response::operator!=(execute_response const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
request* execute_response::clone() const {
  return (new execute_response(*this));
}

/**
 *  Create the data with the request information.
 *
 *  @return The data request.
 */
std::string execute_response::build() {
  std::ostringstream oss;
  oss << _id << '\0'
      << static_cast<qulonglong>(_cmd_id) << '\0'
      << _is_executed << '\0'
      << _exit_code << '\0'
      << _end_time.toTime_t() << '\0'
      << _stderr.c_str() << '\0'
      << _stdout.c_str();
  oss.write(cmd_ending().c_str(), cmd_ending().size());
  return (oss.str());
}

/**
 *  Restore object with the data information.
 *
 *  @param[in] data The data of the request information.
 */
void execute_response::restore(std::string const& data) {
  std::vector<std::string> list;
  size_t last(0);
  size_t pos(data.find('\0', last));
  while (pos != std::string::npos) {
    list.push_back(data.substr(last, pos - last));
    last = pos + 1;
    pos = data.find('\0', last);
  }
  if (last != data.size())
    list.push_back(data.substr(last));

  if (list.size() != 7) {
    throw (engine_error() << "bad request argument.");
  }

  int id(0);
  std::istringstream iss(list[0]);
  if ((!(iss >> id) || !iss.eof()) || id < 0 || id != _id)
    throw (engine_error() << "bad request id.");

  {
    std::istringstream iss(list[1]);
    if (!(iss >> _cmd_id) || !iss.eof())
      throw (engine_error() << "bad request argument, invalid cmd_id.");
  }

  {
    std::istringstream iss(list[2]);
    if (!(iss >> _is_executed) || !iss.eof())
      throw (engine_error() << "bad request argument, invalid is_executed.");
  }

  {
    std::istringstream iss(list[3]);
    if (!(iss >> _exit_code) || !iss.eof())
      throw (engine_error() << "bad request argument, invalid exit_code.");
  }

  {
    unsigned int timestamp;
    std::istringstream iss(list[4]);
    if (!(iss >> timestamp) || !iss.eof())
      throw (engine_error() << "bad request argument, invalid end_time.");
    _end_time.setTime_t(timestamp);
  }

  _stderr = list[5];
  _stdout = list[6];
}

/**
 *  Get the error string.
 *
 *  @return The error string.
 */
std::string const& execute_response::get_stderr() const throw() {
 return (_stderr);
}

/**
 *  Get the output string.
 *
 *  @return The output string.
 */
std::string const& execute_response::get_stdout() const throw() {
 return (_stdout);
}

/**
 *  Get the end time of execute command.
 *
 *  @return The date time.
 */
QDateTime const& execute_response::get_end_time() const throw() {
 return (_end_time);
}

/**
 *  Get the command line id.
 *
 *  @return The command line id.
 */
unsigned long execute_response::get_command_id() const throw() {
 return (_cmd_id);
}

/**
 *  Get the exit code value.
 *
 *  @return The exit code of the command line.
 */
int execute_response::get_exit_code() const throw() {
 return (_exit_code);
}

/**
 *  Get is the command was executed.
 *
 *  @return True if the command was executed.
 */
bool execute_response::get_is_executed() const throw() {
 return (_is_executed);
}
