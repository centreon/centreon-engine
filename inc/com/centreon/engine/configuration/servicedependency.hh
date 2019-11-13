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

#ifndef CCE_CONFIGURATION_SERVICEDEPENDENCY_HH
#define CCE_CONFIGURATION_SERVICEDEPENDENCY_HH

#include <memory>
#include <set>
#include "com/centreon/engine/configuration/group.hh"
#include "com/centreon/engine/configuration/object.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/opt.hh"

CCE_BEGIN()

namespace configuration {
class servicedependency : public object {
 public:
  enum action_on {
    none = 0,
    ok = (1 << 0),
    unknown = (1 << 1),
    warning = (1 << 2),
    critical = (1 << 3),
    pending = (1 << 4)
  };
  enum dependency_kind {
    unknown_type = 0,
    notification_dependency,
    execution_dependency
  };
  typedef servicedependency key_type;

  servicedependency();
  servicedependency(servicedependency const& right);
  ~servicedependency() throw() override;
  servicedependency& operator=(servicedependency const& right);
  bool operator==(servicedependency const& right) const throw();
  bool operator!=(servicedependency const& right) const throw();
  bool operator<(servicedependency const& right) const;
  void check_validity() const override;
  key_type const& key() const throw();
  void merge(object const& obj) override;
  bool parse(char const* key, char const* value) override;

  void dependency_period(std::string const& period);
  std::string const& dependency_period() const throw();
  void dependency_type(dependency_kind type) throw();
  dependency_kind dependency_type() const throw();
  list_string& dependent_hostgroups() throw();
  list_string const& dependent_hostgroups() const throw();
  list_string& dependent_hosts() throw();
  list_string const& dependent_hosts() const throw();
  list_string& dependent_servicegroups() throw();
  list_string const& dependent_servicegroups() const throw();
  list_string& dependent_service_description() throw();
  list_string const& dependent_service_description() const throw();
  void execution_failure_options(unsigned int options) throw();
  unsigned int execution_failure_options() const throw();
  void inherits_parent(bool inherit) throw();
  bool inherits_parent() const throw();
  list_string& hostgroups() throw();
  list_string const& hostgroups() const throw();
  list_string& hosts() throw();
  list_string const& hosts() const throw();
  void notification_failure_options(unsigned int options) throw();
  unsigned int notification_failure_options() const throw();
  list_string& servicegroups() throw();
  list_string const& servicegroups() const throw();
  list_string& service_description() throw();
  list_string const& service_description() const throw();

 private:
  typedef bool (*setter_func)(servicedependency&, char const*);

  bool _set_dependency_period(std::string const& value);
  bool _set_dependent_hostgroups(std::string const& value);
  bool _set_dependent_hosts(std::string const& value);
  bool _set_dependent_servicegroups(std::string const& value);
  bool _set_dependent_service_description(std::string const& value);
  bool _set_execution_failure_options(std::string const& value);
  bool _set_inherits_parent(bool value);
  bool _set_hostgroups(std::string const& value);
  bool _set_hosts(std::string const& value);
  bool _set_notification_failure_options(std::string const& value);
  bool _set_servicegroups(std::string const& value);
  bool _set_service_description(std::string const& value);

  std::string _dependency_period;
  dependency_kind _dependency_type;
  group<list_string> _dependent_hostgroups;
  group<list_string> _dependent_hosts;
  group<list_string> _dependent_servicegroups;
  group<list_string> _dependent_service_description;
  opt<unsigned int> _execution_failure_options;
  group<list_string> _hostgroups;
  group<list_string> _hosts;
  opt<bool> _inherits_parent;
  opt<unsigned int> _notification_failure_options;
  group<list_string> _servicegroups;
  group<list_string> _service_description;
  static std::unordered_map<std::string, setter_func> const _setters;
};

typedef std::shared_ptr<servicedependency> servicedependency_ptr;
typedef std::set<servicedependency> set_servicedependency;
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_SERVICEDEPENDENCY_HH
