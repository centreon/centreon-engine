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

#ifndef CCE_NOTIFIER_HH
#define CCE_NOTIFIER_HH

# include <array>
# include <string>
# include "com/centreon/engine/namespace.hh"

// Forward declarations
struct nagios_macros;

CCE_BEGIN()
class escalation;

class                         notifier {
 public:
  enum notification_type {
    none =        0,
    // Host
    down =        1 << 0,
    unreachable = 1 << 1,
    // Service
    warning =     1 << 2,
    critical =    1 << 3,
    unknown =     1 << 4,
    // Common
    recovery =    1 << 5,
    flapping =    1 << 6,
    downtime =    1 << 7,
  };
  static std::array<std::string, 8> const tab_notification_str;
  static std::array<std::string, 2> const tab_state_type;

  static void                 inc_next_notification_id();

                              notifier(int notification_type,
                                       std::string const& display_name,
                                       std::string const& check_command,
                                       bool checks_enabled,
                                       bool accept_passive_checks,
                                       int initial_state,
                                       double check_interval,
                                       double retry_interval,
                                       int max_attempts,
                                       double first_notification_delay,
                                       std::string const& notification_period,
                                       bool notifications_enabled,
                                       std::string const& check_period,
                                       std::string const& event_handler,
                                       bool event_handler_enabled,
                                       std::string const& notes,
                                       std::string const& notes_url,
                                       std::string const& action_url,
                                       std::string const& icon_image,
                                       std::string const& icon_image_alt,
                                       bool flap_detection_enabled,
                                       double low_flap_threshold,
                                       double high_flap_threshold,
                                       bool check_freshness,
                                       std::string const& timezone);
  virtual                     ~notifier() {}

  std::string const&          get_display_name() const;
  void                        set_display_name(std::string const& name);
  std::string const&          get_check_command() const;
  void                        set_check_command(
                                std::string const& check_command);
  int                         get_initial_state() const;
  void                        set_initial_state(int initial_state);
  double                      get_check_interval() const;
  void                        set_check_interval(double check_interval);
  double                      get_retry_interval() const;
  void                        set_retry_interval(double retry_interval);
  void                        set_notification_number(int num);
  virtual void                update_status(bool aggregated_dump) = 0;
  int                         get_max_attempts() const;
  void                        set_max_attempts(int max_attempts);
  virtual int                 check_notification_viability(unsigned int type,
                                                           int options) = 0;
  int                         notify(unsigned int type,
                                     std::string const& not_author,
                                     std::string const& not_data,
                                     int options);

