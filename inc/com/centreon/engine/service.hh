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
#  include <unordered_map>
#  include <utility>
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
};

CCE_BEGIN()
  namespace commands {
    class command;
  }
  class host;
  class service;
  class serviceescalation;
  class timeperiod;
CCE_END()

//Needed by service to use pair<string, string> as umap key.
//TODO SGA : check why servicedependency does not need it...
struct pair_hash {
  template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> & pair) const {
      return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

typedef std::unordered_map<std::pair<std::string, std::string>,
  std::shared_ptr<com::centreon::engine::service>, pair_hash> service_map;

CCE_BEGIN()

class                           service : public notifier {
 public:
  static std::array<std::pair<uint32_t, std::string>, 3> const tab_service_states;

                                service(std::string const& hostname,
                                        std::string const& description,
                                        std::string const& display_name,
                                        std::string const& check_command,
                                        bool checks_enabled,
                                        int initial_state,
                                        double check_interval,
                                        double retry_interval,
                                        int max_attempts,
                                        double first_notification_delay,
                                        std::string const& notification_period,
                                        bool notifications_enabled,
                                        std::string const& check_period,
                                        std::string const& event_handler,
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
                                ~service();
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
  void                          update_status(bool aggregated_dump) override;
  void                          set_notification_number(int num);
  int                           check_notification_viability(unsigned int type,
                                                             int options) override;
  int                           verify_check_viability(int check_options,
                                                       int* time_is_valid,
                                                       time_t* new_time);
  void                          grab_macros_r(nagios_macros* mac) override;
  int                           notify_contact(nagios_macros* mac,
                                               contact* cntct,
                                               int type,
                                               char const* not_author,
                                               char const* not_data,
                                               int options,
                                               int escalated) override;
  void                          update_notification_flags() override;
  time_t                        get_next_notification_time(time_t offset) override;
  void                          check_for_expired_acknowledgement();
  void                          schedule_acknowledgement_expiration();
  bool                          operator==(service const& other) throw();
  bool                          operator!=(service const& other) throw();
  bool                          is_valid_escalation_for_notification(
                                  std::shared_ptr<escalation> e,
                                  int options) const override;

  double                        notification_interval;
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
  int                           flap_detection_on_ok;
  int                           flap_detection_on_warning;
  int                           flap_detection_on_unknown;
  int                           flap_detection_on_critical;
  int                           process_performance_data;
  int                           freshness_threshold;
  int                           accept_passive_service_checks;
  int                           event_handler_enabled;
  int                           retain_status_information;
  int                           retain_nonstatus_information;
  int                           obsess_over_service;
  int                           problem_has_been_acknowledged;
  int                           acknowledgement_type;
  int                           host_problem_at_last_check;
  int                           current_state;
  int                           last_state;
  int                           last_hard_state;
  int                           state_type;
  time_t                        next_check;
  int                           should_be_scheduled;
  time_t                        last_check;
  uint64_t                      current_event_id;
  uint64_t                      last_event_id;
  uint64_t                      current_problem_id;
  uint64_t                      last_problem_id;
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

  std::unordered_map<std::string, customvariable>
    custom_variables;

  host*                         host_ptr;
  commands::command*            event_handler_ptr;
  char*                         event_handler_args;
  commands::command*            check_command_ptr;
  char*                         check_command_args;
  timeperiod*                   check_period_ptr;
  timeperiod*                   notification_period_ptr;
  objectlist_struct*            servicegroups_ptr;
  service*                      next;

 private:
  std::string                   _hostname;
  std::string                   _description;

};
CCE_END()

/* Other SERVICE structure. */
struct                          service_other_properties {
  uint64_t                      host_id;
  uint64_t                      service_id;
};

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

com::centreon::engine::service* add_service(
           uint64_t host_id,
           uint64_t service_id,
           std::string const& host_name,
           std::string const& description,
           std::string const& display_name,
           std::string const& check_period,
           int initial_state,
           int max_attempts,
           int accept_passive_checks,
           double check_interval,
           double retry_interval,
           double notification_interval,
           double first_notification_delay,
           std::string const& notification_period,
           bool notify_recovery,
           bool notify_unknown,
           bool notify_warning,
           bool notify_critical,
           bool notify_flapping,
           bool notify_downtime,
           bool notifications_enabled,
           bool is_volatile,
           std::string const& event_handler,
           bool event_handler_enabled,
           std::string const& check_command,
           bool checks_enabled,
           bool flap_detection_enabled,
           double low_flap_threshold,
           double high_flap_threshold,
           bool flap_detection_on_ok,
           bool flap_detection_on_warning,
           bool flap_detection_on_unknown,
           bool flap_detection_on_critical,
           bool stalk_on_ok,
           bool stalk_on_warning,
           bool stalk_on_unknown,
           bool stalk_on_critical,
           int process_perfdata,
           bool check_freshness,
           int freshness_threshold,
           std::string const& notes,
           std::string const& notes_url,
           std::string const& action_url,
           std::string const& icon_image,
           std::string const& icon_image_alt,
           int retain_status_information,
           int retain_nonstatus_information,
           int obsess_over_service,
           std::string const& timezone);
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

std::ostream& operator<<(std::ostream& os, com::centreon::engine::service const& obj);
std::ostream& operator<<(std::ostream& os, service_map const& obj);

CCE_BEGIN()

com::centreon::engine::service&      find_service(
                uint64_t host_id,
                uint64_t service_id);
bool          is_service_exist(
                std::pair<uint64_t, uint64_t> const& id);
std::pair<uint64_t, uint64_t>
              get_host_and_service_id(
                std::string const& host,
                std::string const& svc);
uint64_t      get_service_id(std::string const& host, std::string const& svc);

CCE_END()

#  endif /* C++ */

#endif // !CCE_SERVICE_HH
