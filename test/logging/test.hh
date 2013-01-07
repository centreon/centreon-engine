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
#  define TEST_LOGGING_COMMON_HH

#  include "com/centreon/engine/logging/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace               logging {
  /**
   *  @class test test.hh
   *  @brief Class test for testing logging system.
   *
   *  Simple Class for testing logging system.
   */
  class                 test : public object {
  public:
                        test(
                          std::string const& msg,
                          unsigned long long type,
                          unsigned int verbosity,
                          unsigned int total_call);
                        ~test();
    static unsigned int get_nb_instance();
    void                log(
                          char const* message,
                          unsigned long long type,
                          unsigned int verbosity) throw ();

  private:
    std::string         _msg;
    unsigned int        _nb_call;
    static unsigned int _nb_instance;
    unsigned int        _total_call;
    unsigned long long  _type;
    unsigned int        _verbosity;
  };
}

CCE_END()

#endif // !TEST_LOGGING_COMMON_HH
