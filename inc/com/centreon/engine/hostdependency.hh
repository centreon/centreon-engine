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

#ifndef CCE_OBJECTS_HOSTDEPENDENCY_HH
#  define CCE_OBJECTS_HOSTDEPENDENCY_HH

#  include <ostream>
#  include <string>
#  include "com/centreon/engine/dependency.hh"
#  include "com/centreon/engine/configuration/hostdependency.hh"

/* Forward declaration. */
CCE_BEGIN()
  class host;
  class hostdependency;
  class timeperiod;
CCE_END()

typedef std::unordered_multimap<std::string,
  std::shared_ptr<com::centreon::engine::hostdependency>> hostdependency_mmap;

CCE_BEGIN()
class                           hostdependency : public dependency {
 public:

                                hostdependency(
                                  std::string const& dependent_hostname,
                                  std::string const& hostname,
                                  dependency::types dependency_type,
                                  bool inherits_parent,
                                  bool fail_on_up,
                                  bool fail_on_down,
                                  bool fail_on_unreachable,
                                  bool fail_on_pending,
                                  std::string const& dependency_period);

  bool                          get_fail_on_up() const;
  void                          set_fail_on_up(bool fail_on_up);
  bool                          get_fail_on_down() const;
  void                          set_fail_on_down(bool fail_on_down);
  bool                          get_fail_on_unreachable() const;
  void                          set_fail_on_unreachable(
                                  bool fail_on_unreachable);

  bool                          check_for_circular_hostdependency_path(
                                  hostdependency* dep,
                                  types dependency_type);
  void resolve(int& w, int& e);
  bool get_fail_on(int state) const override;

  bool operator==(hostdependency const& obj) = delete;
  bool                          operator<(hostdependency const& obj) throw ();

  static hostdependency_mmap    hostdependencies;
  static hostdependency_mmap::iterator
                                hostdependencies_find(configuration::hostdependency const& k);

  host*                         master_host_ptr;
  host*                         dependent_host_ptr;

 private:
  bool                          _fail_on_up;
  bool                          _fail_on_down;
  bool                          _fail_on_unreachable;
};

CCE_END()

std::ostream& operator<<(std::ostream& os,
  com::centreon::engine::hostdependency const& obj);

#endif // !CCE_OBJECTS_HOSTDEPENDENCY_HH
