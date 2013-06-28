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

#ifndef CCE_CONFIGURATION_SERVICEDEPENDENCY_HH
#  define CCE_CONFIGURATION_SERVICEDEPENDENCY_HH

#  include <list>
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/configuration/opt.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    servicedependency
    : public object {
  public:
    enum                   action_on {
      none = 0,
      ok = (1 << 0),
      unknown = (1 << 1),
      warning = (1 << 2),
      critical = (1 << 3),
      pending = (1 << 4)
    };

                           servicedependency();
                           servicedependency(
                             servicedependency const& right);
                           ~servicedependency() throw ();
    servicedependency&     operator=(servicedependency const& right);
    bool                   operator==(
                             servicedependency const& right) const throw ();
    bool                   operator!=(
                             servicedependency const& right) const throw ();
    void                   check_validity() const;
    std::size_t            id() const throw ();
    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

    std::string const&     dependency_period() const throw ();
    list_string const&     dependent_hostgroups() const throw ();
    list_string const&     dependent_hosts() const throw ();
    list_string const&     dependent_servicegroups() const throw ();
    list_string const&     dependent_service_description() const throw ();
    unsigned int           execution_failure_options() const throw ();
    bool                   inherits_parent() const throw ();
    list_string const&     hostgroups() const throw ();
    list_string const&     hosts() const throw ();
    unsigned int           notification_failure_options() const throw ();
    list_string const&     servicegroups() const throw ();
    list_string const&     service_description() const throw ();

  private:
    bool                   _set_dependency_period(std::string const& value);
    bool                   _set_dependent_hostgroups(std::string const& value);
    bool                   _set_dependent_hosts(std::string const& value);
    bool                   _set_dependent_servicegroups(std::string const& value);
    bool                   _set_dependent_service_description(std::string const& value);
    bool                   _set_execution_failure_options(std::string const& value);
    bool                   _set_inherits_parent(bool value);
    bool                   _set_hostgroups(std::string const& value);
    bool                   _set_hosts(std::string const& value);
    bool                   _set_notification_failure_options(std::string const& value);
    bool                   _set_servicegroups(std::string const& value);
    bool                   _set_service_description(std::string const& value);

    std::string            _dependency_period;
    group                  _dependent_hostgroups;
    group                  _dependent_hosts;
    group                  _dependent_servicegroups;
    group                  _dependent_service_description;
    opt<unsigned int>      _execution_failure_options;
    opt<bool>              _inherits_parent;
    group                  _hostgroups;
    group                  _hosts;
    opt<unsigned int>      _notification_failure_options;
    group                  _servicegroups;
    group                  _service_description;
  };

  typedef shared_ptr<servicedependency>    servicedependency_ptr;
  typedef std::list<servicedependency_ptr> list_servicedependency;
}

CCE_END()

#endif // !CCE_CONFIGURATION_SERVICEDEPENDENCY_HH
