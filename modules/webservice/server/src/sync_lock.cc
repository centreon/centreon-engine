/*
** Copyright 2012 Merethis
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

#include <assert.h>
#include <stdlib.h>
#include "com/centreon/engine/modules/webservice/sync.hh"
#include "com/centreon/engine/modules/webservice/sync_lock.hh"

using namespace com::centreon::engine::modules::webservice;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  @brief Default constructor.
 *
 *  Wait for Engine thread safeness.
 *
 *  @see sync
 */
sync_lock::sync_lock() {
  sync::instance().wait_thread_safeness();
}

/**
 *  @brief Destructor.
 *
 *  Release processing for further threads.
 */
sync_lock::~sync_lock() {
  sync::instance().worker_finish();
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Copy constructor.
 *
 *  @param[in] right Unused.
 */
sync_lock::sync_lock(sync_lock const& right) {
  _internal_copy(right);
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Unused.
 *
 *  @return This object.
 */
sync_lock& sync_lock::operator=(sync_lock const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Copy internal data members.
 *
 *  @param[in] right Unused.
 */
void sync_lock::_internal_copy(sync_lock const& right) {
  (void)right;
  assert(!"synchronization lock is not copyable");
  abort();
  return ;
}
