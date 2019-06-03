/*
** Copyright 2011-2019 Centreon
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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/serviceescalation.hh"

using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine;

serviceescalation::serviceescalation(std::string const& hostname,
                                     std::string const& description,
                                     int first_notification,
                                     int last_notification,
                                     double notification_interval,
                                     std::string const& escalation_period)
    : escalation{first_notification, last_notification, notification_interval,
                 escalation_period},
      _hostname{hostname},
      _description{description} {}

serviceescalation::~serviceescalation() {
  this->contact_groups.clear();
  this->contacts.clear();

  // service_ptr not free.
  // escalation_period_ptr not free.

}

/**
 *  Add a new service escalation to the list in memory.
 *
 *  @param[in] host_name             Host name.
 *  @param[in] description           Description.
 *  @param[in] first_notification    First notification.
 *  @param[in] last_notification     Last notification.
 *  @param[in] notification_interval Notification interval.
 *  @param[in] escalation_period     Escalation timeperiod name.
 *  @param[in] escalate_on_warning   Do we escalate on warning ?
 *  @param[in] escalate_on_unknown   Do we escalate on unknown ?
 *  @param[in] escalate_on_critical  Do we escalate on critical ?
 *  @param[in] escalate_on_recovery  Do we escalate on recovery ?
 *
 *  @return New service escalation.
 */
serviceescalation* add_service_escalation(std::string const& host_name,
                                          std::string const& description,
                                          int first_notification,
                                          int last_notification,
                                          double notification_interval,
                                          std::string const& escalation_period,
                                          int escalate_on_warning,
                                          int escalate_on_unknown,
                                          int escalate_on_critical,
                                          int escalate_on_recovery) {
  // Make sure we have the data we need.
  if (host_name.empty() || description.empty()) {
    logger(log_config_error, basic)
        << "Error: Service escalation host name or description is NULL";
    return NULL;
  }

  // Allocate memory for a new service escalation entry.
  std::shared_ptr<serviceescalation> obj(new serviceescalation(
      host_name, description, first_notification, last_notification,
      notification_interval, escalation_period));

  try {
    obj->escalate_on_critical = (escalate_on_critical > 0);
    obj->escalate_on_recovery = (escalate_on_recovery > 0);
    obj->escalate_on_unknown = (escalate_on_unknown > 0);
    obj->escalate_on_warning = (escalate_on_warning > 0);

    // Add new items to the configuration state.
    state::instance().serviceescalations().insert(
        {{obj->get_hostname(), obj->get_description()}, obj});

    // Add new items to tail the list.
    obj->next = serviceescalation_list;
    serviceescalation_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_escalation_data(NEBTYPE_SERVICEESCALATION_ADD, NEBFLAG_NONE,
                                    NEBATTR_NONE, obj.get(), &tv);
  } catch (...) {
    obj.reset();
  }

  return obj.get();
}

std::string const& serviceescalation::get_hostname() const {
  return _hostname;
}

std::string const& serviceescalation::get_description() const {
  return _description;
}
