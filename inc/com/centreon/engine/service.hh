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

#ifndef CCE_SERVICE_HH
#  define CCE_SERVICE_HH

#  include <array>
#  include <memory>
#  include <string>
#  include <time.h>
#  include "com/centreon/engine/common.hh"
#  include "com/centreon/engine/contact.hh"
#  include "com/centreon/engine/contactgroup.hh"
#  include "com/centreon/engine/logging.hh"
#  include "com/centreon/engine/customvariable.hh"
#  include "com/centreon/engine/notifier.hh"
#  include "com/centreon/engine/checks.hh"

/* Forward declaration. */
extern "C" {
struct objectlist_struct;
struct timeperiod_struct;
};

CCE_BEGIN()
  namespace commands {
    class command;
  }
  class host;

class                           service : public notifier {
 public:
  static std::array<std::pair<uint32_t, std::string>, 3> const tab_service_states;

                                service(std::string const& hostname,
                                        std::string const& description,
                                        std::string const& display_name,
                                        std::string const& check_command,
                                        int initial_state,
                                        double check_interval,
                                        double retry_interval);
  virtual                       ~service() override;
  void                          set_hostname(std::string const& name);
  std::string const&            get_hostname() const;
  void                          set_description(std::string const& desc);
  std::string const&            get_description() const;
  int                           handle_async_check_result(
                                  check_result* queued_check_result);
  int                           log_event();
  void                          check_for_flapping(
                                              int update,
                                              int allow_flapstart_notification);
  int                           handle_service_event();
  int                           obsessive_compulsive_service_check_processor();
  int                           update_service_performance_data();
  int                           run_scheduled_check(int check_options, double latency);
  int                           run_async_check(int check_options,
                                                double latency,
                                                int scheduled_check,
                                                int reschedule_check,
                                                int* time_is_valid,
                                                time_t* preferred_time);
  void                          schedule_check(time_t check_time,
                                               int options);
  void                          set_flap(double percent_change,
                                         double high_threshold,
                                         double low_threshold,
                                         int allow_flapstart_notification);
  // handles a service that has stopped flapping
  void                          clear_flap(double percent_change,
                                           double high_threshold,
                                           double low_threshold);
  void                          enable_flap_detection();
  void                          disable_flap_detection();
  int                           notify(unsigned int type,
                                       char const* not_author,
                                       char const* not_data,
                                       int options);
  int                           check_notification_viability(unsigned int type,
                                                             int options);

  char*                         event_handler;
  int                           max_attempts;
  int                           parallelize;
  contactgroup_map              contact_groups;
  contact_map                   contacts;
  double                        notification_interval;
  double                        first_notification_delay;
  int                           notify_on_unknown;
  int                           notify_on_warning;
  int                           notify_on_critical;
  int                           notify_on_recovery;
  int                           notify_on_flapping;
  int                           notify_on_downtime;
  int                           stalk_on_ok;
  int                           stalk_on_warning;
  int                           stalk_on_unknown;
  int                           stalk_on_critical;
  int                           is_volatile;
  char*                         notification_period;
  char*                         check_period;
  int                           flap_detection_enabled;
  double                        low_flap_threshold;
  double                        high_flap_threshold;
  int                           flap_detection_on_ok;
  int                           flap_detection_on_warning;
  int                           flap_detection_on_unknown;
  int                           flap_detection_on_critical;
  int                           process_performance_data;
  int                           check_freshness;
  int                           freshness_threshold;
  int                           accept_passive_service_checks;
  int                           event_handler_enabled;
  int                           checks_enabled;
  int                           retain_status_information;
  int                           retain_nonstatus_information;
  int                           notifications_enabled;
  int                           obsess_over_service;
  int                           failure_prediction_enabled;
  char*                         failure_prediction_options;
  char*                         notes;
  char*                         notes_url;
  char*                         action_url;
  char*                         icon_image;
  char*                         icon_image_alt;
  std::unordered_map<std::string, com::centreon::engine::customvariable>
                                custom_variables;
  int                           problem_has_been_acknowledged;
  int                           acknowledgement_type;
  int                           host_problem_at_last_check;
  int                           check_type;
  int                           current_state;
  int                           last_state;
  int                           last_hard_state;
  char*                         plugin_output;
  char*                         long_plugin_output;
  char*                         perf_data;
  int                           state_type;
  time_t                        next_check;
  int                           should_be_scheduled;
  time_t                        last_check;
  int                           current_attempt;
  uint64_t                      current_event_id;
  uint64_t                      last_event_id;
  uint64_t                      current_problem_id;
  uint64_t                      last_problem_id;
  time_t                        last_notification;
  time_t                        next_notification;
  int                           no_more_notifications;
  int                           check_flapping_recovery_notification;
  time_t                        last_state_change;
  time_t                        last_hard_state_change;
  time_t                        last_time_ok;
  time_t                        last_time_warning;
  time_t                        last_time_unknown;
  time_t                        last_time_critical;
  int                           has_been_checked;
  int                           is_being_freshened;
  int                           notified_on_unknown;
  int                           notified_on_warning;
  int                           notified_on_critical;
  int                           current_notification_number;
  uint64_t                      current_notification_id;
  double                        latency;
  double                        execution_time;
  int                           is_executing;
  int                           check_options;
  int                           scheduled_downtime_depth;
  int                           pending_flex_downtime;
  int                           state_history[MAX_STATE_HISTORY_ENTRIES];
  unsigned int                  state_history_index;
  int                           is_flapping;
  uint64_t                      flapping_comment_id;
  double                        percent_state_change;
  unsigned long                 modified_attributes;

