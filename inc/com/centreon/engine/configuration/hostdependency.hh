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

#ifndef CCE_CONFIGURATION_HOSTDEPENDENCY_HH
#  define CCE_CONFIGURATION_HOSTDEPENDENCY_HH

#  include <memory>
#  include <set>
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/opt.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    hostdependency : public object {
   public:
    enum                   action_on {
      none = 0,
      up = (1 << 0),
      down = (1 << 1),
      unreachable = (1 << 2),
      pending = (1 << 3)
    };
    enum                   dependency_kind {
      unknown = 0,
      notification_dependency,
      execution_dependency
    };
    typedef hostdependency key_type;

                           hostdependency();
                           hostdependency(hostdependency const& right);
                           ~hostdependency() throw () override;
    hostdependency&        operator=(hostdependency const& right);
    bool                   operator==(
                             hostdependency const& right) const throw ();
    bool                   operator!=(
                             hostdependency const& right) const throw ();
    bool                   operator<(hostdependency const& right) const;
    void                   check_validity() const override;
    key_type const&        key() const throw ();
    void                   merge(object const& obj) override;
    bool                   parse(char const* key, char const* value) override;

    void                   dependency_period(std::string const& period);
    std::string const&     dependency_period() const throw ();
    void                   dependency_type(
                             dependency_kind type) throw ();
    dependency_kind        dependency_type() const throw ();
    set_string&            dependent_hostgroups() throw ();
    set_string const&      dependent_hostgroups() const throw ();
    set_string&            dependent_hosts() throw ();
    set_string const&      dependent_hosts() const throw ();
    void                   execution_failure_options(
                             unsigned int options) throw ();
    unsigned int           execution_failure_options() const throw ();
    set_string&            hostgroups() throw ();
    set_string const&      hostgroups() const throw ();
    set_string&            hosts() throw ();
    set_string const&      hosts() const throw ();
    void                   inherits_parent(bool inherit) throw ();
    bool                   inherits_parent() const throw ();
    void                   notification_failure_options(
                             unsigned int options) throw ();
    unsigned int           notification_failure_options() const throw ();

   private:
    typedef bool (*setter_func)(hostdependency&, char const*);

    bool                   _set_dependency_period(std::string const& value);
    bool                   _set_dependent_hostgroups(std::string const& value);
    bool                   _set_dependent_hosts(std::string const& value);
    bool                   _set_execution_failure_options(std::string const& value);
    bool                   _set_hostgroups(std::string const& value);
    bool                   _set_hosts(std::string const& value);
    bool                   _set_inherits_parent(bool value);
    bool                   _set_notification_failure_options(std::string const& value);

    std::string            _dependency_period;
    dependency_kind        _dependency_type;
    group<set_string>      _dependent_hostgroups;
    group<set_string>      _dependent_hosts;
    opt<unsigned int>      _execution_failure_options;
    group<set_string>      _hostgroups;
    group<set_string>      _hosts;
    opt<bool>              _inherits_parent;
    opt<unsigned int>      _notification_failure_options;
    static std::unordered_map<std::string, setter_func> const _setters;
  };

  typedef std::shared_ptr<hostdependency>  hostdependency_ptr;
  typedef std::set<hostdependency>         set_hostdependency;
}

CCE_END()

#endif // !CCE_CONFIGURATION_HOSTDEPENDENCY_HH
