/*
** Copyright 2011-2013,2017 Centreon
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

#ifndef CCE_CONFIGURATION_HOSTESCALATION_HH
#define CCE_CONFIGURATION_HOSTESCALATION_HH

#include <memory>
#include <set>
#include "com/centreon/engine/configuration/group.hh"
#include "com/centreon/engine/configuration/object.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/opt.hh"
#include "com/centreon/engine/shared.hh"

CCE_BEGIN()

namespace configuration {
class hostescalation : public object {
 public:
  enum action_on {
    none = 0,
    down = (1 << 0),
    unreachable = (1 << 1),
    recovery = (1 << 2)
  };
  typedef hostescalation key_type;

  hostescalation();
  hostescalation(hostescalation const& right);
  ~hostescalation() throw() override;
  hostescalation& operator=(hostescalation const& right);
  bool operator==(hostescalation const& right) const throw();
  bool operator!=(hostescalation const& right) const throw();
  bool operator<(hostescalation const& right) const;
  void check_validity() const override;
  key_type const& key() const throw();
  void merge(object const& obj) override;
  bool parse(char const* key, char const* value) override;

  set_string& contactgroups() throw();
  set_string const& contactgroups() const throw();
  bool contactgroups_defined() const throw();
  void escalation_options(unsigned short options) throw();
  unsigned short escalation_options() const throw();
  void escalation_period(std::string const& period);
  std::string const& escalation_period() const throw();
  bool escalation_period_defined() const throw();
  void first_notification(unsigned int n) throw();
  uint32_t first_notification() const throw();
  set_string& hostgroups() throw();
  set_string const& hostgroups() const throw();
  set_string& hosts() throw();
  set_string const& hosts() const throw();
  void last_notification(unsigned int n) throw();
  unsigned int last_notification() const throw();
  void notification_interval(unsigned int interval);
  unsigned int notification_interval() const throw();
  bool notification_interval_defined() const throw();
  Uuid const& uuid() const;

 private:
  typedef bool (*setter_func)(hostescalation&, char const*);

  bool _set_contactgroups(std::string const& value);
  bool _set_escalation_options(std::string const& value);
  bool _set_escalation_period(std::string const& value);
  bool _set_first_notification(unsigned int value);
  bool _set_hostgroups(std::string const& value);
  bool _set_hosts(std::string const& value);
  bool _set_last_notification(unsigned int value);
  bool _set_notification_interval(unsigned int value);

  group<set_string> _contactgroups;
  opt<unsigned short> _escalation_options;
  opt<std::string> _escalation_period;
  opt<unsigned int> _first_notification;
  group<set_string> _hostgroups;
  group<set_string> _hosts;
  opt<unsigned int> _last_notification;
  opt<unsigned int> _notification_interval;
  static std::unordered_map<std::string, setter_func> const _setters;
  Uuid _uuid;
};

typedef std::shared_ptr<hostescalation> hostescalation_ptr;
typedef std::set<hostescalation> set_hostescalation;
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_HOSTESCALATION_HH
