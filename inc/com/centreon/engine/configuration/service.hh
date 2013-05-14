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

#ifndef CCE_CONFIGURATION_SERVICE_HH
#  define CCE_CONFIGURATION_SERVICE_HH

#  include "com/centreon/engine/common.hh"
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/unordered_hash.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    service
    : public object {
  public:
    enum                   action_on {
      none = 0,
      ok = (1 << 0),
      warning = (1 << 1),
      unknown = (1 << 2),
      critical = (1 << 3),
      recovery = (1 << 4),
      flapping = (1 << 5),
      downtime = (1 << 6)
    };

                           service();
                           service(service const& right);
                           ~service() throw ();
    service&               operator=(service const& right);
    bool                   operator==(
                             service const& right) const throw ();
    bool                   operator!=(
                             service const& right) const throw ();
    /*
    std::string const&     action_url() const throw ();
    std::string const&     check_command() const throw ();
    bool                   checks_enabled() const throw ();
    bool                   check_freshness() const throw ();
    unsigned int           check_interval() const throw ();
    std::string const&     check_period() const throw ();
    std::list<std::string> const&
                           contactgroups() const throw ();
    std::list<std::string> const&
                           contacts() const throw ();
    std::string const&     display_name() const throw ();
    std::string const&     event_handler() const throw ();
    bool                   event_handler_enabled() const throw ();
    unsigned int           first_notification_delay() const throw ();
    bool                   flap_detection_enabled() const throw ();
    unsigned int           flap_detection_options() const throw ();
    unsigned int           freshness_threshold() const throw ();
    unsigned int           high_flap_threshold() const throw ();
    std::list<std::string> const&
                           hostgroups() const throw ();
    std::list<std::string> const&
                           hosts() const throw ();
    std::string const&     icon_image() const throw ();
    std::string const&     icon_image_alt() const throw ();
    unsigned int           initial_state() const throw ();
    bool                   is_volatile() const throw ();
    unsigned int           low_flap_threshold() const throw ();
    unsigned int           max_check_attempts() const throw ();
    std::string const&     notes() const throw ();
    std::string const&     notes_url() const throw ();
    bool                   notifications_enabled() const throw ();
    unsigned int           notification_interval() const throw ();
    unsigned int           notification_options() const throw ();
    std::string const&     notification_period() const throw ();
    bool                   obsess_over_service() const throw ();
    bool                   process_perf_data() const throw ();
    bool                   retain_nonstatus_information() const throw ();
    bool                   retain_status_information() const throw ();
    unsigned int           retry_interval() const throw ();
    std::list<std::string> const&
                           servicegroups() const throw ();
    std::string const&     service_description() const throw ();
    unsigned int           stalking_options() const throw ();
    */

    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

  private:
    void                   _set_action_url(std::string const& value);
    void                   _set_check_command(std::string const& value);
    void                   _set_checks_active(bool value);
    void                   _set_checks_passive(bool value);
    void                   _set_check_freshness(bool value);
    void                   _set_check_interval(unsigned int value);
    void                   _set_check_period(std::string const& value);
    void                   _set_contactgroups(std::string const& value);
    void                   _set_contacts(std::string const& value);
    void                   _set_display_name(std::string const& value);
    void                   _set_event_handler(std::string const& value);
    void                   _set_event_handler_enabled(bool value);
    void                   _set_failure_prediction_enabled(bool value);
    void                   _set_failure_prediction_options(std::string const& value);
    void                   _set_first_notification_delay(unsigned int value);
    void                   _set_flap_detection_enabled(bool value);
    void                   _set_flap_detection_options(std::string const& value);
    void                   _set_freshness_threshold(unsigned int value);
    void                   _set_high_flap_threshold(unsigned int value);
    void                   _set_hostgroups(std::string const& value);
    void                   _set_hosts(std::string const& value);
    void                   _set_icon_image(std::string const& value);
    void                   _set_icon_image_alt(std::string const& value);
    void                   _set_initial_state(std::string const& value);
    void                   _set_is_volatile(bool value);
    void                   _set_low_flap_threshold(unsigned int value);
    void                   _set_max_check_attempts(unsigned int value);
    void                   _set_notes(std::string const& value);
    void                   _set_notes_url(std::string const& value);
    void                   _set_notifications_enabled(bool value);
    void                   _set_notification_options(std::string const& value);
    void                   _set_notification_interval(unsigned int value);
    void                   _set_notification_period(std::string const& value);
    void                   _set_obsess_over_service(bool value);
    void                   _set_parallelize_check(bool value);
    void                   _set_process_perf_data(bool value);
    void                   _set_retain_nonstatus_information(bool value);
    void                   _set_retain_status_information(bool value);
    void                   _set_retry_interval(unsigned int value);
    void                   _set_servicegroups(std::string const& value);
    void                   _set_service_description(std::string const& value);
    void                   _set_stalking_options(std::string const& value);

    std::string            _action_url;
    bool                   _checks_active;
    bool                   _checks_passive;
    std::string            _check_command;
    bool                   _check_command_is_important;
    bool                   _check_freshness;
    unsigned int           _check_interval;
    std::string            _check_period;
    group                  _contactgroups;
    group                  _contacts;
    umap<std::string, std::string>
                           _customvariables;
    std::string            _display_name;
    std::string            _event_handler;
    bool                   _event_handler_enabled;
    unsigned int           _first_notification_delay;
    bool                   _flap_detection_enabled;
    unsigned int           _flap_detection_options;
    unsigned int           _freshness_threshold;
    unsigned int           _high_flap_threshold;
    group                  _hostgroups;
    group                  _hosts;
    std::string            _icon_image;
    std::string            _icon_image_alt;
    unsigned int           _initial_state;
    bool                   _is_volatile;
    unsigned int           _low_flap_threshold;
    unsigned int           _max_check_attempts;
    std::string            _notes;
    std::string            _notes_url;
    bool                   _notifications_enabled;
    unsigned int           _notification_interval;
    unsigned int           _notification_options;
    std::string            _notification_period;
    bool                   _obsess_over_service;
    bool                   _process_perf_data;
    bool                   _retain_nonstatus_information;
    bool                   _retain_status_information;
    unsigned int           _retry_interval;
    group                  _servicegroups;
    std::string            _service_description;
    unsigned int           _stalking_options;
 };
}

CCE_END()

#endif // !CCE_CONFIGURATION_SERVICE_HH

