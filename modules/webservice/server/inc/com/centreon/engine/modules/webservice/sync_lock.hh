/*
** Copyright 2012-2013 Merethis
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

#ifndef CCE_MOD_WS_SYNC_LOCK_HH
#  define CCE_MOD_WS_SYNC_LOCK_HH

#  include "com/centreon/engine/modules/webservice/namespace.hh"

CCE_MOD_WS_BEGIN()

/**
 *  @class sync_lock sync_lock.hh "com/centreon/engine/modules/webservice/sync_lock.hh"
 *  @brief Handle resource release of the sync class.
 *
 *  Handle accesses to the sync class to prevent resource leak.
 *
 *  @see sync
 */
class        sync_lock {
public:
             sync_lock();
             ~sync_lock();

private:
             sync_lock(sync_lock const& right);
  sync_lock& operator=(sync_lock const& right);
  void       _internal_copy(sync_lock const& right);
};

CCE_MOD_WS_END()

#endif // !CCE_MOD_WS_SYNC_LOCK_HH