  void                        set_current_notification_id(uint64_t id);
  uint64_t                    get_current_notification_id() const;
  virtual void                grab_macros_r(nagios_macros* mac) = 0;
  virtual int                 notify_contact(nagios_macros* mac,
                                             contact* cntct,
                                             int type,
                                             char const* not_author,
                                             char const* not_data,
                                             int options,
                                             int escalated) = 0;
  time_t get_next_notification() const;
  void set_next_notification(time_t next_notification);
  time_t get_last_notification() const;
  void set_last_notification(time_t last_notification);
  virtual void update_notification_flags() = 0;
  int get_current_state() const;
  void set_current_state(int current_state);
  virtual time_t get_next_notification_time(time_t offset) = 0;
  void set_initial_notif_time(time_t notif_time);
  time_t get_initial_notif_time() const;
  void set_acknowledgement_timeout(int timeout);
  void set_last_acknowledgement(time_t ack);
  time_t get_last_acknowledgement() const;
  void set_recovery_notification_delay(uint32_t delay);
  void set_recovery_been_sent(bool sent);
  bool get_recovery_been_sent() const;
  double get_notification_interval(void) const;
  void set_notification_interval(double notification_interval);
  std::string const& get_notification_period() const;
  void set_notification_period(std::string const& notification_period);
  std::string const& get_check_period() const;
  void set_check_period(std::string const& check_period);
  std::string const& get_action_url() const;
  void set_action_url(std::string const& action_url);
  std::string const& get_icon_image() const;
  void set_icon_image(std::string const& icon_image);
  std::string const& get_icon_image_alt() const;
  void set_icon_image_alt(std::string const& icon_image_alt);
  std::string const& get_event_handler() const;
  void set_event_handler(std::string const& event_handler);
  std::string const& get_notes() const;
  void set_notes(std::string const& notes);
  std::string const& get_notes_url() const;
  void set_notes_url(std::string const& notes_url);
  std::string const& get_plugin_output() const;
  void set_plugin_output(std::string const& plugin_output);
  std::string const& get_long_plugin_output() const;
  void set_long_plugin_output(std::string const& long_plugin_output);
  std::string const& get_perf_data() const;
  void set_perf_data(std::string const& perf_data);
  bool get_flap_detection_enabled(void) const;
  void set_flap_detection_enabled(bool flap_detection_enabled);
  double get_low_flap_threshold() const;
  void set_low_flap_threshold(double low_flap_threshold);
  double get_high_flap_threshold() const;
  void set_high_flap_threshold(double high_flap_threshold);
  bool get_notify_on(notification_type type) const;
  uint32_t get_notify_on() const;
  void set_notify_on(uint32_t type);
  void add_notify_on(notification_type type);
  void remove_notify_on(notification_type type);
  bool get_notified_on(notification_type type) const;
  uint32_t get_notified_on() const;
  void set_notified_on(uint32_t type);
  void add_notified_on(notification_type type);
  void remove_notified_on(notification_type type);
  double get_first_notification_delay(void) const;
  void set_first_notification_delay(double notification_delay);
  bool get_notifications_enabled() const;
  void set_notifications_enabled(bool notifications_enabled);
  std::string const& get_timezone() const;
  void set_timezone(std::string const& timezone);
  bool should_notification_be_escalated() const;
  std::list<std::shared_ptr<escalation>>& get_escalations();
  std::list<std::shared_ptr<escalation>> const&
                              get_escalations() const;
  void add_escalation(std::shared_ptr<escalation> e);
  bool is_escalated_contact(contact* cntct) const;
  void create_notification_list(nagios_macros* mac,
                                int options,
                                bool* esclated);
  virtual bool is_valid_escalation_for_notification(
      std::shared_ptr<escalation> e,
      int options) const = 0;
  void               add_modified_attributes(uint32_t attr);
  uint32_t           get_modified_attributes() const;
  void               set_modified_attributes(uint32_t modified_attributes);
  bool               get_checks_enabled() const;
  void               set_checks_enabled(bool checks_enabled);
  bool               get_check_freshness() const;
  void               set_check_freshness(bool check_freshness);
  int                get_check_type() const;
  void               set_check_type(int check_type);
  int                get_current_attempt() const;
  void               set_current_attempt(int current_attempt);
  void               add_current_attempt(int num);
  bool get_problem_has_been_acknowledged() const;
  void set_problem_has_been_acknowledged(bool problem_has_been_acknowledged);
  bool               get_has_been_checked() const;
  void               set_has_been_checked(bool has_been_checked);
  bool               get_event_handler_enabled() const;
  void               set_event_handler_enabled(bool event_handler_enabled);
  bool               get_accept_passive_checks() const;
  void               set_accept_passive_checks(bool accept_passive_checks);

  contact_map        contacts;
  contactgroup_map   contact_groups;
 protected:
  int _notifier_type;
  std::string _display_name;
  std::string _check_command;
  int _initial_state;
  double _check_interval;
  double _retry_interval;
  int _current_notification_number;
  int _max_attempts;
  uint64_t _current_notification_id;
  time_t _next_notification;
  time_t _last_notification;
  int _current_state;

  time_t _initial_notif_time;
  int _acknowledgement_timeout;
  time_t _last_acknowledgement;
  uint32_t _recovery_notification_delay;
  bool _recovery_been_sent;
  double _notification_interval;
  std::string _notification_period;
  uint32_t _out_notification_type;
  uint32_t _in_notification_type;
  uint32_t _modified_attributes;

 private:
  static uint64_t _next_notification_id;
  std::string _check_period;
  std::string _event_handler;
  std::string _action_url;
  std::string _icon_image;
  std::string _icon_image_alt;
  std::string _notes;
  std::string _notes_url;
  std::string _plugin_output;
  std::string _long_plugin_output;
  std::string _perf_data;
  bool _flap_detection_enabled;
  double _low_flap_threshold;
  double _high_flap_threshold;
  double _first_notification_delay;
  bool _notifications_enabled;
  std::string _timezone;
  std::list<std::shared_ptr<escalation>> _escalations;
  bool _checks_enabled;
  bool                _accept_passive_checks;
  bool _check_freshness;
  int                 _check_type;
  int                 _current_attempt;
  bool _problem_has_been_acknowledged;
  bool                _has_been_checked;
  bool                _event_handler_enabled;
};

CCE_END()

#endif  // !CCE_NOTIFIER_HH
