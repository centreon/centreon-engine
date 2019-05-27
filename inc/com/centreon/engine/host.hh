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

#ifndef CCE_HOST_HH
#  define CCE_HOST_HH

#  include <list>
#  include <unordered_map>
#  include <memory>
#  include <ostream>
#  include <string>
#  include <time.h>
#  include "com/centreon/engine/common.hh"
#  include "com/centreon/engine/logging.hh"
#  include "com/centreon/engine/contact.hh"
#  include "com/centreon/engine/contactgroup.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/notifier.hh"
#  include "com/centreon/engine/checks.hh"


/* Forward declaration. */
extern "C" {
struct objectlist_struct;
struct servicesmember_struct;
struct timeperiod_struct;
};

CCE_BEGIN()
class host;
namespace commands {
  class command;
}
CCE_END()

typedef std::unordered_map<std::string,
  std::shared_ptr<com::centreon::engine::host>> host_map;

CCE_BEGIN()
class                 host : public notifier {
 public:
  static std::array<std::pair<uint32_t, std::string>, 3> const tab_host_states;

                      host(uint64_t host_id,
                           std::string const& name,
                           std::string const& display_name,
                           std::string const& alias,
                           std::string const& address,
                           std::string const& check_period,
                           int initial_state,
                           double check_interval,
                           double retry_interval,
                           int max_attempts,
                           int notify_up,
                           int notify_down,
                           int notify_unreachable,
                           int notify_flapping,
                           int notify_downtime,
                           double notification_interval,
                           double first_notification_delay,
                           std::string const& notification_period,
                           int notifications_enabled,
                           std::string const& check_command,
                           int checks_enabled,
                           int accept_passive_checks,
                           std::string const& event_handler,
                           int event_handler_enabled,
                           int flap_detection_enabled,
                           double low_flap_threshold,
                           double high_flap_threshold,
                           int flap_detection_on_up,
                           int flap_detection_on_down,
                           int flap_detection_on_unreachable,
                           int stalk_on_up,
                           int stalk_on_down,
                           int stalk_on_unreachable,
                           int process_perfdata,
                           int check_freshness,
                           int freshness_threshold,
                           std::string const& notes,
                           std::string const& notes_url,
                           std::string const& action_url,
                           std::string const& icon_image,
                           std::string const& icon_image_alt,
                           std::string const& vrml_image,
                           std::string const& statusmap_image,
                           int x_2d,
                           int y_2d,
                           int have_2d_coords,
                           double x_3d,
                           double y_3d,
                           double z_3d,
                           int have_3d_coords,
                           int should_be_drawn,
                           int retain_status_information,
                           int retain_nonstatus_information,
                           int obsess_over_host);

  void               add_child_link(host* child);
  void               add_parent_host(std::string const& host_name);
  int                log_event();
  int                handle_async_check_result_3x(
                       check_result* queued_check_result);
  int                run_scheduled_check(int check_options, double latency);
  int                run_async_check(int check_options,
                                     double latency,
                                     int scheduled_check,
                                     int reschedule_check,
                                     int* time_is_valid,
                                     time_t* preferred_time);
  void               schedule_check(time_t check_time,
                                    int options);
  void               check_for_flapping(int update,
                                        int actual_check,
                                        int allow_flapstart_notification);
  void               set_flap(double percent_change,
                              double high_threshold,
                              double low_threshold,
                              int allow_flapstart_notification);
  void               clear_flap(double percent_change,
                                double high_threshold,
                                double low_threshold);
  void               enable_flap_detection();
  void               disable_flap_detection();
  void               update_status(bool aggregated_dump);
  void               check_for_expired_acknowledgement();
  void               set_notification_number(int num);
  int                notify(unsigned int type,
                            char const* not_author,
                            char const* not_data,
                            int options);
  int                check_notification_viability(unsigned int type,
                                                  int options);

