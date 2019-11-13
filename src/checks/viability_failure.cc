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

#include "com/centreon/engine/checks/viability_failure.hh"

using namespace com::centreon::engine::checks;

/**
 *  Default constructor.
 */
viability_failure::viability_failure() {}

/**
 *  Debug constructor.
 *
 *  @param[in] file      Source file.
 *  @param[in] function  Source function.
 *  @param[in] line      Line at which exception is thrown.
 */
viability_failure::viability_failure(char const* file,
                                     char const* function,
                                     int line)
    : error(file, function, line) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
viability_failure::viability_failure(viability_failure const& other)
    : error(other) {}

/**
 *  Destructor.
 */
viability_failure::~viability_failure() throw() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
viability_failure& viability_failure::operator=(
    viability_failure const& other) {
  error::operator=(other);
  return (*this);
}
