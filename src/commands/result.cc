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

#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/timestamp.hh"

using namespace com::centreon;
using namespace com::centreon::engine::commands;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Constructor.
 */
result::result() : command_id(0), exit_code(0), exit_status(process::normal) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The copy class.
 */
result::result(result const& right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
result::~result() throw() {}

/**
 *  Default copy operator.
 *
 *  @param[in] right The copy class.
 *
 *  @return This object.
 */
result& result::operator=(result const& right) {
  if (this != &right)
    _internal_copy(right);
  return (*this);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool result::operator==(result const& right) const throw() {
  return (command_id == right.command_id && exit_code == right.exit_code &&
          exit_status == right.exit_status && end_time == right.end_time &&
          start_time == right.start_time && output == right.output);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool result::operator!=(result const& right) const throw() {
  return (!operator==(right));
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
void result::_internal_copy(result const& right) {
  command_id = right.command_id;
  end_time = right.end_time;
  exit_code = right.exit_code;
  exit_status = right.exit_status;
  start_time = right.start_time;
  output = right.output;
  return;
}