  // setters / getters
  std::string const& get_name() const;
  void               set_name(std::string const& name);
  std::string const& get_alias() const;
  void               set_alias(std::string const& alias);
  std::string const& get_address() const;
  void               set_address(std::string const& address);
  int                get_max_attempts() const;
  void               set_max_attempts(int max_attempts);
  std::string const& get_event_handler() const;
  void               set_event_handler(std::string const& event_handler);
  double             get_notification_interval(void) const;
  void               set_notification_interval(double notification_interval);
  double             get_first_notification_delay(void) const;
  void               set_first_notification_delay(double notification_delay);
  int                get_notify_on_down() const;
  void               set_notify_on_down(int notify_on_down);
  int                get_notify_on_unreachable() const;
  void               set_notify_on_unreachable(int notify_on_unreachable);
  int                get_notify_on_recovery() const;
  void               set_notify_on_recovery(int notify_on_recovery);
  int                get_notify_on_flapping() const;
  void               set_notify_on_flapping(int notify_on_flapping);
  int                get_notify_on_downtime() const;
  void               set_notify_on_downtime(int notify_on_downtime);
  std::string const& get_notification_period() const;
  void               set_notification_period(
                       std::string const &notification_period);
  std::string const& get_check_period() const;
  void               set_check_period(std::string const& check_period);
  bool               get_flap_detection_enabled(void) const;
  void               set_flap_detection_enabled(bool flap_detection_enabled);
  double             get_low_flap_threshold() const;
  void               set_low_flap_threshold(double low_flap_threshold);
  double             get_high_flap_threshold() const;
  void               set_high_flap_threshold(double high_flap_threshold);
  bool               get_flap_detection_on_up() const;
  void               set_flap_detection_on_up(bool flag_detection_on_up);
  bool               get_flap_detection_on_down() const;
  void               set_flap_detection_on_down(bool flag_detection_on_down);
  bool               get_flap_detection_on_unreachable() const;
  void               set_flap_detection_on_unreachable(bool detection);
  bool               get_stalk_on_down() const;
  void               set_stalk_on_down(bool stalk);
  bool               get_stalk_on_unreachable() const;
  void               set_stalk_on_unreachable(bool stalk);
  bool               get_stalk_on_up() const;
  void               set_stalk_on_up(bool stalk);
  int                get_check_freshness() const;
  void               set_check_freshness(int check_freshness);
  int                get_freshness_threshold() const;
  void               set_freshness_threshold(int freshness_threshold);
  bool               get_process_performance_data() const;
  void               set_process_performance_data(bool process_performance_data);
  bool               get_checks_enabled() const;
  void               set_checks_enabled(bool checks_enabled);
  int                get_accept_passive_host_checks() const;
  void               set_accept_passive_host_checks(bool accept_passive_host_checks);
  bool               get_event_handler_enabled() const;
  void               set_event_handler_enabled(bool event_handler_enabled);
  int                get_retain_status_information() const;
  void               set_retain_status_information(int retain_status_information);
  int                get_retain_nonstatus_information() const;
  void               set_retain_nonstatus_information(int retain_nonstatus_information);
  bool               get_failure_prediction_enabled() const;
  void               set_failure_prediction_enabled(bool failure_prediction_enabled);
  std::string const& get_failure_prediction_options() const;
  void               set_failure_prediction_options(std::string const& failure);
  int                get_obsess_over_host() const;
  void               set_obsess_over_host(int obsess_over_host);
  std::string const& get_notes() const;
  void               set_notes(std::string const& notes);
  std::string const& get_notes_url() const;
  void               set_notes_url(std::string const& notes_url);
  std::string const& get_action_url() const;
  void               set_action_url(std::string const& action_url);
  std::string const& get_icon_image() const;
  void               set_icon_image(std::string const& icon_image);
  std::string const& get_icon_image_alt() const;
  void               set_icon_image_alt(std::string const& icon_image_alt);
  std::string const& get_vrml_image() const;
  void               set_vrml_image(std::string const& image);
  std::string const& get_statusmap_image() const;
  void               set_statusmap_image(std::string const& image);
  bool               get_have_2d_coords() const;
  void               set_have_2d_coords(bool has_coords);
  bool               get_have_3d_coords() const;
  void               set_have_3d_coords(bool has_coords);
  int                get_x_2d() const;
  void               set_x_2d(int x);
  int                get_y_2d() const;
  void               set_y_2d(int y);
  int                get_x_3d() const;
  void               set_x_3d(int x);
  int                get_y_3d() const;
  void               set_y_3d(int y);
  int                get_z_3d() const;
  void               set_z_3d(int z);
  int                get_should_be_drawn() const;
  void               set_should_be_drawn(int should_be_drawn);
  int                get_problem_has_been_acknowledged() const;
  void               set_problem_has_been_acknowledged(int problem_has_been_acknowledged);
  int                get_acknowledgement_type() const;
  void               set_acknowledgement_type(int acknowledgement_type);
  int                get_check_type() const;
  void               set_check_type(int check_type);
  int                get_current_state() const;
  void               set_current_state(int current_state);
  int                get_last_state() const;
  void               set_last_state(int last_state);
  int                get_last_hard_state() const;
  void               set_last_hard_state(int last_hard_state);
  std::string const& get_plugin_output() const;
  void               set_plugin_output(std::string const& plugin_output);
  std::string const& get_long_plugin_output() const;
  void               set_long_plugin_output(std::string const& long_plugin_output);
  std::string const& get_perf_data() const;
  void               set_perf_data(std::string const& perf_data);
  int                get_state_type() const;
  void               set_state_type(int state_type);
  int                get_current_attempt() const;
  void               set_current_attempt(int current_attempt);
  unsigned long      get_current_event_id() const;
  void               set_current_event_id(unsigned long current_event_id);
  unsigned long      get_last_event_id() const;
  void               set_last_event_id(unsigned long last_event_id);
  unsigned long      get_current_problem_id() const;
  void               set_current_problem_id(unsigned long current_problem_id);
  unsigned long      get_last_problem_id() const;
  void               set_last_problem_id(unsigned long last_problem_id);
  double             get_latency() const;
  void               set_latency(double latency);
  double             get_execution_time() const;
  void               set_execution_time(double execution_time);
  bool               get_is_executing() const;
  void               set_is_executing(bool is_executing);
  int                get_check_options() const;
  void               set_check_options(int check_options);
  bool               get_notifications_enabled() const;
  void               set_notifications_enabled(bool notifications_enabled);
  time_t             get_last_host_notification() const;
  void               set_last_host_notification(time_t last_host_notification);
  time_t             get_next_host_notification() const;
  void               set_next_host_notification(time_t next_host_notification);
  time_t             get_next_check() const;
  void               set_next_check(time_t next_check);
  int                get_should_be_scheduled() const;
  void               set_should_be_scheduled(int should_be_scheduled);
  time_t             get_last_check() const;
  void               set_last_check(time_t last_check);
  time_t             get_last_state_change() const;
  void               set_last_state_change(time_t last_state_change);
  time_t             get_last_hard_state_change() const;
  void               set_last_hard_state_change(time_t last_hard_state_change);
  time_t             get_last_time_down() const;
  void               set_last_time_down(time_t last_time);
  time_t             get_last_time_unreachable() const;
  void               set_last_time_unreachable(time_t last_time);
  time_t             get_last_time_up() const;
  void               set_last_time_up(time_t last_time);
  bool               get_has_been_checked() const;
  void               set_has_been_checked(bool has_been_checked);
  bool               get_is_being_freshened() const;
  void               set_is_being_freshened(bool is_being_freshened);
  bool               get_notified_on_down() const;
  void               set_notified_on_down(bool notified_on_down);
  bool               get_notified_on_unreachable() const;
  void               set_notified_on_unreachable(bool notified_on_unreachable);
  int                get_current_notification_number() const;
  void               set_current_notification_number(int current_notification_number);
  int                get_no_more_notifications() const;
  void               set_no_more_notifications(int no_more_notifications);
  unsigned long      get_current_notification_id() const;
  void               set_current_notification_id(unsigned long current_notification_id);
  int                get_check_flapping_recovery_notification() const;
  void               set_check_flapping_recovery_notification(int check_flapping_recovery_notification);
  int                get_scheduled_downtime_depth() const;
  void               set_scheduled_downtime_depth(int scheduled_downtime_depth);
  int                get_pending_flex_downtime() const;
  void               set_pending_flex_downtime(int pending_flex_downtime);
  unsigned int       get_state_history_index() const;
  void               set_state_history_index(unsigned int state_history_index);
  time_t             get_last_state_history_update() const;
  void               set_last_state_history_update(time_t last_state_history_update);
  bool               get_is_flapping() const;
  void               set_is_flapping(bool is_flapping);
  unsigned long      get_flapping_comment_id() const;
  void               set_flapping_comment_id(unsigned long flapping_comment_id);
  double             get_percent_state_change() const;
  void               set_percent_state_change(double percent_state_change);
  int                get_total_services() const;
  void               set_total_services(int total_services);
  unsigned long      get_total_service_check_interval() const;
  void               set_total_service_check_interval(unsigned long total_service_check_interval);
  unsigned long      get_modified_attributes() const;
  void               set_modified_attributes(unsigned long modified_attributes);
  int                get_circular_path_checked() const;
  void               set_circular_path_checked(int check_level);
  bool               get_contains_circular_path() const;
  void               set_contains_circular_path(bool contains_circular_path);

