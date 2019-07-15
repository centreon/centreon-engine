/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#ifndef CCE_NOTIFIER_HH
#define CCE_NOTIFIER_HH

# include <array>
# include <list>
# include <string>
# include <unordered_set>
# include "com/centreon/engine/checkable.hh"
# include "com/centreon/engine/contactgroup.hh"
# include "com/centreon/engine/customvariable.hh"
# include "com/centreon/engine/dependency.hh"
# include "common.hh"

class nagios_macros;
CCE_BEGIN()
// Forward declarations
class escalation;
class contact;
class timeperiod;
class notification;

class                notifier : public checkable {
 public:
  enum               notification_category {
    cat_normal,
    cat_recovery,
    cat_acknowledgement,
    cat_flapping,
    cat_downtime,
    cat_custom,
  };

  enum               notification_flag {
    none =              0,
    // Host
    up   =              1 << 0,
    down =              1 << 1,
    unreachable =       1 << 2,
    // Service
    ok =                1 << 3,
    warning =           1 << 4,
    critical =          1 << 5,
    unknown =           1 << 6,
    // Flapping
    flappingstart =     1 << 7,
    flappingstop =      1 << 8,
    flappingdisabled =  1 << 9,

    // Downtime
    downtime =          1 << 10,
  };

  enum               notifier_type {
    host_notification,
    service_notification,
  };

  enum               reason_type {
    reason_normal,
    reason_recovery,
    reason_acknowledgement,
    reason_flappingstart,
    reason_flappingstop,
    reason_flappingdisabled,
    reason_downtimestart,
    reason_downtimeend,
    reason_downtimecancelled,
    reason_custom = 99,
  };

  enum                notification_option {
    notification_option_none      = 0,
    notification_option_broadcast = 1,
    notification_option_forced    = 2,
    notification_option_increment = 4,
  };

  static std::array<std::string, 9> const tab_notification_str;

  static std::array<std::string, 2> const tab_state_type;

  typedef bool (notifier::*is_viable)(reason_type type,
                                      notification_option);

                     notifier(notifier_type notification_flag,
                              std::string const& display_name,
                              std::string const& check_command,
                              bool checks_enabled,
                              bool accept_passive_checks,
                              uint32_t check_interval,
                              uint32_t retry_interval,
                              uint32_t notification_interval,
                              int max_attempts,
                              int32_t notify,
                              int32_t stalk,
                              uint32_t first_notification_delay,
                              uint32_t recovery_notification_delay,
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
                              int freshness_threshold,
                              bool obsess_over,
                              std::string const& timezone);
                     ~notifier() {}


  bool               get_notify_on(notification_flag type) const;
  uint32_t           get_notify_on() const;
  void               set_notify_on(uint32_t type);
  void               add_notify_on(notification_flag type);
  void               remove_notify_on(notification_flag type);
  virtual bool       get_notify_on_current_state() const = 0;

  bool               get_notified_on(notification_flag type) const;
  uint32_t           get_notified_on() const;
  void               set_notified_on(uint32_t type);
  void               add_notified_on(notification_flag type);
  void               remove_notified_on(notification_flag type);

  bool               get_stalk_on(notification_flag type) const;
  uint32_t           get_stalk_on() const;
  void               set_stalk_on(uint32_t type);
  void               add_stalk_on(notification_flag type);

  bool               get_flap_detection_on(notification_flag type) const;
  uint32_t           get_flap_detection_on() const;
  void               set_flap_detection_on(uint32_t type);
  void               add_flap_detection_on(notification_flag type);


  unsigned long      get_current_event_id() const;
  void               set_current_event_id(unsigned long current_event_id);
  unsigned long      get_last_event_id() const;
  void               set_last_event_id(unsigned long last_event_id);
  unsigned long      get_current_problem_id() const;
  void               set_current_problem_id(unsigned long current_problem_id);
  unsigned long      get_last_problem_id() const;
  void               set_last_problem_id(unsigned long last_problem_id);

  virtual void       update_status(bool aggregated_dump) = 0;
  int                notify(reason_type type,
                            std::string const& not_author,
                            std::string const& not_data,
                            notification_option options);

  void               set_current_notification_id(uint64_t id);
  uint64_t           get_current_notification_id() const;
  virtual void       grab_macros_r(nagios_macros* mac) = 0;
  virtual int        notify_contact(nagios_macros* mac,
                                    contact* cntct,
                                    reason_type type,
                                    std::string const& not_author,
                                    std::string const& not_data,
                                    int options,
                                    int escalated) = 0;
  time_t             get_next_notification() const;
  void               set_next_notification(time_t next_notification);
  time_t             get_last_notification() const;
  void               set_last_notification(time_t last_notification);
  virtual void       update_notification_flags() = 0;
  time_t get_next_notification_time(time_t offset);
  void               set_initial_notif_time(time_t notif_time);
  time_t             get_initial_notif_time() const;
  void               set_acknowledgement_timeout(int timeout);
  void               set_last_acknowledgement(time_t ack);
  time_t             get_last_acknowledgement() const;
  uint32_t           get_notification_interval(void) const;
  void               set_notification_interval(uint32_t notification_interval);
  std::string const& get_notification_period() const;
  void               set_notification_period(std::string const& notification_period);