  com::centreon::engine::host*  host_ptr;
  com::centreon::engine::commands::command*
                                event_handler_ptr;
  char*                         event_handler_args;
  com::centreon::engine::commands::command*
                                check_command_ptr;
  char*                         check_command_args;
  timeperiod_struct*            check_period_ptr;
  timeperiod_struct*            notification_period_ptr;
  objectlist_struct*            servicegroups_ptr;
  service*        next;
  service*        nexthash;
 private:
  std::string                   _hostname;
  std::string                   _description;
};
CCE_END()

/* Other SERVICE structure. */
struct                          service_other_properties {
  time_t                        initial_notif_time;
  std::string                   timezone;
  uint64_t                      host_id;
  uint64_t                      service_id;
  int                           acknowledgement_timeout;
  time_t                        last_acknowledgement;
  unsigned int                  recovery_notification_delay;
  bool                          recovery_been_sent;
};

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

com::centreon::engine::service* add_service(
           uint64_t host_id,
           uint64_t service_id,
           std::string const& host_name,
           std::string const& description,
           char const* display_name,
           char const* check_period,
           int initial_state,
           int max_attempts,
           int parallelize,
           int accept_passive_checks,
           double check_interval,
           double retry_interval,
           double notification_interval,
           double first_notification_delay,
           char const* notification_period,
           int notify_recovery,
           int notify_unknown,
           int notify_warning,
           int notify_critical,
           int notify_flapping,
           int notify_downtime,
           int notifications_enabled,
           int is_volatile,
           char const* event_handler,
           int event_handler_enabled,
           char const* check_command,
           int checks_enabled,
           int flap_detection_enabled,
           double low_flap_threshold,
           double high_flap_threshold,
           int flap_detection_on_ok,
           int flap_detection_on_warning,
           int flap_detection_on_unknown,
           int flap_detection_on_critical,
           int stalk_on_ok,
           int stalk_on_warning,
           int stalk_on_unknown,
           int stalk_on_critical,
           int process_perfdata,
           int failure_prediction_enabled,
           char const* failure_prediction_options,
           int check_freshness,
           int freshness_threshold,
           char const* notes,
           char const* notes_url,
           char const* action_url,
           char const* icon_image,
           char const* icon_image_alt,
           int retain_status_information,
           int retain_nonstatus_information,
           int obsess_over_service);
int      get_service_count();
int      is_contact_for_service(
           com::centreon::engine::service* svc,
           com::centreon::engine::contact* cntct);
int      is_escalated_contact_for_service(
           com::centreon::engine::service* svc,
           com::centreon::engine::contact* cntct);

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>

bool          operator==(
                com::centreon::engine::service const& obj1,
                com::centreon::engine::service const& obj2) throw ();
bool          operator!=(
                com::centreon::engine::service const& obj1,
                com::centreon::engine::service const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, com::centreon::engine::service const& obj);

CCE_BEGIN()

void          check_for_expired_acknowledgement(com::centreon::engine::service* s);
com::centreon::engine::service&      find_service(
                uint64_t host_id,
                uint64_t service_id);
char const*   get_service_timezone(std::string const& hst, std::string const& svc);
bool          is_service_exist(
                std::pair<uint64_t, uint64_t> const& id);
std::pair<uint64_t, uint64_t>
              get_host_and_service_id(
                std::string const& host,
                std::string const& svc);
uint64_t      get_service_id(std::string const& host, std::string const& svc);
void          schedule_acknowledgement_expiration(com::centreon::engine::service* s);

CCE_END()

#  endif /* C++ */

#endif // !CCE_SERVICE_HH
