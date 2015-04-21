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

#ifndef CCE_CONFIGURATION_SERVICE_HH
#  define CCE_CONFIGURATION_SERVICE_HH

#  include <list>
#  include <set>
#  include <utility>
#  include "com/centreon/engine/common.hh"
#  include "com/centreon/engine/configuration/duration.hh"
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/objects/customvariable.hh"
#  include "com/centreon/engine/opt.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                        configuration {
  /**
   *  @class service service.hh "com/centreon/engine/configuration/service.hh"
   *  @brief Service configuratin class.
   *
   *  Centreon Engine uses this class to hold the configuration of a
   *  service object.
   */
  class                          service : public object {
  public:
    enum                         action_on {
      none = 0,
      ok = (1 << 0),
      warning = (1 << 1),
      unknown = (1 << 2),
      critical = (1 << 3),
      flapping = (1 << 4)
    };
    typedef                      std::pair<std::string, std::string>
                                 key_type;

                                 service();
                                 service(service const& other);
                                 ~service() throw ();
    service&                     operator=(service const& other);
    bool                         operator==(service const& other) const throw ();
    bool                         operator!=(service const& other) const throw ();
    bool                         operator<(service const& other) const throw ();
    void                         check_validity() const;
    key_type                     key() const;
    void                         merge(object const& obj);
    bool                         parse(char const* key, char const* value);

    bool                         checks_active() const throw ();
    std::string const&           check_command() const throw ();
    bool                         check_freshness() const throw ();
    duration const&              check_interval() const throw ();
    std::string const&           check_period() const throw ();
    duration const&              check_timeout() const throw();
    map_customvar const&         customvariables() const throw ();
    std::string const&           event_handler() const throw ();
    bool                         event_handler_enabled() const throw ();
    bool                         flap_detection_enabled() const throw ();
    unsigned short               flap_detection_options() const throw ();
    unsigned int                 freshness_threshold() const throw ();
    unsigned int                 high_flap_threshold() const throw ();
    list_string&                 hosts() throw ();
    list_string const&           hosts() const throw ();
    unsigned int                 initial_state() const throw ();
    bool                         is_volatile() const throw ();
    unsigned int                 low_flap_threshold() const throw ();
    unsigned int                 max_check_attempts() const throw ();
    bool                         obsess_over_service() const throw ();
    duration const&              retry_interval() const throw ();
    std::string&                 service_description() throw ();
    std::string const&           service_description() const throw ();
    void                         timezone(std::string const& tz);
    std::string const&           timezone() const throw ();
    bool                         timezone_defined() const throw ();

  private:
    struct                       setters {
      char const*                name;
      bool (*                    func)(service&, char const*);
    };

    bool                         _set_check_command(std::string const& value);
    bool                         _set_checks_active(bool value);
    bool                         _set_check_freshness(bool value);
    bool                         _set_check_interval(std::string const& value);
    bool                         _set_check_period(std::string const& value);
    bool                         _set_check_timeout(std::string const& value);
    bool                         _set_event_handler(std::string const& value);
    bool                         _set_event_handler_enabled(bool value);
    bool                         _set_flap_detection_enabled(bool value);
    bool                         _set_flap_detection_options(std::string const& value);
    bool                         _set_freshness_threshold(unsigned int value);
    bool                         _set_high_flap_threshold(unsigned int value);
    bool                         _set_hosts(std::string const& value);
    bool                         _set_initial_state(std::string const& value);
    bool                         _set_is_volatile(bool value);
    bool                         _set_low_flap_threshold(unsigned int value);
    bool                         _set_max_check_attempts(unsigned int value);
    bool                         _set_obsess_over_service(bool value);
    bool                         _set_retry_interval(std::string const& value);
    bool                         _set_service_description(std::string const& value);
    bool                         _set_timezone(std::string const& value);

    opt<bool>                    _checks_active;
    std::string                  _check_command;
    opt<bool>                    _check_freshness;
    opt<duration>                _check_interval;
    std::string                  _check_period;
    opt<duration>                _check_timeout;
    map_customvar                _customvariables;
    std::string                  _event_handler;
    opt<bool>                    _event_handler_enabled;
    opt<bool>                    _flap_detection_enabled;
    opt<unsigned short>          _flap_detection_options;
    opt<unsigned int>            _freshness_threshold;
    opt<unsigned int>            _high_flap_threshold;
    group                        _hosts;
    opt<unsigned int>            _initial_state;
    opt<bool>                    _is_volatile;
    opt<unsigned int>            _low_flap_threshold;
    opt<unsigned int>            _max_check_attempts;
    opt<bool>                    _obsess_over_service;
    opt<duration>                _retry_interval;
    std::string                  _service_description;
    opt<std::string>             _timezone;

    static setters const         _setters[];
    static char const* const     _deprecated[][2];
 };

  typedef shared_ptr<service>    service_ptr;
  typedef std::list<service_ptr> list_service;
  typedef std::set<service_ptr>  set_service;
  typedef umap<std::pair<std::string, std::string>, service_ptr> map_service;
}

CCE_END()

#endif // !CCE_CONFIGURATION_SERVICE_HH
