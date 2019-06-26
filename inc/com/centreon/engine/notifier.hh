/*
** Copyright 2019 Centreon
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
# include <list>
# include <string>
# include <unordered_set>
# include "com/centreon/engine/checkable.hh"
# include "com/centreon/engine/contactgroup.hh"
# include "com/centreon/engine/customvariable.hh"
# include "com/centreon/engine/namespace.hh"
# include "common.hh"

// Forward declarations
struct nagios_macros;

CCE_BEGIN()
class escalation;
class contact;
class timeperiod;

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

  enum               notification_type {
    none =            0,
    // Host
    up   =            1 << 0,
    down =            1 << 1,
    unreachable =     1 << 2,
    // Service
    ok =              1 << 3,
    warning =         1 << 4,
    critical =        1 << 5,
    unknown =         1 << 6,
    // Common
    recovery =        1 << 7,
    flapping =        1 << 8,
    downtime =        1 << 9,
  };

  enum               notifier_type {
    host_notification,
    service_notification,
  };

  enum               reason_type {
    notification_normal,
    notification_recovery,
    notification_acknowledgement,
    notification_flappingstart,
    notification_flappingstop,
    notification_flappingdisabled,
    notification_downtimestart,
    notification_downtimeend,
    notification_downtimecancelled,
    notification_custom = 99,
  };

  enum                notification_option {
    notification_option_none      = 0,
    notification_option_broadcast = 1,
    notification_option_forced    = 2,
    notification_option_increment = 4,
  };

  static std::array<std::string, 8> const tab_notification_str;
  static std::array<std::string, 2> const tab_state_type;

  typedef bool (notifier::* is_viable)(notification_option) const;

  //static void        inc_next_notification_id();

                     notifier(notifier_type notification_type,
                              std::string const& display_name,
                              std::string const& check_command,
                              bool checks_enabled,
                              bool accept_passive_checks,
                              double check_interval,
                              double retry_interval,
                              double notification_interval,
                              int max_attempts,
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


  bool               get_notify_on(notification_type type) const;
  uint32_t           get_notify_on() const;
  void               set_notify_on(uint32_t type);
  void               add_notify_on(notification_type type);
  void               remove_notify_on(notification_type type);
  virtual bool       get_notify_on_current_state() const = 0;

  bool               get_notified_on(notification_type type) const;
  uint32_t           get_notified_on() const;
  void               set_notified_on(uint32_t type);
  void               add_notified_on(notification_type type);
  void               remove_notified_on(notification_type type);

  bool               get_stalk_on(notification_type type) const;
  uint32_t           get_stalk_on() const;
  void               set_stalk_on(uint32_t type);
  void               add_stalk_on(notification_type type);

  bool               get_flap_detection_on(notification_type type) const;
  uint32_t           get_flap_detection_on() const;
  void               set_flap_detection_on(uint32_t type);
  void               add_flap_detection_on(notification_type type);


  unsigned long      get_current_event_id() const;
  void               set_current_event_id(unsigned long current_event_id);
  unsigned long      get_last_event_id() const;
  void               set_last_event_id(unsigned long last_event_id);
  unsigned long      get_current_problem_id() const;
  void               set_current_problem_id(unsigned long current_problem_id);
  unsigned long      get_last_problem_id() const;
  void               set_last_problem_id(unsigned long last_problem_id);

  void               set_notification_number(int num);
  virtual void       update_status(bool aggregated_dump) = 0;
//  virtual bool       check_notification_viability(reason_type type,
//                                                  int options) = 0;
  int                notify(reason_type type,
                            std::string const& not_author,
                            std::string const& not_data,
                            notification_option options);

  void               set_current_notification_id(uint64_t id);
  uint64_t           get_current_notification_id() const;
  virtual void       grab_macros_r(nagios_macros* mac) = 0;
  virtual int        notify_contact(nagios_macros* mac,
                                    contact* cntct,
                                    int type,
                                    char const* not_author,
                                    char const* not_data,
                                    int options,
                                    int escalated) = 0;
  time_t             get_next_notification() const;
  void               set_next_notification(time_t next_notification);
  time_t             get_last_notification() const;
  void               set_last_notification(time_t last_notification);
  virtual void       update_notification_flags() = 0;
  virtual time_t     get_next_notification_time(time_t offset) = 0;
  void               set_initial_notif_time(time_t notif_time);
  time_t             get_initial_notif_time() const;
  void               set_acknowledgement_timeout(int timeout);
  void               set_last_acknowledgement(time_t ack);
  time_t             get_last_acknowledgement() const;
  void               set_recovery_been_sent(bool sent);
  bool               get_recovery_been_sent() const;
  double             get_notification_interval(void) const;
  void               set_notification_interval(double notification_interval);
  std::string const& get_notification_period() const;
  void               set_notification_period(std::string const& notification_period);

  uint32_t           get_first_notification_delay(void) const;
  void               set_first_notification_delay(uint32_t notification_delay);
  uint32_t           get_recovery_notification_delay(void) const;
  void               set_recovery_notification_delay(uint32_t notification_delay);
  bool               get_notifications_enabled() const;
  void               set_notifications_enabled(bool notifications_enabled);
  bool               should_notification_be_escalated() const;
  std::list<std::shared_ptr<escalation>>&
                     get_escalations();
  std::list<std::shared_ptr<escalation>> const&
                     get_escalations() const;
  void               add_escalation(std::shared_ptr<escalation> e);
  bool               is_escalated_contact(contact* cntct) const;
  void               create_notification_list(nagios_macros* mac,
                                int options,
                                bool* esclated);
  virtual bool       is_valid_escalation_for_notification(
                                std::shared_ptr<escalation> e,
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
  int                get_current_notification_number() const;
  void               set_current_notification_number(int number);

  virtual uint64_t   check_dependencies(int dependency_type) = 0;
  uint64_t           get_next_notification_id() const;
  virtual timeperiod* get_notification_timeperiod() const = 0;
  notification_category
                     get_category(reason_type type) const;
  bool               is_notification_viable(notification_category cat,
                                      notification_option options);
  std::unordered_set<contact*>
                     get_contacts_to_notify(notification_category cat, int state);
  notifier_type      get_notifier_type() const;
  std::unordered_map<std::string, contact*>& get_contacts();
  std::unordered_map<std::string, contact*> const& get_contacts() const;
  contactgroup_map_unsafe& get_contactgroups();
  contactgroup_map_unsafe const& get_contactgroups() const;
  void resolve(int& w, int& e);

  int                state_history[MAX_STATE_HISTORY_ENTRIES];

  std::unordered_map<std::string, customvariable>
    custom_variables;

//  static int         add_notification(nagios_macros* mac, std::shared_ptr<contact> cntct);

  static std::unordered_map<std::string, std::shared_ptr<contact>> current_notifications;

  timeperiod          *notification_period_ptr;

 protected:
  notifier_type      _notifier_type;
  int                _stalk_type;
  int                _flap_type;
  unsigned long      _current_event_id;
  unsigned long      _last_event_id;
  unsigned long      _current_problem_id;
  unsigned long      _last_problem_id;

  uint64_t           _current_notification_id;
  time_t             _next_notification;
  time_t             _last_notification;

  time_t             _initial_notif_time;
  int                _acknowledgement_timeout;
  time_t             _last_acknowledgement;
  bool               _recovery_been_sent;
  double             _notification_interval;
  std::string        _notification_period;
  uint32_t           _out_notification_type;
  uint32_t           _in_notification_type;
  uint32_t           _modified_attributes;

 private:
  static std::array<is_viable, 6> const _is_notification_viable;

  bool               _is_notification_viable_normal(
                                            notification_option options) const;
  bool               _is_notification_viable_recovery(
                                            notification_option options) const;
  bool               _is_notification_viable_acknowledgement(
                                            notification_option options) const;
  bool               _is_notification_viable_flapping(
                                            notification_option options) const;
  bool               _is_notification_viable_downtime(
                                            notification_option options) const;
  bool               _is_notification_viable_custom(
                                            notification_option options) const;

  static uint64_t    _next_notification_id;
  uint32_t           _first_notification_delay;
  uint32_t           _recovery_notification_delay;
  bool               _notifications_enabled;
  std::list<std::shared_ptr<escalation>>
                     _escalations;
  bool               _problem_has_been_acknowledged;
  bool               _has_been_checked;
  bool               _no_more_notifications;

  /* DEPRECATED */
  int                _current_notification_number;

  /* New ones */
  int _notification_number;
  reason_type _type;
  std::unordered_map<std::string, contact*> _contacts;
  contactgroup_map_unsafe _contact_groups;
};

CCE_END()

bool is_contact_for_notifier(com::centreon::engine::notifier* notif,
                             com::centreon::engine::contact* cntct);

#endif  // !CCE_NOTIFIER_HH
