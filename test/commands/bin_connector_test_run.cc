/*
** Copyright 2011-2013 Merethis
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

#include <unistd.h>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <list>
#include <string>
#include <thread>
#include "com/centreon/engine/version.hh"
#include "com/centreon/exceptions/basic.hh"
#include "com/centreon/misc/command_line.hh"
#include "com/centreon/timestamp.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Simulate a execution process.
 *
 *  @param[in] argv      The command arguments.
 *  @param[in] timeout   The command timeout.
 *  @param[in] exit_code The command exit code.
 *
 *  @return The raw query.
 */
static std::string execute_process(char** argv,
                                   uint32_t timeout,
                                   int* exit_code) {
  std::string output;
  uint32_t i(0);
  while (argv[i]) {
    output += argv[i];
    if (argv[++i])
      output += " ";
  }

  if (i != 2) {
    *exit_code = STATE_WARNING;
    return (output);
  }

  std::string const& arg(argv[1]);
  if (arg == "--timeout=on")
    std::this_thread::sleep_for(std::chrono::seconds(timeout + 1));
  else if (arg == "--timeout=off")
    *exit_code = STATE_OK;
  else if (arg.find("--kill=") == 0) {
    std::string value(arg.substr(7));
    uint32_t start_time(strtoul(value.c_str(), NULL, 0));
    uint32_t now(time(NULL));

    if (now < start_time + 1) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      char* ptr(NULL);
      ptr[0] = 0;
    } else
      *exit_code = STATE_OK;
  } else
    *exit_code = STATE_UNKNOWN;
  return (output);
}

/**
 *  Send query execute response.
 *
 *  @param[in] q  The receive data.
 */
static void query_execute(char const* q) {
  // Get query informations.
  char const* startptr(q);
  char* endptr(NULL);
  unsigned long command_id(strtol(startptr, &endptr, 10));
  if (startptr == endptr)
    throw(basic_error() << "invalid query execute: "
                           "invalid command_id");
  startptr = endptr + 1;
  uint32_t timeout(strtol(startptr, &endptr, 10));
  if (startptr == endptr)
    throw(basic_error() << "invalid query execute: "
                           "invalid is_executed");
  startptr = endptr + 1;
  char const* cendptr(startptr);
  while (isdigit(*cendptr))
    ++cendptr;
  if (startptr == cendptr)
    throw(basic_error() << "invalid query execute: "
                           "invalid exit_code");
  char const* cmd(cendptr + 1);

  misc::command_line cmdline(cmd);
  int exit_code(STATE_OK);
  std::string output(execute_process(cmdline.get_argv(), timeout, &exit_code));
  std::ostringstream oss;
  oss << "3" << '\0' << command_id << '\0' << "1" << '\0' << exit_code << '\0'
      << "" << '\0' << output << '\0' << std::string(3, '\0');
  std::string data(oss.str());
  if (write(STDOUT_FILENO, data.c_str(), data.size()) !=
      static_cast<ssize_t>(data.size()))
    throw(basic_error() << "write query execute failed");
}

/**
 *  Send query quit response.
 *
 *  @param[in] q  The receive data.
 */
static void query_quit(char const* q) {
  (void)q;
  std::ostringstream oss;
  oss << "5" << '\0' << std::string(3, '\0');
  std::string data(oss.str());
  if (write(STDOUT_FILENO, data.c_str(), data.size()) !=
      static_cast<ssize_t>(data.size()))
    throw(basic_error() << "write query quit failed");
  exit(EXIT_SUCCESS);
}

/**
 *  Send query version response.
 *
 *  @param[in] q  The receive data.
 */
static void query_version(char const* q) {
  (void)q;
  std::ostringstream oss;
  oss << "1" << '\0' << CENTREON_ENGINE_VERSION_MAJOR << '\0'
      << CENTREON_ENGINE_VERSION_MINOR << '\0' << std::string(3, '\0');
  std::string data(oss.str());
  if (write(STDOUT_FILENO, data.c_str(), data.size()) !=
      static_cast<ssize_t>(data.size()))
    throw(basic_error() << "write query version failed");
}

/**
 *  Wait and get request.
 *
 *  @param[out] query  The requst data.
 *
 *  @return The request id was send by engine, or -1 to quit.
 */
static int wait(std::string& query) {
  static std::list<std::string> responses;
  static std::string ending(4, '\0');
  static std::string data;

  // Split outpout into queries responses.
  while (responses.empty()) {
    char buffer[4096];
    ssize_t ret(read(STDIN_FILENO, buffer, sizeof(buffer)));
    if (ret < 0) {
      char const* msg(strerror(errno));
      throw(basic_error() << "invalid read: " << msg);
    }
    if (!ret)
      return (-1);
    data.append(buffer, ret);
    while (data.size() > 0) {
      size_t pos(data.find(ending));
      if (pos == std::string::npos)
        break;
      responses.push_back(data.substr(0, pos + ending.size()));
      data.erase(0, pos + ending.size());
    }
  }
  std::string response(responses.front());
  char const* query_str(response.c_str());
  char* endptr(NULL);
  uint32_t id(strtol(query_str, &endptr, 10));
  if (query_str == endptr) {
    responses.pop_front();
    throw(basic_error() << "invalid query");
  }
  query.clear();
  query.append(endptr + 1, response.size() - ((endptr + 1) - query_str));
  responses.pop_front();
  return (id);
}

/**
 *  Simulate some behavior of connector.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main() {
  typedef void (*send_query)(char const*);
  static send_query tab_send_query[] = {
      &query_version, NULL, &query_execute, NULL, &query_quit, NULL,
  };

  try {
    while (true) {
      std::string query;
      int id(wait(query));

      if (id == -1 ||
          static_cast<uint32_t>(id) >=
              sizeof(tab_send_query) / sizeof(*tab_send_query) ||
          !tab_send_query[id])
        throw(basic_error() << "reveive bad request id: id=" << id);
      (*tab_send_query[id])(query.c_str());
    }
  } catch (std::exception const& e) {
    (void)e;
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}
