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
struct timeperiod;
CCE_END()

typedef std::unordered_multimap<std::string,
std::shared_ptr<com::centreon::engine::hostescalation>> hostescalation_mmap;

CCE_BEGIN()
class                 hostescalation : public escalation {
 public:
                      hostescalation(std::string const& host_name,
                                     int first_notification,
                                     int last_notification,
                                     double notification_interval,
                                     std::string const& escalation_period,
                                     bool escalate_on_down,
                                     bool escalate_on_unreachable,
                                     bool escalate_on_recovery);

  std::string const&  get_hostname() const;
  void                set_hostname(std::string const& host_name);
  std::string const&  get_escalation_period() const;
  void                set_escalation_period(
    std::string const& escalation_period);
  bool                get_escalate_on_recovery() const;
  void                set_escalate_on_recovery(bool escalate_on_recovery);
  bool                get_escalate_on_down() const;
  void                set_escalate_on_down(bool escalate_on_down);
  bool                get_escalate_on_unreachable() const;
  void                set_escalate_on_unreachable(bool escalate_on_unreachable);

  bool                operator==(
    com::centreon::engine::hostescalation const& obj) throw ();
  bool                operator!=(
    com::centreon::engine::hostescalation const& obj) throw();
  bool                operator<(
    com::centreon::engine::hostescalation const& obj);

  contactgroup_map    contact_groups;
  contact_map         contacts;
  host*               host_ptr;
  timeperiod*         escalation_period_ptr;
  static hostescalation_mmap
                      hostescalations;

 private:
  std::string         _host_name;
  std::string         _escalation_period;
  bool                _escalate_on_recovery;
  bool                _escalate_on_down;
  bool                _escalate_on_unreachable;
};
CCE_END()

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */



#  ifdef __cplusplus
}

std::ostream& operator<<(std::ostream& os,
  com::centreon::engine::hostescalation const& obj);

#  endif /* C++ */

#endif // !CCE_HOSTESCALATION_HH


