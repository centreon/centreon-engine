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

/* Forward declaration. */
CCE_BEGIN()
  class host;
  class hostdependency;
  class timeperiod;
CCE_END()

typedef std::unordered_multimap<std::string,
  std::shared_ptr<com::centreon::engine::hostdependency>> hostdependency_mmap;

CCE_BEGIN()
class                           hostdependency {
 public:
    enum                        types {
      notification = 1,
      execution
    };

                                hostdependency(
                                  std::string const& dependent_host_name,
                                  std::string const& host_name,
                                  types dependency_type,
                                  bool inherits_parent,
                                  bool fail_on_up,
                                  bool fail_on_down,
                                  bool fail_on_unreachable,
                                  bool fail_on_pending,
                                  std::string const& dependency_period);

  types                         get_dependency_type() const;
  void                          set_dependency_type(types dependency_type);
  std::string const&            get_dependent_host_name() const;
  void                          set_dependent_host_name(
                                  std::string const& dependent_host_name);
  std::string const&            get_host_name() const;
  void                          set_host_name(std::string const& host_name);
  std::string const&            get_dependency_period() const;
  void                          set_dependency_period(
                                  std::string const& dependency_period);
  bool                          get_inherits_parent() const;
  void                          set_inherits_parent(bool inherits_parent);
  bool                          get_fail_on_up() const;
  void                          set_fail_on_up(bool fail_on_up);
  bool                          get_fail_on_down() const;
  void                          set_fail_on_down(bool fail_on_down);
  bool                          get_fail_on_unreachable() const;
  void                          set_fail_on_unreachable(
                                  bool fail_on_unreachable);
  bool                          get_fail_on_pending() const;
  void                          set_fail_on_pending(bool fail_on_pending);
  bool                          get_circular_path_checked() const;
  void                          set_circular_path_checked(
                                  bool circular_path_checked);
  bool                          get_contains_circular_path() const;
  void                          set_contains_circular_path(
                                  bool contains_circular_path);

  bool                          check_for_circular_hostdependency_path(
                                  hostdependency* dep,
                                  types dependency_type);

  static hostdependency_mmap    hostdependencies;

  com::centreon::engine::host*  master_host_ptr;
  com::centreon::engine::host*  dependent_host_ptr;
  com::centreon::engine::timeperiod*
                                dependency_period_ptr;

  bool                          operator==(
    com::centreon::engine::hostdependency const& obj) throw ();
  bool                          operator!=(
    com::centreon::engine::hostdependency const& obj) throw ();
  bool                          operator<(
    com::centreon::engine::hostdependency const& obj) throw ();

 private:
  types                         _dependency_type;
  std::string                   _dependent_host_name;
  std::string                   _host_name;
  std::string                   _dependency_period;
  int                           _inherits_parent;
  int                           _fail_on_up;
  int                           _fail_on_down;
  int                           _fail_on_unreachable;
  int                           _fail_on_pending;
  int                           _circular_path_checked;
  int                           _contains_circular_path;
};

CCE_END()


std::ostream& operator<<(std::ostream& os, com::centreon::engine::hostdependency const& obj);

#endif // !CCE_OBJECTS_HOSTDEPENDENCY_HH


