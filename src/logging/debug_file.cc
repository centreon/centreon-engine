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

#include <cassert>
#include <cstdlib>
#include "com/centreon/engine/logging/debug_file.hh"

using namespace com::centreon::engine::logging;

/**
 *  Constructor.
 *
 *  @param[in] path      Path to the debug file.
 *  @param[in] max_size  Maximum debug file size.
 */
debug_file::debug_file(
              std::string const& path,
              long long max_size)
  : com::centreon::logging::file(
                              path,
                              true,
                              true,
                              com::centreon::logging::second,
                              false,
                              max_size)
  {}

/**
 *  Destructor.
 */
debug_file::~debug_file() throw () {}

/**
 *  Copy constructor.
 */
debug_file::debug_file(debug_file const& other)
  : com::centreon::logging::file("") {
  (void)other;
  assert(!"debug file is not copyable");
  abort();
}

/**
 *  Assignment operator.
 *
 *  @param[in] other Object to copy.
 *
 *  @return This object.
 */
debug_file& debug_file::operator=(debug_file const& other) {
  (void)other;
  assert(!"debug file is not copyable");
  abort();
  return (*this);
}
