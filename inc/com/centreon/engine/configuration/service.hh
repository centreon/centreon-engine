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
#  include "com/centreon/engine/configuration/opt.hh"
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
    std::size_t            id() const throw ();
    bool                   is_valid() const throw ();
    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

  private:
    bool                   _set_action_url(std::string const& value);
    bool                   _set_check_command(std::string const& value);
    bool                   _set_checks_active(bool value);
    bool                   _set_checks_passive(bool value);
    bool                   _set_check_freshness(bool value);
    bool                   _set_check_interval(unsigned int value);
    bool                   _set_check_period(std::string const& value);
    bool                   _set_contactgroups(std::string const& value);
    bool                   _set_contacts(std::string const& value);
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
    bool                   _set_hostgroups(std::string const& value);
    bool                   _set_hosts(std::string const& value);
    bool                   _set_icon_image(std::string const& value);
    bool                   _set_icon_image_alt(std::string const& value);
    bool                   _set_initial_state(std::string const& value);
    bool                   _set_is_volatile(bool value);
    bool                   _set_low_flap_threshold(unsigned int value);
    bool                   _set_max_check_attempts(unsigned int value);
    bool                   _set_notes(std::string const& value);
    bool                   _set_notes_url(std::string const& value);
    bool                   _set_notifications_enabled(bool value);
    bool                   _set_notification_options(std::string const& value);
    bool                   _set_notification_interval(unsigned int value);
    bool                   _set_notification_period(std::string const& value);
    bool                   _set_obsess_over_service(bool value);
    bool                   _set_parallelize_check(bool value);
    bool                   _set_process_perf_data(bool value);
    bool                   _set_retain_nonstatus_information(bool value);
    bool                   _set_retain_status_information(bool value);
    bool                   _set_retry_interval(unsigned int value);
    bool                   _set_servicegroups(std::string const& value);
    bool                   _set_service_description(std::string const& value);
    bool                   _set_stalking_options(std::string const& value);

    std::string            _action_url;
    opt<bool>              _checks_active;
    opt<bool>              _checks_passive;
    std::string            _check_command;
    bool                   _check_command_is_important;
    opt<bool>              _check_freshness;
    opt<unsigned int>      _check_interval;
    std::string            _check_period;
    group                  _contactgroups;
    group                  _contacts;
    umap<std::string, std::string>
                           _customvariables;
    std::string            _display_name;
    std::string            _event_handler;
    opt<bool>              _event_handler_enabled;
    opt<unsigned int>      _first_notification_delay;
    opt<bool>              _flap_detection_enabled;
    opt<unsigned short>    _flap_detection_options;
    opt<unsigned int>      _freshness_threshold;
    opt<unsigned int>      _high_flap_threshold;
    group                  _hostgroups;
    group                  _hosts;
    std::string            _icon_image;
    std::string            _icon_image_alt;
    opt<unsigned int>      _initial_state;
    opt<bool>              _is_volatile;
    opt<unsigned int>      _low_flap_threshold;
    opt<unsigned int>      _max_check_attempts;
    std::string            _notes;
    std::string            _notes_url;
    opt<bool>              _notifications_enabled;
    opt<unsigned int>      _notification_interval;
    opt<unsigned short>    _notification_options;
    std::string            _notification_period;
    opt<bool>              _obsess_over_service;
    opt<bool>              _process_perf_data;
    opt<bool>              _retain_nonstatus_information;
    opt<bool>              _retain_status_information;
    opt<unsigned int>      _retry_interval;
    group                  _servicegroups;
    std::string            _service_description;
    opt<unsigned short>    _stalking_options;
 };

  typedef umap<std::size_t, shared_ptr<service> > map_service;
}

CCE_END()

#endif // !CCE_CONFIGURATION_SERVICE_HH

