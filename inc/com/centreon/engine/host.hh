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
#  include "com/centreon/engine/service.hh"


/* Forward declaration. */
extern "C" {
struct objectlist_struct;
};

CCE_BEGIN()
class host;
class hostescalation;
class timeperiod;
namespace commands {
  class command;
}
CCE_END()

typedef std::unordered_map<std::string,
  std::shared_ptr<com::centreon::engine::host>> host_map;

CCE_BEGIN()
class                host : public notifier {
 public:
  static std::array<std::pair<uint32_t, std::string>, 3> const tab_host_states;



  enum               host_state {
    state_up,
    state_down,
    state_unreachable
  };

                     host(uint64_t host_id,
                           std::string const& name,
                           std::string const& display_name,
                           std::string const& alias,
                           std::string const& address,
                           std::string const& check_period,
                           enum host::host_state initial_state,
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
                           bool notifications_enabled,
                           std::string const& check_command,
                           bool checks_enabled,
                           bool accept_passive_checks,
                           std::string const& event_handler,
                           bool event_handler_enabled,
                           bool flap_detection_enabled,
                           double low_flap_threshold,
                           double high_flap_threshold,
                           int flap_detection_on_up,
                           int flap_detection_on_down,
                           int flap_detection_on_unreachable,
                           int stalk_on_up,
                           int stalk_on_down,
                           int stalk_on_unreachable,
                           int process_perfdata,
                           bool check_freshness,
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
                           int obsess_over_host,
                           std::string const& timezone);
                     ~host() {}
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
  void               update_status(bool aggregated_dump) override;
  void               check_for_expired_acknowledgement();
  int                check_notification_viability(unsigned int type,
                                                  int options) override;
  int                handle_state();
  void               update_performance_data();
  int                verify_check_viability(int check_options,
                                            int* time_is_valid,
                                            time_t* new_time);
  void               grab_macros_r(nagios_macros* mac) override;
  bool               operator==(host const& other) throw ();
  bool               operator!=(host const& other) throw ();
  int                is_escalated_contact(contact* cntct);
  bool               is_result_fresh(time_t current_time, int log_this);

  int                run_sync_check_3x(enum host::host_state* check_result_code,
                                            int check_options,
                                            int use_cached_result,
                                            unsigned long check_timestamp_horizon);
  int                perform_on_demand_check_3x( enum host::host_state* check_result_code,
                                            int check_options,
                                            int use_cached_result,
                                            unsigned long check_timestamp_horizon);

  int                perform_on_demand_check(enum host::host_state* check_return_code,
                                                 int check_options,
                                                 int use_cached_result,
                                                 unsigned long check_timestamp_horizon);
  int                process_check_result_3x(enum host::host_state new_state,
                                                 char* old_plugin_output,
                                                 int check_options,
                                                 int reschedule_check,
                                                 int use_cached_result,
                                                 unsigned long check_timestamp_horizon);
  enum host_state    determine_host_reachability();
  bool               recovered() const override ;
  int                get_current_state_int() const override ;

