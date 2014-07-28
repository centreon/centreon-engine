/*
** Copyright 2011-2014 Merethis
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

#include <algorithm>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/downtime.hh"
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/contactsmember.hh"
#include "com/centreon/engine/deleter/contactgroupsmember.hh"
#include "com/centreon/engine/deleter/customvariablesmember.hh"
#include "com/centreon/engine/deleter/hostsmember.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/deleter/servicesmember.hh"
#include "com/centreon/engine/configuration/downtime.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;


/**
 *  Default constructor.
 */
applier::downtime::downtime() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::downtime::downtime(applier::downtime const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::downtime::~downtime() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::downtime& applier::downtime::operator=(applier::downtime const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new downtime.
 *
 *  @param[in] obj The new downtime to add into the monitoring engine.
 */
void applier::downtime::add_object(
               shared_ptr<configuration::downtime> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new downtime.";

  // Add downtime to the global configuration set.
  config->downtimes().insert(obj);

  //if ()

  // Create downtime.
  if (add_host_downtime(
    obj->host_name().c_str(),
    obj->entry_time(),
    obj->author().c_str(),
    obj->comment_data().c_str(),
    obj->start_time(),
    obj->end_time(),
    obj->fixed(),
    obj->triggered_by(),
    obj->duration(),
    obj->recurring_interval(),
    obj->recurring_period(),
    obj->downtime_id()) == ERROR)
    throw (engine_error() << "Could not register downtime '"
         << obj->host_name() << "'");

  register_downtime(HOST_DOWNTIME, obj->downtime_id());
}

void applier::downtime::expand_object(
    shared_ptr<configuration::downtime> obj,
    configuration::state& s) {

}

void applier::downtime::modify_object(
    shared_ptr<configuration::downtime> obj) {

}

void applier::downtime::remove_object(
    shared_ptr<configuration::downtime> obj) {

}

void applier::downtime::resolve_object(
    shared_ptr<configuration::downtime> obj) {

}
