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

#include "com/centreon/engine/retention/applier/downtime.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/downtimes/host_downtime.hh"
#include "com/centreon/engine/downtimes/service_downtime.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::retention;

/**
 *  Add downtimes on appropriate hosts and services.
 *
 *  @param[in] lst The downtime list to add.
 */
void applier::downtime::apply(list_downtime const& lst) {
  // Big speedup when reading retention.dat in bulk.

  for (list_downtime::const_iterator it(lst.begin()), end(lst.end()); it != end;
       ++it) {
    if ((*it)->downtime_type() == retention::downtime::host)
      _add_host_downtime(**it);
    else
      _add_service_downtime(**it);
  }
}

/**
 *  Add host downtime.
 *
 *  @param[in] obj The downtime to add into the host.
 */
void applier::downtime::_add_host_downtime(
    retention::downtime const& obj) throw() {
  downtimes::host_downtime* dt{new downtimes::host_downtime(
      obj.host_name(), obj.entry_time(), obj.author(), obj.comment_data(),
      obj.start_time(), obj.end_time(), obj.fixed(), obj.triggered_by(),
      obj.duration(), obj.downtime_id())};
  dt->schedule();
  downtimes::downtime_manager::instance().register_downtime(HOST_DOWNTIME,
                                                            obj.downtime_id());
}

/**
 *  Add serivce downtime.
 *
 *  @param[in] obj The downtime to add into the service.
 */
void applier::downtime::_add_service_downtime(
    retention::downtime const& obj) throw() {
  downtimes::service_downtime* dt{new downtimes::service_downtime(
      obj.host_name(), obj.service_description(), obj.entry_time(),
      obj.author(), obj.comment_data(), obj.start_time(), obj.end_time(),
      obj.fixed(), obj.triggered_by(), obj.duration(), obj.downtime_id())};
  dt->schedule();
  downtimes::downtime_manager::instance().register_downtime(SERVICE_DOWNTIME,
                                                            obj.downtime_id());
}
