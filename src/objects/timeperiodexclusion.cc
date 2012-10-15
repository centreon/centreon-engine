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

#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/timeperiodexclusion.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_timeperiodexclusion(timeperiodexclusion const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "error: " << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "error: release_timeperiodexclusion: unknow exception";
  }
  return;
}

/**
 *  Cleanup memory of timeperiodexclusion.
 *
 *  @param[in] obj The timeperiodexclusion to cleanup memory.
 */
void objects::release(timeperiodexclusion const* obj) {
  if (obj == NULL)
    return;

  delete[] obj->timeperiod_name;
  delete obj;
  return;
}
