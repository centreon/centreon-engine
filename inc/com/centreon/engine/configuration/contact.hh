/*
** Copyright 2011-2013,2015,2017 Centreon
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

#ifndef CCE_CONFIGURATION_CONTACT_HH
#  define CCE_CONFIGURATION_CONTACT_HH

#  include <set>
#  include <string>
#  include <vector>
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/customvariable.hh"
#  include "com/centreon/engine/opt.hh"
#  include "com/centreon/engine/namespace.hh"

typedef std::vector<std::string> tab_string;

CCE_BEGIN()

namespace                  configuration {
  class                    contact : public object {
   public:
    typedef std::string    key_type;

                           contact(key_type const& key = "");
                           contact(contact const& other);
                           ~contact() throw () override;
    contact&               operator=(contact const& other);
    bool                   operator==(
                             contact const& other) const throw ();
    bool                   operator!=(
                             contact const& other) const throw ();
    bool                   operator<(
                             contact const& other) const throw ();
    void                   check_validity() const override;
    key_type const&        key() const throw ();
    void                   merge(object const& obj) override;
    bool                   parse(char const* key, char const* value) override;

    tab_string const&      address() const throw ();
    std::string const&     alias() const throw ();
    bool                   can_submit_commands() const throw ();
    set_string&            contactgroups() throw ();
    set_string const&      contactgroups() const throw ();
    std::string const&     contact_name() const throw ();
    map_customvar const&   customvariables() const throw ();
    std::string const&     email() const throw ();
    bool                   host_notifications_enabled() const throw ();
    list_string const&     host_notification_commands() const throw ();
    unsigned int           host_notification_options() const throw ();
    std::string const&     host_notification_period() const throw ();
    bool                   retain_nonstatus_information() const throw ();
    bool                   retain_status_information() const throw ();
    std::string const&     pager() const throw ();
    list_string const&     service_notification_commands() const throw ();
    unsigned int           service_notification_options() const throw ();
    std::string const&     service_notification_period() const throw ();
    bool                   service_notifications_enabled() const throw ();
    std::string const&     timezone() const throw ();

   private:
    typedef bool (*setter_func)(contact&, char const*);

    bool                   _set_address(
                             std::string const& key,
                             std::string const& value);
    bool                   _set_alias(std::string const& value);
    bool                   _set_can_submit_commands(bool value);
    bool                   _set_contactgroups(std::string const& value);
    bool                   _set_contact_name(std::string const& value);
    bool                   _set_email(std::string const& value);
    bool                   _set_host_notifications_enabled(bool value);
    bool                   _set_host_notification_commands(std::string const& value);
    bool                   _set_host_notification_options(std::string const& value);
    bool                   _set_host_notification_period(std::string const& value);
    bool                   _set_retain_nonstatus_information(bool value);
    bool                   _set_retain_status_information(bool value);
    bool                   _set_pager(std::string const& value);
    bool                   _set_service_notification_commands(std::string const& value);
    bool                   _set_service_notification_options(std::string const& value);
    bool                   _set_service_notification_period(std::string const& value);
    bool                   _set_service_notifications_enabled(bool value);
    bool                   _set_timezone(std::string const& value);

    tab_string             _address;
    std::string            _alias;
    opt<bool>              _can_submit_commands;
    group<set_string>      _contactgroups;
    std::string            _contact_name;
    map_customvar          _customvariables;
    std::string            _email;
    opt<bool>              _host_notifications_enabled;
    group<list_string>     _host_notification_commands;
    opt<unsigned int>      _host_notification_options;
    std::string            _host_notification_period;
    opt<bool>              _retain_nonstatus_information;
    opt<bool>              _retain_status_information;
    std::string            _pager;
    group<list_string>     _service_notification_commands;
    opt<unsigned int>      _service_notification_options;
    std::string            _service_notification_period;
    opt<bool>              _service_notifications_enabled;
    opt<std::string>       _timezone;
    static std::unordered_map<std::string, setter_func> const _setters;
  };

  typedef std::shared_ptr<contact> contact_ptr;
  typedef std::set<contact>        set_contact;
}

CCE_END()

#endif // !CCE_CONFIGURATION_CONTACT_HH