  uint32_t           get_first_notification_delay(void) const;
  void               set_first_notification_delay(uint32_t notification_delay);
  uint32_t           get_recovery_notification_delay(void) const;
  void               set_recovery_notification_delay(uint32_t notification_delay);
  bool               get_notifications_enabled() const;
  void               set_notifications_enabled(bool notifications_enabled);
  bool               should_notification_be_escalated() const;
  std::list<escalation*>&
                     get_escalations();
  std::list<escalation*> const&
                     get_escalations() const;
//  bool               is_escalated_contact(contact* cntct) const;
//  void               create_notification_list(nagios_macros* mac,
//                                int options,
//                                bool* esclated);
  virtual bool       is_valid_escalation_for_notification(
                                escalation const* e,
                                int options) const = 0;
  void               add_modified_attributes(uint32_t attr);
  uint32_t           get_modified_attributes() const;
  void               set_modified_attributes(uint32_t modified_attributes);
  bool               get_problem_has_been_acknowledged() const;
  void               set_problem_has_been_acknowledged(bool problem_has_been_acknowledged);
  virtual bool       recovered() const = 0;
  virtual int        get_current_state_int() const = 0;
  bool               get_no_more_notifications() const;
  void               set_no_more_notifications(bool no_more_notifications);
  bool               notifications_available(int options) const;
  int                get_notification_number() const;
  void               set_notification_number(int number);

  virtual bool authorized_by_dependencies(
      dependency::types dependency_type) const = 0;
  uint64_t           get_next_notification_id() const;
  virtual timeperiod* get_notification_timeperiod() const = 0;
  notification_category
                     get_category(reason_type type) const;
  bool is_notification_viable(notification_category cat,
                              reason_type type,
                              notification_option options);
  std::unordered_set<contact*> get_contacts_to_notify(
      notification_category cat,
      reason_type type,
      uint32_t& notification_interval);
  notifier_type      get_notifier_type() const;
  std::unordered_map<std::string, contact*>& get_contacts();
  std::unordered_map<std::string, contact*> const& get_contacts() const;
  contactgroup_map_unsafe& get_contactgroups();
  contactgroup_map_unsafe const& get_contactgroups() const;
  void resolve(int& w, int& e);
  std::array<int, MAX_STATE_HISTORY_ENTRIES> const& get_state_history() const;
  std::array<int, MAX_STATE_HISTORY_ENTRIES>& get_state_history();
  int get_pending_flex_downtime() const;
  void set_pending_flex_downtime(int pending_flex_downtime);
  virtual bool get_is_volatile() const = 0;
  void set_flap_type(uint32_t type);
  timeperiod* get_notification_period_ptr() const;
  void set_notification_period_ptr(timeperiod* tp);
  int get_acknowledgement_timeout() const;

  map_customvar custom_variables;

//  static int         add_notification(nagios_macros* mac, std::shared_ptr<contact> cntct);

  static std::unordered_map<std::string, std::shared_ptr<contact>> current_notifications;

 private:
  static std::array<is_viable, 6> const _is_notification_viable;
  static uint64_t    _next_notification_id;

  bool _is_notification_viable_normal(reason_type type,
                                      notification_option options);
  bool _is_notification_viable_recovery(reason_type type,
                                        notification_option options);
  bool _is_notification_viable_acknowledgement(
      reason_type type,
      notification_option options);
  bool _is_notification_viable_flapping(reason_type type,
                                        notification_option options);
  bool _is_notification_viable_downtime(reason_type type,
                                        notification_option options);
  bool _is_notification_viable_custom(reason_type type,
                                      notification_option options);

  notifier_type      _notifier_type;
  int32_t _stalk_type;
  uint32_t           _flap_type;
  unsigned long      _current_event_id;
  unsigned long      _last_event_id;
  unsigned long      _current_problem_id;
  unsigned long      _last_problem_id;

  time_t             _initial_notif_time;
  int                _acknowledgement_timeout;
  time_t             _last_acknowledgement;
  int32_t _out_notification_type;
  uint32_t           _current_notifications;
  uint32_t           _notification_interval;
  uint32_t           _modified_attributes;
  uint64_t           _current_notification_id;
  time_t             _next_notification;
  time_t             _last_notification;
  std::string        _notification_period;
  timeperiod* _notification_period_ptr;
  uint32_t           _first_notification_delay;
  uint32_t           _recovery_notification_delay;
  bool               _notifications_enabled;
  std::list<escalation*>
                     _escalations;
  bool               _problem_has_been_acknowledged;
  bool               _has_been_checked;
  bool               _no_more_notifications;

  /* New ones */
  int _notification_number;
  //reason_type _type;
  std::unordered_map<std::string, contact*> _contacts;
  contactgroup_map_unsafe _contact_groups;
  std::array<std::shared_ptr<notification>, 6> _notification;
  std::array<int, MAX_STATE_HISTORY_ENTRIES> _state_history;
  int _pending_flex_downtime;
};

CCE_END()

bool is_contact_for_notifier(com::centreon::engine::notifier* notif,
                             com::centreon::engine::contact* cntct);

#endif  // !CCE_NOTIFIER_HH
