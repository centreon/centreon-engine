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

#ifndef CCE_COMMANDS_RESULT_HH
#  define CCE_COMMANDS_RESULT_HH

#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/process.hh"
#  include "com/centreon/timestamp.hh"

CCE_BEGIN()

namespace              commands {
  /**
   *  @class result result.hh
   *  @brief Result contain the result of execution process.
   *
   *  Result contain the result of execution process (output, retvalue,
   *  execution time).
   */
  class                result {
  public:
                       result();
                       result(result const& right);
                       ~result() throw ();
    result&            operator=(result const& right);
    bool               operator==(result const& right) const throw ();
    bool               operator!=(result const& right) const throw ();
    uint64_t command_id;
    timestamp          end_time;
    int                exit_code;
    process::status    exit_status;
    timestamp          start_time;
    std::string        output;

  private:
    void               _internal_copy(result const& right);
  };
}

CCE_END()

#endif // !CCE_COMMANDS_RESULT_HH
