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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/retention/applier/contact.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::retention;

/**
 *  Update contact list.
 *
 *  @param[in] config The global configuration.
 *  @param[in] lst    The contact list to update.
 */
void applier::contact::apply(
       configuration::state const& config,
       list_contact const& lst) {
  for (list_contact::const_iterator it(lst.begin()), end(lst.end());
       it != end;
       ++it) {
    try {
      contact_struct& cntct(find_contact((*it)->contact_name()));
      _update(config, **it, cntct);
    }
    catch (...) {
      // ignore exception for the retention.
    }
  }
}

/**
 *  Update internal contact base on contact retention.
 *
 *  @param[in]      config The global configuration.
 *  @param[in]      state The contact retention state.
 *  @param[in, out] obj   The contact to update.
 */
void applier::contact::_update(
       configuration::state const& config,
       retention::contact const& state,
       contact_struct& obj) {
  if (state.modified_attributes().is_set()) {
    obj.modified_attributes = *state.modified_attributes();
    // mask out attributes we don't want to retain.
    obj.modified_attributes &= ~0L;
  }
  if (state.modified_host_attributes().is_set()) {
    obj.modified_host_attributes = *state.modified_host_attributes();
    // mask out attributes we don't want to retain.
    obj.modified_host_attributes &= ~config.retained_contact_host_attribute_mask();
  }
  if (state.modified_service_attributes().is_set()) {
    obj.modified_service_attributes = *state.modified_service_attributes();
    // mask out attributes we don't want to retain.
    obj.modified_service_attributes &= ~config.retained_contact_service_attribute_mask();
  }

  if (obj.retain_status_information) {
    if (state.last_host_notification().is_set())
      obj.last_host_notification = *state.last_host_notification();
    if (state.last_service_notification().is_set())
      obj.last_service_notification = *state.last_service_notification();
  }


  if (obj.retain_nonstatus_information) {
    if (state.host_notification_period().is_set()) {
      if (obj.modified_host_attributes & MODATTR_NOTIFICATION_TIMEPERIOD) {
        if (!find_timeperiod(state.host_notification_period()->c_str()))
          obj.modified_host_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
        else
          string::setstr(
            obj.host_notification_period,
            *state.host_notification_period());
      }
    }
    if (state.service_notification_period().is_set()) {
      if (obj.modified_service_attributes & MODATTR_NOTIFICATION_TIMEPERIOD) {
        if (!find_timeperiod(state.service_notification_period()->c_str()))
          obj.modified_service_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
        else
          string::setstr(
            obj.service_notification_period,
            *state.service_notification_period());
      }
    }
    if (state.host_notifications_enabled().is_set()) {
      if (obj.modified_host_attributes & MODATTR_NOTIFICATIONS_ENABLED)
        obj.host_notifications_enabled = *state.host_notifications_enabled();
    }
    if (state.service_notifications_enabled().is_set()) {
      if (obj.modified_service_attributes & MODATTR_NOTIFICATIONS_ENABLED)
        obj.service_notifications_enabled = *state.service_notifications_enabled();
    }

    if (!state.customvariables().empty()
        && (obj.modified_attributes & MODATTR_CUSTOM_VARIABLE)) {
      for (map_customvar::const_iterator
             it(state.customvariables().begin()),
             end(state.customvariables().end());
           it != end;
           ++it) {
        update_customvariable(
          obj.custom_variables,
          it->first,
          it->second);
      }
    }
  }

  // adjust modified attributes if necessary.
  if (!obj.retain_nonstatus_information)
    obj.modified_attributes = MODATTR_NONE;

  // adjust modified attributes if no custom variables have been changed.
  if (obj.modified_attributes & MODATTR_CUSTOM_VARIABLE) {
    for (customvariablesmember* member(obj.custom_variables);
         member;
         member = member->next)
      if (member->has_been_modified) {
        obj.modified_attributes -= MODATTR_CUSTOM_VARIABLE;
        break;
      }
  }

  // Update contact status.
  update_contact_status(&obj);
}
