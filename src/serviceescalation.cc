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

#include "com/centreon/engine/serviceescalation.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine;

serviceescalation_mmap serviceescalation::serviceescalations;

serviceescalation::serviceescalation(std::string const& hostname,
                                     std::string const& description,
                                     uint32_t first_notification,
                                     uint32_t last_notification,
                                     double notification_interval,
                                     std::string const& escalation_period,
                                     uint32_t escalate_on,
                                     Uuid const& uuid)
    : escalation{first_notification, last_notification, notification_interval,
                 escalation_period,  escalate_on,       uuid},
      _hostname{hostname},
      _description{description} {
  if (hostname.empty())
    throw engine_error() << "Could not create escalation "
                         << "on a host without name";
  if (description.empty())
    throw engine_error() << "Could not create escalation "
                         << "on a service without description";
}

serviceescalation::~serviceescalation() {}

std::string const& serviceescalation::get_hostname() const {
  return _hostname;
}

std::string const& serviceescalation::get_description() const {
  return _description;
}

/**
 *  This method is called by a notifier to know if this escalation is touched
 *  by the notification to send.
 *
 * @param state The notifier state.
 * @param notification_number The current notifier notification number.
 *
 * @return A boolean.
 */
bool serviceescalation::is_viable(int state,
                                  uint32_t notification_number) const {
  logger(dbg_functions, basic) << "serviceescalation::is_viable()";

  bool retval{escalation::is_viable(state, notification_number)};
  if (retval) {
    std::array<notifier::notification_flag, 4> nt = {
        notifier::ok,
        notifier::warning,
        notifier::critical,
        notifier::unknown,
    };

    if (!get_escalate_on(nt[state]))
      return false;
    return true;
  } else
    return retval;
}

void serviceescalation::resolve(int& w, int& e) {
  (void)w;
  int errors{0};

  // Find the service.
  service_map::const_iterator found{
      service::services.find({get_hostname(), get_description()})};
  if (found == service::services.end() || !found->second) {
    logger(log_verification_error, basic)
        << "Error: Service '" << get_description() << "' on host '"
        << get_hostname()
        << "' specified in service escalation is not defined anywhere!";
    errors++;
    notifier_ptr = nullptr;
  } else {
    notifier_ptr = found->second.get();
    notifier_ptr->get_escalations().push_back(this);
  }

  try {
    escalation::resolve(w, errors);
  } catch (std::exception const& ee) {
    logger(log_verification_error, basic)
        << "Error: Notifier escalation error: " << ee.what();
  }

  // Add errors.
  if (errors) {
    e += errors;
    throw engine_error() << "Cannot resolve service escalation";
  }
}
