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
#include "com/centreon/engine/hostescalation.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

hostescalation_mmap hostescalation::hostescalations;

/**
 *  Create a new host escalation.
 *
 *  @param[in] host_name               Host name.
 *  @param[in] first_notification      First notification.
 *  @param[in] last_notification       Last notification.
 *  @param[in] notification_interval   Notification interval.
 *  @param[in] escalation_period       Escalation timeperiod name.
 *  @param[in] escalate_on_down        Escalate on down ?
 *  @param[in] escalate_on_unreachable Escalate on unreachable ?
 *  @param[in] escalate_on_recovery    Escalate on recovery ?
 */
hostescalation::hostescalation(std::string const& host_name,
                               int first_notification,
                               int last_notification,
                               double notification_interval,
                               std::string const& escalation_period,
                               uint32_t escalate_on)
    : escalation{first_notification, last_notification, notification_interval,
                 escalation_period, escalate_on},
      _hostname{host_name} {
  if (host_name.empty())
    throw engine_error() << "Could not create escalation "
                         << "on host '" << host_name << "'";
}

hostescalation::~hostescalation() {
  logger(logging::dbg_config, logging::more)
    << "Removing a host escalation (destructor).";
  // Notify event broker.
  timeval tv(get_broker_timestamp(nullptr));
  broker_adaptive_escalation_data(NEBTYPE_HOSTESCALATION_DELETE, NEBFLAG_NONE,
                                  NEBATTR_NONE, this, &tv);
}

std::string const& hostescalation::get_hostname() const {
  return _hostname;
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
bool hostescalation::is_viable(int state, int notification_number) const {
  logger(dbg_functions, basic)
    << "serviceescalation::is_viable()";

  bool retval{escalation::is_viable(state, notification_number)};
  if (retval) {
    std::array<notifier::notification_type, 3> nt = {
      notifier::up,
      notifier::down,
      notifier::unreachable,
    };

    if (!get_escalate_on(nt[state]))
      return false;
    return true;
  }
  else
    return retval;
}

void hostescalation::resolve(int& w, int& e) {
  (void)w;
  int errors{0};

  // Find the host.
  host_map::const_iterator it(host::hosts.find(this->get_hostname()));
  if (it == host::hosts.end() || !it->second) {
    logger(log_verification_error, basic)
      << "Error: Host '" << this->get_hostname()
      << "' specified in host escalation is not defined anywhere!";
    errors++;
    notifier_ptr = nullptr;
  }
  else
    notifier_ptr = it->second.get();

  try {
    escalation::resolve(w, errors);
  }
  catch (std::exception const& ee) {
    logger(log_verification_error, basic)
      << "Error: Notifier escalation error: " << ee.what();
  }

  // Add errors.
  e += errors;
  if (errors)
    throw engine_error() << "Cannot resolve host escalation";
}