  contactgroup_map   contact_groups;
  contact_map        contacts;
  host_map           parent_hosts;
  host_map           child_hosts;
  static host_map    hosts;

  std::unordered_map<std::string, com::centreon::engine::customvariable>
                     custom_variables;

  com::centreon::engine::commands::command*
                      event_handler_ptr;
  com::centreon::engine::commands::command*
                      check_command_ptr;

  int                 state_history[MAX_STATE_HISTORY_ENTRIES];
  servicesmember_struct*
                      services;
  timeperiod_struct*  check_period_ptr;
  timeperiod_struct*  notification_period_ptr;
  objectlist_struct*  hostgroups_ptr;

private:
  std::string         _name;
  std::string         _alias;
  std::string         _address;
  int                 _max_attempts;
  std::string         _event_handler;
  double              _notification_interval;
  double              _first_notification_delay;
  int                 _notify_on_down;
  int                 _notify_on_unreachable;
  int                 _notify_on_recovery;
  int                 _notify_on_flapping;
  int                 _notify_on_downtime;
  std::string         _notification_period;
  std::string         _check_period;
  bool                _flap_detection_enabled;
  double              _low_flap_threshold;
  double              _high_flap_threshold;
  bool                _flap_detection_on_up;
  bool                _flap_detection_on_down;
  bool                _flap_detection_on_unreachable;
  bool                _stalk_on_down;
  bool                _stalk_on_unreachable;
  bool                _stalk_on_up;
  int                 _check_freshness;
  int                 _freshness_threshold;
  bool                _process_performance_data;
  bool                _checks_enabled;
  int                 _accept_passive_host_checks;
  bool                _event_handler_enabled;
  int                 _retain_status_information;
  int                 _retain_nonstatus_information;
  bool                _failure_prediction_enabled;
  std::string         _failure_prediction_options;
  int                 _obsess_over_host;
  std::string         _notes;
  std::string         _notes_url;
  std::string         _action_url;
  std::string         _icon_image;
  std::string         _icon_image_alt;
  std::string         _vrml_image;
  std::string         _statusmap_image;
  bool                _have_2d_coords;
  bool                _have_3d_coords;
  int                 _x_2d;
  int                 _y_2d;
  int                 _x_3d;
  int                 _y_3d;
  int                 _z_3d;
  int                 _should_be_drawn;
  int                 _problem_has_been_acknowledged;
  int                 _acknowledgement_type;
  int                 _check_type;
  int                 _current_state;
  int                 _last_state;
  int                 _last_hard_state;
  std::string         _plugin_output;
  std::string         _long_plugin_output;
  std::string         _perf_data;
  int                 _state_type;
  int                 _current_attempt;
  unsigned long       _current_event_id;
  unsigned long       _last_event_id;
  unsigned long       _current_problem_id;
  unsigned long       _last_problem_id;
  double              _latency;
  double              _execution_time;
  bool                _is_executing;
  int                 _check_options;
  bool                _notifications_enabled;
  time_t              _last_host_notification;
  time_t              _next_host_notification;
  time_t              _next_check;
  int                 _should_be_scheduled;
  time_t              _last_check;
  time_t              _last_state_change;
  time_t              _last_hard_state_change;
  time_t              _last_time_down;
  time_t              _last_time_unreachable;
  time_t              _last_time_up;
  bool                _has_been_checked;
  bool                _is_being_freshened;
  bool                _notified_on_down;
  bool                _notified_on_unreachable;
  int                 _current_notification_number;
  int                 _no_more_notifications;
  unsigned long       _current_notification_id;
  int                 _check_flapping_recovery_notification;
  int                 _scheduled_downtime_depth;
  int                 _pending_flex_downtime;
  unsigned int        _state_history_index;
  time_t              _last_state_history_update;
  bool                _is_flapping;
  unsigned long       _flapping_comment_id;
  double              _percent_state_change;
  int                 _total_services;
  unsigned long       _total_service_check_interval;
  unsigned long       _modified_attributes;
  int                 _circular_path_checked;
  bool                _contains_circular_path;
};

