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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/serviceescalation.hh"

using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine;

serviceescalation_mmap serviceescalation::serviceescalations;

serviceescalation::serviceescalation(std::string const& hostname,
                                     std::string const& description,
                                     int first_notification,
                                     int last_notification,
                                     double notification_interval,
                                     std::string const& escalation_period,
                                     uint32_t escalate_on)
    : escalation{first_notification, last_notification, notification_interval,
                 escalation_period, escalate_on},
      _hostname{hostname},
      _description{description} {
  if (hostname.empty())
    throw engine_error() << "Could not create escalation "
                         << "on a host without name";
  if (description.empty())
    throw engine_error() << "Could not create escalation "
                         << "on a service without description";
}

std::string const& serviceescalation::get_hostname() const {
  return _hostname;
}

std::string const& serviceescalation::get_description() const {
  return _description;
}
