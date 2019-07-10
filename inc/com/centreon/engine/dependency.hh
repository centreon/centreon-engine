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

#ifndef CENTREON_ENGINE_DEPENDENCY_HH
#define CENTREON_ENGINE_DEPENDENCY_HH

#  include <ostream>
#  include <string>
#  include <com/centreon/engine/namespace.hh>
#  include <com/centreon/engine/timeperiod.hh>

CCE_BEGIN();

class                         dependency {

 public:
  enum                        types {
    notification = 1,
    execution
  };

                                dependency(std::string const& dependent_hostname,
                                           std::string const& hostname,
                                           types dependency_type,
                                           bool inherits_parent,
                                           bool fail_on_pending,
                                           std::string const& dependency_period);
  virtual                       ~dependency() {}

  types                         get_dependency_type() const;
  void                          set_dependency_type(types dependency_type);
  std::string const&            get_dependent_hostname() const;
  void                          set_dependent_hostname(
    std::string const& dependent_host_name);
  std::string const&            get_hostname() const;
  void                          set_hostname(std::string const& hostname);
  std::string const&            get_dependency_period() const;
  void                          set_dependency_period(
    std::string const& dependency_period);
  bool                          get_inherits_parent() const;
  void                          set_inherits_parent(bool inherits_parent);
  bool                          get_fail_on_pending() const;
  void                          set_fail_on_pending(bool fail_on_pending);
  bool                          get_circular_path_checked() const;
  void                          set_circular_path_checked(
    bool circular_path_checked);
  bool                          get_contains_circular_path() const;
  void                          set_contains_circular_path(
      bool contains_circular_path);
  virtual bool get_fail_on(int state) const = 0;

  virtual bool                  operator==(dependency const& obj) throw ();
  bool                          operator!=(dependency const& obj) throw ();
  virtual bool                  operator<(dependency const& obj) throw ();

  com::centreon::engine::timeperiod*
                                dependency_period_ptr;

 protected:
  types                         _dependency_type;
  std::string                   _dependent_hostname;
  std::string                   _hostname;
  std::string                   _dependency_period;
  bool                          _inherits_parent;
  bool                          _fail_on_pending;
  bool                          _circular_path_checked;
  bool                          _contains_circular_path;

};

CCE_END();

#endif //CENTREON_ENGINE_DEPENDENCY_HH
