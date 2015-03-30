/*
** Copyright 2011-2015 Merethis
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

#ifndef CCE_CONFIGURATION_HOST_HH
#  define CCE_CONFIGURATION_HOST_HH

#  include <list>
#  include <set>
#  include "com/centreon/engine/common.hh"
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/objects/customvariable.hh"
#  include "com/centreon/engine/opt.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    host : public object {
  public:
    enum                   action_on {
      none = 0,
      up = (1 << 0),
      down = (1 << 1),
      unreachable = (1 << 2),
      flapping = (1 << 3)
    };
    typedef std::string    key_type;

                           host(key_type const& key = "");
                           host(host const& right);
                           ~host() throw ();
    host&                  operator=(host const& right);
    bool                   operator==(host const& right) const throw ();
    bool                   operator!=(host const& right) const throw ();
    bool                   operator<(host const& right) const throw ();
    void                   check_validity() const;
    key_type const&        key() const throw ();
    void                   merge(object const& obj);
    bool                   parse(char const* key, char const* value);

    std::string const&     address() const throw ();
    std::string const&     alias() const throw ();
    bool                   checks_active() const throw ();
    std::string const&     check_command() const throw ();
    bool                   check_freshness() const throw ();
    unsigned int           check_interval() const throw ();
    std::string const&     check_period() const throw ();
    unsigned int           check_timeout() const throw();
    bool                   check_timeout_defined() const throw();
    map_customvar const&   customvariables() const throw ();
    std::string const&     event_handler() const throw ();
    bool                   event_handler_enabled() const throw ();
    bool                   flap_detection_enabled() const throw ();
    unsigned int           flap_detection_options() const throw ();
    unsigned int           freshness_threshold() const throw ();
    unsigned int           high_flap_threshold() const throw ();
    list_string&           hostgroups() throw ();
    list_string const&     hostgroups() const throw ();
    std::string const&     host_name() const throw ();
    unsigned int           initial_state() const throw ();
    unsigned int           low_flap_threshold() const throw ();
    unsigned int           max_check_attempts() const throw ();
    bool                   obsess_over_host() const throw ();
    list_string&           parents() throw ();
    list_string const&     parents() const throw ();
    unsigned int           retry_interval() const throw ();
    std::string const&     timezone() const throw ();

  private:
    struct                 setters {
      char const*          name;
      bool                 (*func)(host&, char const*);
    };

    bool                   _set_action_url(std::string const& value);
    bool                   _set_address(std::string const& value);
    bool                   _set_alias(std::string const& value);
    bool                   _set_checks_active(bool value);
    bool                   _set_checks_passive(bool value);
    bool                   _set_check_command(std::string const& value);
    bool                   _set_check_freshness(bool value);
    bool                   _set_check_interval(unsigned int value);
    bool                   _set_check_period(std::string const& value);
    bool                   _set_check_timeout(unsigned int value);
    bool                   _set_contactgroups(std::string const& value);
    bool                   _set_contacts(std::string const& value);
    bool                   _set_coords_2d(std::string const& value);
    bool                   _set_coords_3d(std::string const& value);
    bool                   _set_display_name(std::string const& value);
    bool                   _set_event_handler(std::string const& value);
    bool                   _set_event_handler_enabled(bool value);
    bool                   _set_failure_prediction_enabled(bool value);
    bool                   _set_failure_prediction_options(std::string const& value);
    bool                   _set_first_notification_delay(unsigned int value);
    bool                   _set_flap_detection_enabled(bool value);
    bool                   _set_flap_detection_options(std::string const& value);
    bool                   _set_freshness_threshold(unsigned int value);
    bool                   _set_high_flap_threshold(unsigned int value);
    bool                   _set_host_name(std::string const& value);
    bool                   _set_hostgroups(std::string const& value);
    bool                   _set_icon_image(std::string const& value);
    bool                   _set_icon_image_alt(std::string const& value);
    bool                   _set_initial_state(std::string const& value);
    bool                   _set_low_flap_threshold(unsigned int value);
    bool                   _set_max_check_attempts(unsigned int value);
    bool                   _set_notes(std::string const& value);
    bool                   _set_notes_url(std::string const& value);
    bool                   _set_notifications_enabled(bool value);
    bool                   _set_notification_interval(unsigned int value);
    bool                   _set_notification_options(std::string const& value);
    bool                   _set_notification_period(std::string const& value);
    bool                   _set_obsess_over_host(bool value);
    bool                   _set_parents(std::string const& value);
    bool                   _set_process_perf_data(bool value);
    bool                   _set_retain_nonstatus_information(bool value);
    bool                   _set_retain_status_information(bool value);
    bool                   _set_retry_interval(unsigned int value);
    bool                   _set_stalking_options(std::string const& value);
    bool                   _set_statusmap_image(std::string const& value);
    bool                   _set_timezone(std::string const& value);
    bool                   _set_vrml_image(std::string const& value);

    std::string            _address;
    std::string            _alias;
    opt<bool>              _checks_active;
    std::string            _check_command;
    opt<bool>              _check_freshness;
    opt<unsigned int>      _check_interval;
    std::string            _check_period;
    opt<unsigned int>      _check_timeout;
    map_customvar          _customvariables;
    std::string            _event_handler;
    opt<bool>              _event_handler_enabled;
    opt<bool>              _flap_detection_enabled;
    opt<unsigned int>      _flap_detection_options;
    opt<unsigned int>      _freshness_threshold;
    opt<unsigned int>      _high_flap_threshold;
    group                  _hostgroups;
    std::string            _host_name;
    opt<unsigned int>      _initial_state;
    opt<unsigned int>      _low_flap_threshold;
    opt<unsigned int>      _max_check_attempts;
    opt<bool>              _obsess_over_host;
    group                  _parents;
    opt<unsigned int>      _retry_interval;
    static setters const   _setters[];
    opt<std::string>       _timezone;
  };

  typedef shared_ptr<host>    host_ptr;
  typedef std::list<host_ptr> list_host;
  typedef std::set<host_ptr>  set_host;
}

CCE_END()

#endif // !CCE_CONFIGURATION_HOST_HH
