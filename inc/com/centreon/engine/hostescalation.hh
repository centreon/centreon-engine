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

#ifndef CCE_HOSTESCALATION_HH
#  define CCE_HOSTESCALATION_HH
#  include <ostream>
#  include "com/centreon/engine/escalation.hh"

/* Forward declaration. */
CCE_BEGIN()
class host;
class hostescalation;
CCE_END()

typedef std::unordered_multimap<std::string,
std::shared_ptr<com::centreon::engine::hostescalation>> hostescalation_mmap;
typedef std::unordered_multimap<std::string, com::centreon::engine::hostescalation*> hostescalation_mmap_unsafe;

CCE_BEGIN()
class hostescalation : public escalation {
 public:
  hostescalation(std::string const& host_name,
                 int first_notification,
                 int last_notification,
                 double notification_interval,
                 std::string const& escalation_period,
                 uint32_t escalate_on);
  virtual ~hostescalation();

  std::string const& get_hostname() const;
  bool is_viable(int state, int notification_number) const override;
  void resolve(int& w, int& e) override;

  static hostescalation_mmap hostescalations;

 private:
  std::string _hostname;
};

CCE_END()

#endif  // !CCE_HOSTESCALATION_HH