  // setters / getters
  std::string const& get_name() const;
  void               set_name(std::string const& name);
  std::string const& get_alias() const;
  void               set_alias(std::string const& alias);
  std::string const& get_address() const;
  void               set_address(std::string const& address);
  bool               get_process_performance_data() const;
  void               set_process_performance_data(bool process_performance_data);
  int                get_retain_status_information() const;
  void               set_retain_status_information(int retain_status_information);
  int                get_retain_nonstatus_information() const;
  void               set_retain_nonstatus_information(int retain_nonstatus_information);
  bool               get_failure_prediction_enabled() const;
  void               set_failure_prediction_enabled(bool failure_prediction_enabled);
  int                get_obsess_over_host() const;
  void               set_obsess_over_host(int obsess_over_host);
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
  int                get_acknowledgement_type() const;
  void               set_acknowledgement_type(int acknowledgement_type);
  bool               get_is_executing() const;
  void               set_is_executing(bool is_executing);
  int                get_check_options() const;
  void               set_check_options(int check_options);
  time_t             get_next_check() const;
  void               set_next_check(time_t next_check);
  int                get_should_be_scheduled() const;
  void               set_should_be_scheduled(int should_be_scheduled);
  time_t             get_last_time_down() const;
  void               set_last_time_down(time_t last_time);
  time_t             get_last_time_unreachable() const;
  void               set_last_time_unreachable(time_t last_time);
  time_t             get_last_time_up() const;
  void               set_last_time_up(time_t last_time);
  bool               get_is_being_freshened() const;
  void               set_is_being_freshened(bool is_being_freshened);
  int                get_current_notification_number() const;
  void               set_current_notification_number(int current_notification_number);
  int                get_no_more_notifications() const;
  void               set_no_more_notifications(int no_more_notifications);
  int                get_check_flapping_recovery_notification() const;
  void               set_check_flapping_recovery_notification(int check_flapping_recovery_notification);
  int                get_pending_flex_downtime() const;
  void               set_pending_flex_downtime(int pending_flex_downtime);
  time_t             get_last_state_history_update() const;
  void               set_last_state_history_update(time_t last_state_history_update);
  unsigned long      get_flapping_comment_id() const;
  void               set_flapping_comment_id(unsigned long flapping_comment_id);
  void               disable_flap_detection();
  void               enable_flap_detection();
  int                get_total_services() const;
  void               set_total_services(int total_services);
  unsigned long      get_total_service_check_interval() const;
  void               set_total_service_check_interval(unsigned long total_service_check_interval);
  int                get_circular_path_checked() const;
  void               set_circular_path_checked(int check_level);
  bool               get_contains_circular_path() const;
  void               set_contains_circular_path(bool contains_circular_path);
  enum host_state    get_current_state() const;
  void               set_current_state(enum host_state current_state);
  enum host_state    get_last_state() const;
  void               set_last_state(enum host_state last_state);
  enum host_state    get_last_hard_state() const;
  void               set_last_hard_state(enum host_state last_hard_state);
  enum host_state    get_initial_state() const;
  void               set_initial_state(enum host_state current_state);
  int                notify_contact(nagios_macros* mac,
                                    contact* cntct,
                                    int type,
                                    char const* not_author,
                                    char const* not_data,
                                    int options,
                                    int escalated) override;
  void               update_notification_flags() override;
  time_t             get_next_notification_time(time_t offset) override;
  void               schedule_acknowledgement_expiration();
  bool               is_valid_escalation_for_notification(
                       std::shared_ptr<escalation> e,
                       int options) const override;
  void               handle_flap_detection_disabled();

  host_map            parent_hosts;
  host_map            child_hosts;
  static host_map     hosts;

  std::unordered_map<std::string, customvariable>
                      custom_variables;

  commands::command*  event_handler_ptr;
  commands::command*  check_command_ptr;

  service_map         services;
  timeperiod          *check_period_ptr;
  timeperiod          *notification_period_ptr;
  objectlist_struct*  hostgroups_ptr;

private:
  std::string         _name;
  std::string         _alias;
  std::string         _address;
  bool                _process_performance_data;
  int                 _retain_status_information;
  int                 _retain_nonstatus_information;
  bool                _failure_prediction_enabled;
  int                 _obsess_over_host;
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
  int                 _acknowledgement_type;
  bool                _is_executing;
  int                 _check_options;
  time_t              _next_check;
  int                 _should_be_scheduled;
  time_t              _last_time_down;
  time_t              _last_time_unreachable;
  time_t              _last_time_up;
  bool                _is_being_freshened;
  int                 _no_more_notifications;
  int                 _check_flapping_recovery_notification;
  int                 _pending_flex_downtime;
  unsigned int        _state_history_index;
  time_t              _last_state_history_update;
  unsigned long       _flapping_comment_id;
  int                 _total_services;
  unsigned long       _total_service_check_interval;
  int                 _circular_path_checked;
  bool                _contains_circular_path;

  enum host_state    _last_state;
  enum host_state    _last_hard_state;
  enum host_state    _current_state;
  enum host_state    _initial_state;
};

CCE_END()

/* Other HOST structure. */
struct                host_other_properties {
  bool                should_reschedule_current_check;
  uint64_t            host_id;
};

/* Hash structures. */
typedef struct        host_cursor_struct {
  int                 host_hashchain_iterator;
  com::centreon::engine::host*
                      current_host_pointer;
}                     host_cursor;

int                   is_contact_for_host(com::centreon::engine::host* hst,
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

std::ostream&         operator<<(std::ostream& os,
                            com::centreon::engine::host const& obj);
std::ostream&         operator<<(std::ostream& os, host_map const& obj);

CCE_BEGIN()

void                  check_for_expired_acknowledgement(
                            com::centreon::engine::host* h);
com::centreon::engine::host&
                      find_host(uint64_t host_id);
bool                  is_host_exist(uint64_t host_id) throw ();
uint64_t              get_host_id(std::string const& name);

CCE_END()

#endif // !CCE_HOST_HH
