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

#ifndef TEST_LOGGING_COMMON_HH
#define TEST_LOGGING_COMMON_HH

#include "com/centreon/engine/namespace.hh"
#include "com/centreon/logging/backend.hh"

CCE_BEGIN()

namespace logging {
/**
 *  @class test test.hh
 *  @brief Class test for testing logging system.
 *
 *  Simple Class for testing logging system.
 */
class test : public com::centreon::logging::backend {
 public:
  test(std::string const& msg,
       uint64_t type,
       uint32_t verbosity,
       uint32_t total_call);
  ~test() throw();
  void close() throw();
  void flush() throw();
  static uint32_t get_nb_instance();
  void log(uint64_t type,
           uint32_t verbosity,
           char const* message,
           uint32_t size) throw();
  void open();
  void reopen();

 private:
  std::string _msg;
  uint32_t _nb_call;
  static uint32_t _nb_instance;
  uint32_t _total_call;
  uint64_t _type;
  uint32_t _verbosity;
};
}  // namespace logging

CCE_END()

#endif  // !TEST_LOGGING_COMMON_HH