CCE_END()

/* Other HOST structure. */
struct                host_other_properties {
  time_t              initial_notif_time;
  bool                should_reschedule_current_check;
  std::string         timezone;
  uint64_t            host_id;
  int                 acknowledgement_timeout;
  time_t              last_acknowledgement;
  unsigned int        recovery_notification_delay;
  bool                recovery_been_sent;
};

/* Hash structures. */
typedef struct        host_cursor_struct {
  int                 host_hashchain_iterator;
  com::centreon::engine::host*
                      current_host_pointer;
}                     host_cursor;

int                   is_contact_for_host(com::centreon::engine::host* hst,
                          com::centreon::engine::contact* cntct);
int                   is_escalated_contact_for_host(
                                    com::centreon::engine::host* hst,
                                    com::centreon::engine::contact* cntct);
int                   is_host_immediate_child_of_host(
                                    com::centreon::engine::host* parent,
                                    com::centreon::engine::host* child);
int                   is_host_immediate_parent_of_host(
                                    com::centreon::engine::host* child,
                                    com::centreon::engine::host* parent);
int                   number_of_immediate_child_hosts(
                                    com::centreon::engine::host* hst);
int                   number_of_immediate_parent_hosts(
                                    com::centreon::engine::host* hst);
int                   number_of_total_child_hosts(
                                    com::centreon::engine::host* hst);
int                   number_of_total_parent_hosts(
                                    com::centreon::engine::host* hst);

bool                  operator==(
                            com::centreon::engine::host const& obj1,
                            com::centreon::engine::host const& obj2) throw ();
bool                  operator!=(
                            com::centreon::engine::host const& obj1,
                            com::centreon::engine::host const& obj2) throw ();
std::ostream&         operator<<(std::ostream& os,
                            com::centreon::engine::host const& obj);
std::ostream&         operator<<(std::ostream& os, host_map const& obj);

CCE_BEGIN()

void                  check_for_expired_acknowledgement(
                            com::centreon::engine::host* h);
com::centreon::engine::host&
                      find_host(uint64_t host_id);
char const*           get_host_timezone(std::string const& name);
bool                  is_host_exist(uint64_t host_id) throw ();
uint64_t              get_host_id(std::string const& name);
void                  schedule_acknowledgement_expiration(
                            com::centreon::engine::host* h);

CCE_END()

#endif // !CCE_HOST_HH
