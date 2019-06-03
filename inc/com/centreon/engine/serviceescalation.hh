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

#ifndef CCE_SERVICEESCALATION_HH
#  define CCE_SERVICEESCALATION_HH

#include <string>
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/escalation.hh"
#include "com/centreon/engine/namespace.hh"

/* Forward declaration. */
CCE_BEGIN()
  class service;
  class timeperiod;

class                              serviceescalation : public escalation {
 public:
                                   serviceescalation(
                                       std::string const& hostname,
                                       std::string const& description,
                                       int first_notification,
                                       int last_notification,
                                       double notification_interval,
                                       std::string const& escalation_period);
                                   ~serviceescalation();
  std::string const&               get_hostname() const;
  std::string const&               get_description() const;
  int                              escalate_on_recovery;
  int                              escalate_on_warning;
  int                              escalate_on_unknown;
  int                              escalate_on_critical;
  contactgroup_map                 contact_groups;
  contact_map                      contacts;
  service*                         service_ptr;
  timeperiod*                      escalation_period_ptr;
  serviceescalation*               next;
  serviceescalation*               nexthash;
 private:
  std::string                      _hostname;
  std::string                      _description;
};
CCE_END()

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

com::centreon::engine::serviceescalation* add_service_escalation(
    std::string const& host_name,
    std::string const& description,
    int first_notification,
    int last_notification,
    double notification_interval,
    std::string const& escalation_period,
    int escalate_on_warning,
    int escalate_on_unknown,
    int escalate_on_critical,
    int escalate_on_recovery);

#  ifdef __cplusplus
}

#    include <ostream>

bool operator==(com::centreon::engine::serviceescalation const& obj1,
                com::centreon::engine::serviceescalation const& obj2) throw();
bool          operator!=(
                com::centreon::engine::serviceescalation const& obj1,
                com::centreon::engine::serviceescalation const& obj2) throw ();
bool          operator<(
                com::centreon::engine::serviceescalation const& obj1,
                com::centreon::engine::serviceescalation const& obj2) throw ();
std::ostream& operator<<(
                std::ostream& os,
                com::centreon::engine::serviceescalation const& obj);

#  endif /* C++ */

#endif // !CCE_SERVICEESCALATION_HH
