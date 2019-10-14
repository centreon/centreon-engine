/*
** Copyright 2014 Merethis
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

#ifndef CCE_LOGGING_DEBUG_FILE_HH
#  define CCE_LOGGING_DEBUG_FILE_HH

#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/logging/file.hh"

CCE_BEGIN()

namespace        logging {
  /**
   *  @class debug_file debug_file.hh "com/centreon/engine/logging/debug_file.hh"
   *  @brief Debug file.
   *
   *  Engine debug file.
   */
  class          debug_file : public com::centreon::logging::file {
  public:
                 debug_file(
                   std::string const& path,
                   uint64_t max_size = 0);
                 ~debug_file() throw () override;

  private:
                 debug_file(debug_file const& other);
    debug_file&  operator=(debug_file const& other);
  };
}

CCE_END()

#endif // !CCE_LOGGING_DEBUG_FILE_HH
