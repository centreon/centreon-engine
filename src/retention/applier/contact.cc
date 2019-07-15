/*
** Copyright 2011-2013,2016 Merethis
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
  for (list_contact::const_iterator it{lst.begin()}, end{lst.end()};
       it != end; ++it) {
    contact_map::const_iterator ct_it{engine::contact::contacts.find((*it)->contact_name())};
    if (ct_it != engine::contact::contacts.end())
      _update(config, **it, ct_it->second.get());
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
       com::centreon::engine::contact* obj) {
  if (state.modified_attributes().is_set()) {
    obj->set_modified_attributes(*state.modified_attributes() & ~0L);
    // mask out attributes we don't want to retain.
  }
  if (state.modified_host_attributes().is_set()) {
    obj->set_modified_host_attributes(*state.modified_host_attributes()
                                     & ~config.retained_contact_host_attribute_mask());
    // mask out attributes we don't want to retain.
  }
  if (state.modified_service_attributes().is_set()) {
    obj->set_modified_service_attributes(*state.modified_service_attributes()
                                        & ~config.retained_contact_service_attribute_mask());
    // mask out attributes we don't want to retain.
  }
  if (obj->get_retain_status_information()) {
    if (state.last_host_notification().is_set())
      obj->set_last_host_notification(*state.last_host_notification());
    if (state.last_service_notification().is_set())
      obj->set_last_service_notification(*state.last_service_notification());
  }
  if (obj->get_retain_nonstatus_information()) {
    if (state.host_notification_period().is_set()) {
      if (obj->get_modified_host_attributes() & MODATTR_NOTIFICATION_TIMEPERIOD) {
        timeperiod* temp_timeperiod(nullptr);
        timeperiod_map::const_iterator
          found(timeperiod::timeperiods.find(
            state.host_notification_period()));

        if (found != timeperiod::timeperiods.end())
          temp_timeperiod = found->second.get();

        if (!temp_timeperiod)
          obj->set_modified_host_attributes(
                obj->get_modified_host_attributes()
                - MODATTR_NOTIFICATION_TIMEPERIOD);
        else
          obj->set_host_notification_period(*state.host_notification_period());
      }
    }
    if (state.service_notification_period().is_set()) {
      if (obj->get_modified_service_attributes()
          & MODATTR_NOTIFICATION_TIMEPERIOD) {
        timeperiod* temp_timeperiod(nullptr);
        timeperiod_map::const_iterator
          found(timeperiod::timeperiods.find(
          state.host_notification_period()));

        if (found != timeperiod::timeperiods.end())
          temp_timeperiod = found->second.get();

        if (!temp_timeperiod)
          obj->set_modified_service_attributes(
                obj->get_modified_service_attributes()
                - MODATTR_NOTIFICATION_TIMEPERIOD);
        else
          obj->set_service_notification_period(*state.service_notification_period());
      }
    }
    if (state.host_notifications_enabled().is_set()) {
      if (obj->get_modified_host_attributes() & MODATTR_NOTIFICATIONS_ENABLED)
        obj->set_host_notifications_enabled(*state.host_notifications_enabled());
    }
    if (state.service_notifications_enabled().is_set()) {
      if (obj->get_modified_service_attributes() & MODATTR_NOTIFICATIONS_ENABLED)
        obj->set_service_notifications_enabled(*state.service_notifications_enabled());
    }

    if (!state.customvariables().empty()
        && (obj->get_modified_attributes() & MODATTR_CUSTOM_VARIABLE)) {
      for (std::pair<std::string, std::shared_ptr<customvariable>> const& cv : state.customvariables()) {
        obj->get_custom_variables()[cv.first]->update(cv.second->get_value());
      }
    }
  }
  // Adjust modified attributes if necessary.
  else
    obj->set_modified_attributes(MODATTR_NONE);

  // Adjust modified attributes if no custom variable has been changed.
  if (obj->get_modified_attributes() & MODATTR_CUSTOM_VARIABLE) {
    bool at_least_one_modified(false);
    for (std::pair<std::string, std::shared_ptr<customvariable>> const& cv : obj->get_custom_variables())
      if (cv.second->has_been_modified()) {
        at_least_one_modified = true;
        break;
      }
    if (!at_least_one_modified)
      obj->set_modified_attributes(
            obj->get_modified_attributes()
            - MODATTR_CUSTOM_VARIABLE);
  }

  // update contact status.
  obj->update_status_info(false);
}
