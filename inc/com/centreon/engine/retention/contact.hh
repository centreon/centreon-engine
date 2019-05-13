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

#ifndef CCE_RETENTION_CONTACT_HH
#  define CCE_RETENTION_CONTACT_HH

#  include <ctime>
#  include <list>
#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/customvariable.hh"
#  include "com/centreon/engine/opt.hh"
#  include "com/centreon/engine/retention/object.hh"

CCE_BEGIN()

namespace                     retention {
  class                       contact : public object {
  public:
                              contact();
                              contact(contact const& right);
                              ~contact() throw () override;
    contact&                  operator=(contact const& right);
    bool                      operator==(contact const& right) const throw ();
    bool                      operator!=(contact const& right) const throw ();
    bool                      set(char const* key, char const* value) override;

    std::string const&        contact_name() const throw ();
    map_customvar const&      customvariables() const throw ();
    opt<std::string> const&   host_notification_period() const throw ();
    opt<bool> const&          host_notifications_enabled() const throw ();
    opt<time_t> const&        last_host_notification() const throw ();
    opt<time_t> const&        last_service_notification() const throw ();
    opt<unsigned long> const& modified_attributes() const throw ();
    opt<unsigned long> const& modified_host_attributes() const throw ();
    opt<unsigned long> const& modified_service_attributes() const throw ();
    opt<std::string> const&   service_notification_period() const throw ();
    opt<bool> const&          service_notifications_enabled() const throw ();

  private:
    struct                    setters {
      char const*             name;
      bool                    (*func)(contact&, char const*);
    };

    bool                      _set_contact_name(std::string const& value);
    bool                      _set_host_notification_period(std::string const& value);
    bool                      _set_host_notifications_enabled(bool value);
    bool                      _set_last_host_notification(time_t value);
    bool                      _set_last_service_notification(time_t value);
    bool                      _set_modified_attributes(unsigned long value);
    bool                      _set_modified_host_attributes(unsigned long value);
    bool                      _set_modified_service_attributes(unsigned long value);
    bool                      _set_service_notification_period(std::string const& value);
    bool                      _set_service_notifications_enabled(bool value);

    std::string               _contact_name;
    map_customvar             _customvariables;
    opt<std::string>          _host_notification_period;
    opt<bool>                 _host_notifications_enabled;
    opt<time_t>               _last_host_notification;
    opt<time_t>               _last_service_notification;
    opt<unsigned long>        _modified_attributes;
    opt<unsigned long>        _modified_host_attributes;
    opt<unsigned long>        _modified_service_attributes;
    opt<std::string>          _service_notification_period;
    opt<bool>                 _service_notifications_enabled;
    static setters const      _setters[];
  };

  typedef std::shared_ptr<contact> contact_ptr;
  typedef std::list<contact_ptr>   list_contact;
}

CCE_END()

#endif // !CCE_RETENTION_CONTACT_HH

