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
#define CCE_SERVICEESCALATION_HH

#include <ostream>
#include <string>

#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/escalation.hh"
#include "com/centreon/engine/namespace.hh"

/* Forward declaration. */
CCE_BEGIN()
class service;
class timeperiod;

class serviceescalation : public escalation {
 public:
  serviceescalation(std::string const& hostname,
                    std::string const& description,
                    int first_notification,
                    int last_notification,
                    double notification_interval,
                    std::string const& escalation_period,
                    uint32_t escalate_on);
  std::string const& get_hostname() const;
  std::string const& get_description() const;

  contactgroup_map contact_groups;
  service* service_ptr;
  timeperiod* escalation_period_ptr;
  serviceescalation* next;
  serviceescalation* nexthash;

 private:
  std::string _hostname;
  std::string _description;
};

CCE_END()

#endif  // !CCE_SERVICEESCALATION_HH
