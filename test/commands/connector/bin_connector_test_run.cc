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

#include <cstdlib>
#include <ctime>
#include <list>
#include <QCoreApplication>
#include <string>
#include <unistd.h>
#include "com/centreon/engine/commands/connector/execute_query.hh"
#include "com/centreon/engine/commands/connector/execute_response.hh"
#include "com/centreon/engine/commands/connector/quit_response.hh"
#include "com/centreon/engine/commands/connector/request_builder.hh"
#include "com/centreon/engine/commands/connector/version_response.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/version.hh"
#include "com/centreon/shared_ptr.hh"
#include "com/centreon/timestamp.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Wait and get request.
 *
 *  @return The request was send by engine.
 */
static shared_ptr<connector::request> wait() {
  static connector::request_builder& req_builder = connector::request_builder::instance();
  static std::list<shared_ptr<connector::request> > requests;
  static std::string data;

  while (requests.size() == 0) {
    char buf[4096];
    int ret(read(0, buf, sizeof(buf) - 1));
    data.append(buf, ret);

    while (data.size() > 0) {
      size_t pos(data.find(connector::request::cmd_ending()));
      if (pos == std::string::npos)
        break ;

      std::string req_data(data.substr(0, pos));
      data.erase(0, pos + connector::request::cmd_ending().size());

      try {
        requests.push_back(req_builder.build(req_data));
      }
      catch (std::exception const& e) {
        (void)e;
      }
    }
  }

  shared_ptr<connector::request> req(requests.front());
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
static std::string execute_process(
                     std::list<std::string> const& argv,
                     unsigned int timeout,
                     int* exit_code) {
  std::string output;
  for (std::list<std::string>::const_iterator
         it(argv.begin()),
         end(argv.end());
       it != end;
       ++it) {
    output += *it;
    std::list<std::string>::const_iterator next(it);
    ++next;
    if (next != end)
      output += " ";
  }

  if (argv.size() != 2) {
    *exit_code = STATE_WARNING;
    return (output);
  }

  std::string const& arg(*++argv.begin());
  if (arg == "--timeout=on") {
    sleep(timeout  + 1);
  }
  else if (arg == "--timeout=off") {
    *exit_code = STATE_OK;
  }
  else if (arg.find("--kill=") == 0) {
    std::string value(arg.substr(7));
    unsigned int start_time(strtoul(value.c_str(), NULL, 0));
    unsigned int now(time(NULL));

    if (now < start_time + 1) {
      sleep(1);
      char* ptr = NULL;
      ptr[0] = 0;
    }
    else
      *exit_code = STATE_OK;
  }
  else
    *exit_code = STATE_UNKNOWN;
  return (output);
}

/**
 *  Simulate some behavior of connector.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    while (true) {
      shared_ptr<connector::request> req(wait());

      switch (req->get_id()) {
      case connector::request::version_q: {
        connector::version_response version(
                                      CENTREON_ENGINE_VERSION_MAJOR,
                                      CENTREON_ENGINE_VERSION_MINOR);
        std::string data(version.build());
        if (write(STDOUT_FILENO, data.c_str(), data.size())
            != static_cast<ssize_t>(data.size()))
          throw (engine_error() << "write query version failed");
        break ;
      }

      case connector::request::execute_q: {
        connector::execute_query const*
          exec_query(
            static_cast<connector::execute_query const*>(&(*req)));
        int exit_code(STATE_OK);
        std::string output(execute_process(
                             exec_query->get_args(),
                             exec_query->get_timeout(),
                             &exit_code));
        connector::execute_response execute(
                                      exec_query->get_command_id(),
                                      true,
                                      exit_code,
                                      com::centreon::timestamp::now(),
                                      "",
                                      output);
        std::string data(execute.build());
        if (write(1, data.c_str(), data.size())
            != static_cast<ssize_t>(data.size()))
          throw (engine_error() << "write query execute failed");
        break;
      }

      case connector::request::quit_q: {
        connector::quit_response quit;
        std::string data(quit.build());
        if (write(1, data.c_str(), data.size())
            != static_cast<ssize_t>(data.size()))
          throw (engine_error() << "write query quit failed");
        return (EXIT_SUCCESS);
      }

      default:
        break ;
      }
    }
  }
  catch (std::exception const& e) {
    (void)e;
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}
