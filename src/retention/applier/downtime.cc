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

#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/objects/downtime.hh"
#include "com/centreon/engine/retention/applier/downtime.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::retention;

/**
 *  Add downtimes on appropriate hosts and services.
 *
 *  @param[in] lst The downtime list to add.
 */
void applier::downtime::apply(list_downtime const& lst) {
  // Big speedup when reading retention.dat in bulk.
  defer_downtime_sorting = 1;

  for (list_downtime::const_iterator it(lst.begin()), end(lst.end());
       it != end;
       ++it) {
    // TRICKY STUFF: non recurring period are loaded entirely as they aren't in the conf.
    // Recurring period are in the conf: they are only loaded if there is no matching periods
    // already loaded somewhere in the engine.
    if ((*it)->recurring_period() == NULL)
      _add_downtime(**it);
    else
      _add_recurring_downtime(**it);

  }

  // Sort all downtimes.
  sort_downtime();
}

/**
 *  Add a non recurring downtime.
 *
 *  @param[in] obj  The non recurring downtime to add.
 */
void  applier::downtime::_add_downtime(
        retention::downtime const& obj) throw() {
  if (obj.downtime_type() == retention::downtime::host)
    _add_host_downtime(obj);
  else
    _add_service_downtime(obj);
}

/**
 *  Add a recurring downtime.
 *
 *  @param[in] obj  The recurring downtime to add.
 */

void  applier::downtime::_add_recurring_downtime(
        retention::downtime const& obj) throw() {
  umap<std::string, shared_ptr<timeperiod_struct> >::iterator it =
      configuration::applier::state::instance().timeperiods().begin();
  umap<std::string, shared_ptr<timeperiod_struct> >::iterator end =
      configuration::applier::state::instance().timeperiods().end();
  for (; it != end; ++it) {
    for (scheduled_downtime* downtime = scheduled_downtime_list;
         downtime != NULL;
         downtime = downtime->next) {
      // Ids can change between recurring downtime: all the rest remain.
      if (is_equal(downtime->host_name, obj.host_name().c_str())
      && is_equal(downtime->service_description, obj.service_description().c_str())
      && downtime->entry_time == obj.entry_time()
      && downtime->start_time == obj.start_time()
      && downtime->end_time == obj.end_time()
      && downtime->fixed == obj.fixed()
      && downtime->triggered_by == obj.triggered_by()
      && downtime->duration == obj.duration()
      && downtime->downtime_id == obj.downtime_id()
      && is_equal(downtime->author, obj.author().c_str())
      && is_equal(downtime->comment, obj.comment_data().c_str())
      && downtime->recurring_interval == obj.recurring_interval()
      && downtime->recurring_period == obj.recurring_period())
        return ;
    }
  }
  _add_downtime(obj);
}

/**
 *  Add host downtime.
 *
 *  @param[in] obj The downtime to add into the host.
 */
void applier::downtime::_add_host_downtime(
       retention::downtime const& obj) throw () {
  add_host_downtime(
    obj.host_name().c_str(),
    obj.entry_time(),
    obj.author().c_str(),
    obj.comment_data().c_str(),
    obj.start_time(),
    obj.end_time(),
    obj.fixed(),
    obj.triggered_by(),
    obj.duration(),
    obj.recurring_interval(),
    obj.recurring_period(),
    obj.downtime_id());
  register_downtime(HOST_DOWNTIME, obj.downtime_id());
}

/**
 *  Add service downtime.
 *
 *  @param[in] obj The downtime to add into the service.
 */
void applier::downtime::_add_service_downtime(
       retention::downtime const& obj) throw () {
  add_service_downtime(
    obj.host_name().c_str(),
    obj.service_description().c_str(),
    obj.entry_time(),
    obj.author().c_str(),
    obj.comment_data().c_str(),
    obj.start_time(),
    obj.end_time(),
    obj.fixed(),
    obj.triggered_by(),
    obj.duration(),
    obj.recurring_interval(),
    obj.recurring_period(),
    obj.downtime_id());
  register_downtime(SERVICE_DOWNTIME, obj.downtime_id());
}
