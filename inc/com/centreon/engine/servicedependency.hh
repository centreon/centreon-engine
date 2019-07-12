/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_OBJECTS_SERVICEDEPENDENCY_HH
#  define CCE_OBJECTS_SERVICEDEPENDENCY_HH
#  include "com/centreon/engine/dependency.hh"
#  include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/configuration/servicedependency.hh"

/* Forward declaration. */
CCE_BEGIN()
class service;
class servicedependency;
class timeperiod;
CCE_END()

typedef std::unordered_multimap<std::pair<std::string, std::string>,
  std::shared_ptr<com::centreon::engine::servicedependency>, pair_hash> servicedependency_mmap;

CCE_BEGIN()
class                 servicedependency : public dependency {
 public:
                      servicedependency(
                        std::string const& dependent_host_name,
                        std::string const& dependent_service_description,
                        std::string const& host_name,
                        std::string const& service_description,
                        dependency::types dependency_type,
                        bool inherits_parent,
                        bool fail_on_ok,
                        bool fail_on_warning,
                        bool fail_on_unknown,
                        bool fail_on_critical,
                        bool fail_on_pending,
                        std::string const& dependency_period);

  std::string const & get_dependent_service_description() const;
  void                set_dependent_service_description(
    std::string const& dependent_service_desciption);
  std::string const & get_service_description() const;
  void                set_service_description(
    std::string const& service_description);
  bool                get_fail_on_ok() const;
  void                set_fail_on_ok(bool fail_on_ok);
  bool                get_fail_on_warning() const;
  void                set_fail_on_warning(bool fail_on_warning);
  bool                get_fail_on_unknown() const;
  void                set_fail_on_unknown(bool fail_on_unknown);
  bool                get_fail_on_critical() const;
  void                set_fail_on_critical(bool fail_on_critical);

  bool                check_for_circular_servicedependency_path(
                        servicedependency* dep,
                        types dependency_type);
  void resolve(int& w, int& e);
  bool get_fail_on(int state) const override;

  bool operator==(servicedependency const& obj) = delete;

  service*            master_service_ptr;
  service*            dependent_service_ptr;

  static servicedependency_mmap
                      servicedependencies;
  static servicedependency_mmap::iterator servicedependencies_find(
      configuration::servicedependency const& k);

 private:
  std::string         _dependent_service_description;
  std::string         _service_description;
  bool                _fail_on_ok;
  bool                _fail_on_warning;
  bool                _fail_on_unknown;
  bool                _fail_on_critical;
};

CCE_END();

std::ostream& operator<<(std::ostream& os,
  com::centreon::engine::servicedependency const& obj);

#endif // !CCE_OBJECTS_SERVICEDEPENDENCY_HH


