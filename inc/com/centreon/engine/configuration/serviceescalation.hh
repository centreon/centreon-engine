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

#ifndef CCE_CONFIGURATION_SERVICEESCALATION_HH
#  define CCE_CONFIGURATION_SERVICEESCALATION_HH

#  include <set>
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/opt.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    serviceescalation : public object {
  public:
    enum                   action_on {
      none = 0,
      unknown = (1 << 1),
      warning = (1 << 2),
      critical = (1 << 3),
      pending = (1 << 4),
      recovery = (1 << 5)
    };
    typedef                serviceescalation
                           key_type;

                           serviceescalation();
                           serviceescalation(
                             serviceescalation const& right);
                           ~serviceescalation() throw ();
    serviceescalation&     operator=(serviceescalation const& right);
    bool                   operator==(
                             serviceescalation const& right) const throw ();
    bool                   operator!=(
                             serviceescalation const& right) const throw ();
    bool                   operator<(
                             serviceescalation const& right) const;
    void                   check_validity() const;
    key_type const&        key() const throw ();
    void                   merge(object const& obj);
    bool                   parse(char const* key, char const* value);

    list_string&           contactgroups() throw ();
    list_string const&     contactgroups() const throw ();
    bool                   contactgroups_defined() const throw ();
    list_string&           contacts() throw ();
    list_string const&     contacts() const throw ();
    bool                   contacts_defined() const throw ();
    void                   escalation_options(
                             unsigned int options) throw ();
    unsigned short         escalation_options() const throw ();
    void                   escalation_period(std::string const& period);
    std::string const&     escalation_period() const throw ();
    bool                   escalation_period_defined() const throw ();
    void                   first_notification(unsigned int n) throw ();
    unsigned int           first_notification() const throw ();
    list_string&           hostgroups() throw ();
    list_string const&     hostgroups() const throw ();
    list_string&           hosts() throw ();
    list_string const&     hosts() const throw ();
    void                   last_notification(
                             unsigned int options) throw ();
    unsigned int           last_notification() const throw ();
    void                   notification_interval(
                             unsigned int interval) throw ();
    unsigned int           notification_interval() const throw ();
    bool                   notification_interval_defined() const throw ();
    list_string&           servicegroups() throw ();
    list_string const&     servicegroups() const throw ();
    list_string&           service_description() throw ();
    list_string const&     service_description() const throw ();

  private:
    struct                 setters {
      char const*          name;
      bool                 (*func)(serviceescalation&, char const*);
    };

    bool                   _set_contactgroups(std::string const& value);
    bool                   _set_contacts(std::string const& value);
    bool                   _set_escalation_options(std::string const& value);
    bool                   _set_escalation_period(std::string const& value);
    bool                   _set_first_notification(unsigned int value);
    bool                   _set_hostgroups(std::string const& value);
    bool                   _set_hosts(std::string const& value);
    bool                   _set_last_notification(unsigned int value);
    bool                   _set_notification_interval(unsigned int value);
    bool                   _set_servicegroups(std::string const& value);
    bool                   _set_service_description(std::string const& value);

    group                  _contactgroups;
    group                  _contacts;
    opt<unsigned short>    _escalation_options;
    opt<std::string>       _escalation_period;
    opt<unsigned int>      _first_notification;
    group                  _hostgroups;
    group                  _hosts;
    opt<unsigned int>      _last_notification;
    opt<unsigned int>      _notification_interval;
    group                  _servicegroups;
    group                  _service_description;
    static setters const   _setters[];
  };

  typedef shared_ptr<serviceescalation>   serviceescalation_ptr;
  typedef std::set<serviceescalation_ptr> set_serviceescalation;
}

CCE_END()

#endif // !CCE_CONFIGURATION_SERVICEESCALATION_HH
