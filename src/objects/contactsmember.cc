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

#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/contactsmember.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
contactsmember const* release_contactsmember(
                        contactsmember const* obj) {
  try {
    return (objects::release(obj));
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "error: " << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "error: release_contactsmember: unknow exception";
  }
  return (NULL);
}

/**
 *  Cleanup memory of contactsmember.
 *
 *  @param[in] obj The contact member to cleanup memory.
 *
 *  @return The next contactsmember.
 */
contactsmember const* objects::release(contactsmember const* obj) {
  if (obj == NULL)
    return (NULL);

  contactsmember const* next = obj->next;
  delete[] obj->contact_name;
  delete obj;
  return (next);
}
