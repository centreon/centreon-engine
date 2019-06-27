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

serviceescalation::~serviceescalation() {
  logger(logging::dbg_config, logging::more)
    << "Removing a service escalation (destructor).";
  // Notify event broker.
  timeval tv(get_broker_timestamp(nullptr));
  broker_adaptive_escalation_data(NEBTYPE_SERVICEESCALATION_DELETE, NEBFLAG_NONE,
                                  NEBATTR_NONE, this, &tv);
}

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
bool serviceescalation::is_viable(int state, int notification_number) const {
  logger(dbg_functions, basic)
    << "serviceescalation::is_viable()";

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
  }
  else
    return retval;
}

void serviceescalation::resolve(int& w, int& e) {
  (void)w;
  int errors{0};

  // Find the service.
  service_map::const_iterator found{service::services.find(
    {get_hostname(), get_description()})};
  if (found == service::services.end() || !found->second) {
    logger(log_verification_error, basic) << "Error: Service '"
        << this->get_description() << "' on host '" << this->get_hostname()
        << "' specified in service escalation is not defined anywhere!";
    errors++;
    notifier_ptr = nullptr;
  }
  else
    notifier_ptr = found->second.get();

  try {
    escalation::resolve(w, errors);
  }
  catch (std::exception const& ee) {
    logger(log_verification_error, basic)
      << "Error: Notifier escalation error: " << ee.what();
  }

  // Add errors.
  if (errors) {
    e += errors;
    throw engine_error() << "Cannot resolve service escalation";
  }
}
