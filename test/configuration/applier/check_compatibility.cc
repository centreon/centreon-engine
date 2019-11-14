/*
** Copyright 2011-2014 Merethis
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

#include <string>
#include "chkdiff.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/deleter.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects/comment.hh"
#include "com/centreon/engine/objects/downtime.hh"
#include "com/centreon/engine/objects/hostdependency.hh"
#include "com/centreon/engine/objects/servicedependency.hh"
#include "com/centreon/engine/retention/parser.hh"
#include "com/centreon/engine/retention/state.hh"
#include "com/centreon/engine/string.hh"
#include "compatibility/locations.h"
#include "test/unittest.hh"
#include "xodtemplate.hh"
#include "xrddefault.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

struct global {
  command* commands;
  comment* comments;
  contact* contacts;
  contactgroup* contactgroups;
  scheduled_downtime* downtimes;
  host* hosts;
  hostdependency* hostdependencies;
  hostescalation* hostescalations;
  hostgroup* hostgroups;
  service* services;
  servicedependency* servicedependencies;
  serviceescalation* serviceescalations;
  servicegroup* servicegroups;
  timeperiod* timeperiods;

  umap<std::string, std::shared_ptr<command> > save_commands;
  umap<std::string, std::shared_ptr<commands::connector> > save_connectors;
  umap<std::string, std::shared_ptr<contact> > save_contacts;
  umap<std::string, std::shared_ptr<contactgroup> > save_contactgroups;
  umap<std::string, std::shared_ptr<host> > save_hosts;
  umultimap<std::string, std::shared_ptr<hostdependency> >
      save_hostdependencies;
  umultimap<std::string, std::shared_ptr<hostescalation> > save_hostescalations;
  umap<std::string, std::shared_ptr<hostgroup> > save_hostgroups;
  umap<std::pair<std::string, std::string>, std::shared_ptr<service> >
      save_services;
  umultimap<std::pair<std::string, std::string>,
            std::shared_ptr<servicedependency> >
      save_servicedependencies;
  umultimap<std::pair<std::string, std::string>,
            std::shared_ptr<serviceescalation> >
      save_serviceescalations;
  umap<std::string, std::shared_ptr<servicegroup> > save_servicegroups;
  umap<std::string, std::shared_ptr<timeperiod> > save_timeperiods;

  bool accept_passive_host_checks;
  bool accept_passive_service_checks;
  int additional_freshness_latency;
  std::string admin_email;
  std::string admin_pager;
  bool allow_empty_hostgroup_assignment;
  bool auto_reschedule_checks;
  unsigned int auto_rescheduling_interval;
  unsigned int auto_rescheduling_window;
  unsigned long cached_host_check_horizon;
  unsigned long cached_service_check_horizon;
  bool check_external_commands;
  bool check_host_freshness;
  bool check_orphaned_hosts;
  bool check_orphaned_services;
  unsigned int check_reaper_interval;
  std::string check_result_path;
  bool check_service_freshness;
  int command_check_interval;
  std::string command_file;
  int date_format;
  std::string debug_file;
  // unsigned long       debug_level;
  unsigned int debug_verbosity;
  bool enable_environment_macros;
  bool enable_event_handlers;
  bool enable_failure_prediction;
  bool enable_flap_detection;
  bool enable_notifications;
  bool enable_predictive_host_dependency_checks;
  bool enable_predictive_service_dependency_checks;
  unsigned long event_broker_options;
  unsigned int event_handler_timeout;
  bool execute_host_checks;
  bool execute_service_checks;
  int external_command_buffer_slots;
  std::string global_host_event_handler;
  std::string global_service_event_handler;
  float high_host_flap_threshold;
  float high_service_flap_threshold;
  unsigned int host_check_timeout;
  unsigned int host_freshness_check_interval;
  int host_inter_check_delay_method;
  std::string illegal_object_chars;
  std::string illegal_output_chars;
  unsigned int interval_length;
  bool log_event_handlers;
  bool log_external_commands;
  //  std::string         log_file;
  bool log_host_retries;
  bool log_initial_states;
  bool log_notifications;
  bool log_passive_checks;
  bool log_service_retries;
  float low_host_flap_threshold;
  float low_service_flap_threshold;
  unsigned int max_check_reaper_time;
  unsigned long max_check_result_file_age;
  unsigned long max_debug_file_size;
  unsigned int max_host_check_spread;
  unsigned int max_parallel_service_checks;
  unsigned int max_service_check_spread;
  unsigned int notification_timeout;
  bool obsess_over_hosts;
  bool obsess_over_services;
  std::string ochp_command;
  unsigned int ochp_timeout;
  std::string ocsp_command;
  unsigned int ocsp_timeout;
  bool passive_host_checks_are_soft;
  bool process_performance_data;
  unsigned long retained_contact_host_attribute_mask;
  unsigned long retained_contact_service_attribute_mask;
  unsigned long retained_host_attribute_mask;
  unsigned long retained_process_host_attribute_mask;
  bool retain_state_information;
  unsigned int retention_scheduling_horizon;
  unsigned int retention_update_interval;
  unsigned int service_check_timeout;
  unsigned int service_freshness_check_interval;
  int service_inter_check_delay_method;
  int service_interleave_factor_method;
  float sleep_time;
  bool soft_state_dependencies;
  unsigned int status_update_interval;
  unsigned int time_change_threshold;
  bool translate_passive_host_checks;
  bool use_aggressive_host_checking;
  bool use_large_installation_tweaks;
  bool use_regexp_matches;
  bool use_retained_program_state;
  bool use_retained_scheduling_info;
  bool use_syslog;
  std::string use_timezone;
  bool use_true_regexp_matching;
};

#define check_value(id)                                                    \
  if (g1.id != g2.id) {                                                    \
    std::cerr << #id << " is different (" << g1.id << ", " << g2.id << ")" \
              << std::endl;                                                \
    ret = false;                                                           \
  }

static std::string to_str(char const* str) {
  return (str ? str : "");
}

template <typename T>
static void reset_next_check(T* lst) {
  for (T* obj(lst); obj; obj = obj->next) {
    obj->next_check = 0;
    obj->should_be_scheduled = 1;
  }
}

/**
 *  Sort a list.
 */
template <typename T>
static void sort_it(T*& l) {
  T* remaining(l);
  T** new_root(&l);
  *new_root = NULL;
  while (remaining) {
    T** min(&remaining);
    for (T** cur(&((*min)->next)); *cur; cur = &((*cur)->next))
      if (**cur < **min)
        min = cur;
    *new_root = *min;
    *min = (*min)->next;
    new_root = &((*new_root)->next);
    *new_root = NULL;
  }
  return;
}

/**
 *  Sort a list.
 */
template <typename T>
static void sort_it_rev(T*& l) {
  T* remaining(l);
  T** new_root(&l);
  *new_root = NULL;
  while (remaining) {
    T** min(&remaining);
    for (T** cur(&((*min)->next)); *cur; cur = &((*cur)->next))
      if (!(**cur < **min))
        min = cur;
    *new_root = *min;
    *min = (*min)->next;
    new_root = &((*new_root)->next);
    *new_root = NULL;
  }
  return;
}

/**
 *  Sort a list.
 */
static void sort_it(servicesmember*& l) {
  servicesmember* remaining(l);
  servicesmember** new_root(&l);
  *new_root = NULL;
  while (remaining) {
    servicesmember** min(&remaining);
    for (servicesmember** cur(&((*min)->next)); *cur; cur = &((*cur)->next)) {
      int host_less_than(strcmp((*cur)->host_name, (*min)->host_name));
      if ((host_less_than < 0) ||
          (!host_less_than && (strcmp((*cur)->service_description,
                                      (*min)->service_description) < 0)))
        min = cur;
    }
    *new_root = *min;
    *min = (*min)->next;
    new_root = &((*new_root)->next);
    *new_root = NULL;
  }
  return;
}

/**
 *  Remove duplicate members of a list.
 */
template <typename T>
static void remove_duplicates(T* l) {
  while (l) {
    for (T* m(l->next); m; m = m->next)
      if (*l == *m)
        l->next = m->next;
    l = l->next;
  }
  return;
}

/**
 *  Check difference between global object.
 *
 *  @param[in] l1 The first struct.
 *  @param[in] l2 The second struct.
 *
 *  @return True if globals are equal, otherwise false.
 */
bool chkdiff(global& g1, global& g2) {
  bool ret(true);

  check_value(accept_passive_host_checks);
  check_value(accept_passive_service_checks);
  check_value(additional_freshness_latency);
  check_value(admin_email);
  check_value(admin_pager);
  check_value(allow_empty_hostgroup_assignment);
  check_value(auto_reschedule_checks);
  check_value(auto_rescheduling_interval);
  check_value(auto_rescheduling_window);
  check_value(cached_host_check_horizon);
  check_value(cached_service_check_horizon);
  check_value(check_external_commands);
  check_value(check_host_freshness);
  check_value(check_orphaned_hosts);
  check_value(check_orphaned_services);
  check_value(check_reaper_interval);
  check_value(check_result_path);
  check_value(check_service_freshness);
  check_value(command_check_interval);
  check_value(command_file);
  check_value(date_format);
  check_value(debug_file);
  // check_value(debug_level);
  check_value(debug_verbosity);
  check_value(enable_environment_macros);
  check_value(enable_event_handlers);
  check_value(enable_failure_prediction);
  check_value(enable_flap_detection);
  check_value(enable_notifications);
  check_value(enable_predictive_host_dependency_checks);
  check_value(enable_predictive_service_dependency_checks);
  check_value(event_broker_options);
  check_value(event_handler_timeout);
  check_value(execute_host_checks);
  check_value(execute_service_checks);
  check_value(external_command_buffer_slots);
  check_value(global_host_event_handler);
  check_value(global_service_event_handler);
  check_value(high_host_flap_threshold);
  check_value(high_service_flap_threshold);
  check_value(host_check_timeout);
  check_value(host_freshness_check_interval);
  check_value(host_inter_check_delay_method);
  check_value(illegal_object_chars);
  check_value(illegal_output_chars);
  check_value(interval_length);
  check_value(log_event_handlers);
  check_value(log_external_commands);
  // check_value(log_file);
  check_value(log_host_retries);
  check_value(log_initial_states);
  check_value(log_notifications);
  check_value(log_passive_checks);
  check_value(log_service_retries);
  check_value(low_host_flap_threshold);
  check_value(low_service_flap_threshold);
  check_value(max_check_reaper_time);
  check_value(max_check_result_file_age);
  check_value(max_debug_file_size);
  check_value(max_host_check_spread);
  check_value(max_parallel_service_checks);
  check_value(max_service_check_spread);
  check_value(notification_timeout);
  check_value(obsess_over_hosts);
  check_value(obsess_over_services);
  check_value(ochp_command);
  check_value(ochp_timeout);
  check_value(ocsp_command);
  check_value(ocsp_timeout);
  check_value(passive_host_checks_are_soft);
  check_value(process_performance_data);
  check_value(retained_contact_host_attribute_mask);
  check_value(retained_contact_service_attribute_mask);
  check_value(retained_host_attribute_mask);
  check_value(retained_process_host_attribute_mask);
  check_value(retain_state_information);
  check_value(retention_scheduling_horizon);
  check_value(retention_update_interval);
  check_value(service_check_timeout);
  check_value(service_freshness_check_interval);
  check_value(service_inter_check_delay_method);
  check_value(service_interleave_factor_method);
  check_value(sleep_time);
  check_value(soft_state_dependencies);
  check_value(status_update_interval);
  check_value(time_change_threshold);
  check_value(translate_passive_host_checks);
  check_value(use_aggressive_host_checking);
  check_value(use_large_installation_tweaks);
  check_value(use_regexp_matches);
  check_value(use_retained_program_state);
  check_value(use_retained_scheduling_info);
  check_value(use_syslog);
  check_value(use_timezone);
  check_value(use_true_regexp_matching);

  for (scheduled_downtime* d(g1.downtimes); d; d = d->next)
    d->comment_id = 0;
  for (scheduled_downtime* d(g2.downtimes); d; d = d->next)
    d->comment_id = 0;
  if (!chkdiff(g1.downtimes, g2.downtimes))
    ret = false;

  for (comment* d(g1.comments); d; d = d->next)
    d->entry_time = 0;
  for (comment* d(g2.comments); d; d = d->next)
    d->entry_time = 0;
  if (!chkdiff(g1.comments, g2.comments))
    ret = false;

  if (!chkdiff(g1.commands, g2.commands))
    ret = false;
  if (!chkdiff(g1.contacts, g2.contacts))
    ret = false;
  for (contactgroup_struct* cg1(g1.contactgroups); cg1; cg1 = cg1->next)
    sort_it(cg1->members);
  for (contactgroup_struct* cg2(g2.contactgroups); cg2; cg2 = cg2->next)
    sort_it(cg2->members);
  if (!chkdiff(g1.contactgroups, g2.contactgroups))
    ret = false;
  reset_next_check(g1.hosts);
  reset_next_check(g2.hosts);
  if (!chkdiff(g1.hosts, g2.hosts))
    ret = false;
  sort_it(g1.hostdependencies);
  remove_duplicates(g1.hostdependencies);
  sort_it(g2.hostdependencies);
  remove_duplicates(g2.hostdependencies);
  if (!chkdiff(g1.hostdependencies, g2.hostdependencies))
    ret = false;
  for (hostescalation_struct* he1(g1.hostescalations); he1; he1 = he1->next) {
    sort_it(he1->contacts);
    sort_it(he1->contact_groups);
  }
  sort_it(g1.hostescalations);
  remove_duplicates(g1.hostescalations);
  for (hostescalation_struct* he2(g2.hostescalations); he2; he2 = he2->next) {
    sort_it(he2->contacts);
    sort_it(he2->contact_groups);
  }
  sort_it(g2.hostescalations);
  remove_duplicates(g2.hostescalations);
  if (!chkdiff(g1.hostescalations, g2.hostescalations))
    ret = false;
  if (!chkdiff(g1.hostgroups, g2.hostgroups))
    ret = false;
  reset_next_check(g1.services);
  reset_next_check(g2.services);
  for (service2* s(g1.services); s; s = s->next)
    sort_it_rev(s->custom_variables);
  if (!chkdiff(g1.services, g2.services))
    ret = false;
  sort_it(g1.servicedependencies);
  remove_duplicates(g1.servicedependencies);
  sort_it(g2.servicedependencies);
  remove_duplicates(g2.servicedependencies);
  if (!chkdiff(g1.servicedependencies, g2.servicedependencies))
    ret = false;
  for (serviceescalation_struct* se1(g1.serviceescalations); se1;
       se1 = se1->next) {
    sort_it(se1->contacts);
    sort_it(se1->contact_groups);
  }
  sort_it(g1.serviceescalations);
  remove_duplicates(g1.serviceescalations);
  for (serviceescalation_struct* se2(g2.serviceescalations); se2;
       se2 = se2->next) {
    sort_it(se2->contacts);
    sort_it(se2->contact_groups);
  }
  sort_it(g2.serviceescalations);
  remove_duplicates(g2.serviceescalations);
  if (!chkdiff(g1.serviceescalations, g2.serviceescalations))
    ret = false;
  for (servicegroup_struct* sg1(g1.servicegroups); sg1; sg1 = sg1->next)
    sort_it(sg1->members);
  for (servicegroup_struct* sg2(g2.servicegroups); sg2; sg2 = sg2->next)
    sort_it(sg2->members);
  if (!chkdiff(g1.servicegroups, g2.servicegroups))
    ret = false;
  if (!chkdiff(g1.timeperiods, g2.timeperiods))
    ret = false;
  return (ret);
}

/**
 *  Get all global configuration.
 *
 *  @retrun All global.
 */
static global get_globals() {
  global g;
  g.commands = command_list;
  command_list = NULL;
  g.comments = comment_list;
  comment_list = NULL;
  g.contacts = contact_list;
  contact_list = NULL;
  g.contactgroups = contactgroup_list;
  contactgroup_list = NULL;
  g.downtimes = scheduled_downtime_list;
  scheduled_downtime_list = NULL;
  g.hosts = host_list;
  host_list = NULL;
  g.hostdependencies = hostdependency_list;
  hostdependency_list = NULL;
  g.hostescalations = hostescalation_list;
  hostescalation_list = NULL;
  g.hostgroups = hostgroup_list;
  hostgroup_list = NULL;
  g.services = service_list;
  service_list = NULL;
  g.servicedependencies = servicedependency_list;
  servicedependency_list = NULL;
  g.serviceescalations = serviceescalation_list;
  serviceescalation_list = NULL;
  g.servicegroups = servicegroup_list;
  servicegroup_list = NULL;
  g.timeperiods = timeperiod_list;
  timeperiod_list = NULL;

  configuration::applier::state& app_state(
      configuration::applier::state::instance());
  g.save_commands = app_state.commands();
  app_state.commands().clear();
  g.save_connectors = app_state.connectors();
  app_state.connectors().clear();
  g.save_contacts = app_state.contacts();
  app_state.contacts().clear();
  g.save_contactgroups = app_state.contactgroups();
  app_state.contactgroups().clear();
  g.save_hosts = app_state.hosts();
  app_state.hosts().clear();
  g.save_hostdependencies = app_state.hostdependencies();
  app_state.hostdependencies().clear();
  g.save_hostescalations = app_state.hostescalations();
  app_state.hostescalations().clear();
  g.save_hostgroups = app_state.hostgroups();
  app_state.hostgroups().clear();
  g.save_services = app_state.services();
  app_state.services().clear();
  g.save_servicedependencies = app_state.servicedependencies();
  app_state.servicedependencies().clear();
  g.save_serviceescalations = app_state.serviceescalations();
  app_state.serviceescalations().clear();
  g.save_servicegroups = app_state.servicegroups();
  app_state.servicegroups().clear();
  g.save_timeperiods = app_state.timeperiods();
  app_state.timeperiods().clear();

  g.accept_passive_host_checks = accept_passive_host_checks;
  g.accept_passive_service_checks = accept_passive_service_checks;
  g.additional_freshness_latency = additional_freshness_latency;
  g.admin_email = to_str(macro_x[MACRO_ADMINEMAIL]);
  g.admin_pager = to_str(macro_x[MACRO_ADMINPAGER]);
  g.allow_empty_hostgroup_assignment = allow_empty_hostgroup_assignment;
  g.auto_reschedule_checks = auto_reschedule_checks;
  g.auto_rescheduling_interval = auto_rescheduling_interval;
  g.auto_rescheduling_window = auto_rescheduling_window;
  g.cached_host_check_horizon = cached_host_check_horizon;
  g.cached_service_check_horizon = cached_service_check_horizon;
  g.check_external_commands = check_external_commands;
  g.check_host_freshness = check_host_freshness;
  g.check_orphaned_hosts = check_orphaned_hosts;
  g.check_orphaned_services = check_orphaned_services;
  g.check_reaper_interval = check_reaper_interval;
  g.check_result_path = to_str(check_result_path);
  g.check_service_freshness = check_service_freshness;
  g.command_check_interval = command_check_interval;
  g.command_file = to_str(command_file);
  g.date_format = date_format;
  g.debug_file = to_str(debug_file);
  // g.debug_level = debug_level;
  g.debug_verbosity = debug_verbosity;
  g.enable_environment_macros = enable_environment_macros;
  g.enable_event_handlers = enable_event_handlers;
  g.enable_failure_prediction = enable_failure_prediction;
  g.enable_flap_detection = enable_flap_detection;
  g.enable_notifications = enable_notifications;
  g.enable_predictive_host_dependency_checks =
      enable_predictive_host_dependency_checks;
  g.enable_predictive_service_dependency_checks =
      enable_predictive_service_dependency_checks;
  g.event_broker_options = event_broker_options;
  g.event_handler_timeout = event_handler_timeout;
  g.execute_host_checks = execute_host_checks;
  g.execute_service_checks = execute_service_checks;
  g.external_command_buffer_slots = external_command_buffer_slots;
  g.global_host_event_handler = to_str(global_host_event_handler);
  g.global_service_event_handler = to_str(global_service_event_handler);
  g.high_host_flap_threshold = high_host_flap_threshold;
  g.high_service_flap_threshold = high_service_flap_threshold;
  g.host_check_timeout = host_check_timeout;
  g.host_freshness_check_interval = host_freshness_check_interval;
  g.host_inter_check_delay_method = host_inter_check_delay_method;
  g.illegal_object_chars = to_str(illegal_object_chars);
  g.illegal_output_chars = to_str(illegal_output_chars);
  g.interval_length = interval_length;
  g.log_event_handlers = log_event_handlers;
  g.log_external_commands = log_external_commands;
  // g.log_file = to_str(log_file);
  g.log_host_retries = log_host_retries;
  g.log_initial_states = log_initial_states;
  g.log_notifications = log_notifications;
  g.log_passive_checks = log_passive_checks;
  g.log_service_retries = log_service_retries;
  g.low_host_flap_threshold = low_host_flap_threshold;
  g.low_service_flap_threshold = low_service_flap_threshold;
  g.max_check_reaper_time = max_check_reaper_time;
  g.max_check_result_file_age = max_check_result_file_age;
  g.max_debug_file_size = max_debug_file_size;
  g.max_host_check_spread = max_host_check_spread;
  g.max_parallel_service_checks = max_parallel_service_checks;
  g.max_service_check_spread = max_service_check_spread;
  g.notification_timeout = notification_timeout;
  g.obsess_over_hosts = obsess_over_hosts;
  g.obsess_over_services = obsess_over_services;
  g.ochp_command = to_str(ochp_command);
  g.ochp_timeout = ochp_timeout;
  g.ocsp_command = to_str(ocsp_command);
  g.ocsp_timeout = ocsp_timeout;
  g.passive_host_checks_are_soft = passive_host_checks_are_soft;
  g.process_performance_data = process_performance_data;
  g.retained_contact_host_attribute_mask = retained_contact_host_attribute_mask;
  g.retained_contact_service_attribute_mask =
      retained_contact_service_attribute_mask;
  g.retained_host_attribute_mask = retained_host_attribute_mask;
  g.retained_process_host_attribute_mask = retained_process_host_attribute_mask;
  g.retain_state_information = retain_state_information;
  g.retention_scheduling_horizon = retention_scheduling_horizon;
  g.retention_update_interval = retention_update_interval;
  g.service_check_timeout = service_check_timeout;
  g.service_freshness_check_interval = service_freshness_check_interval;
  g.service_inter_check_delay_method = service_inter_check_delay_method;
  g.service_interleave_factor_method = service_interleave_factor_method;
  g.sleep_time = sleep_time;
  g.soft_state_dependencies = soft_state_dependencies;
  g.status_update_interval = status_update_interval;
  g.time_change_threshold = time_change_threshold;
  g.translate_passive_host_checks = translate_passive_host_checks;
  g.use_aggressive_host_checking = use_aggressive_host_checking;
  g.use_large_installation_tweaks = use_large_installation_tweaks;
  g.use_regexp_matches = use_regexp_matches;
  g.use_retained_program_state = use_retained_program_state;
  g.use_retained_scheduling_info = use_retained_scheduling_info;
  g.use_syslog = use_syslog;
  g.use_timezone = to_str(use_timezone);
  g.use_true_regexp_matching = use_true_regexp_matching;
  return (g);
}

/**
 *  Check for contact member.
 *
 *  @param[in] lst The object list to check.
 *  @param[in] obj The object to check.
 */
static bool member_is_already_in_list(contactsmember const* lst,
                                      contactsmember const* obj) {
  for (contactsmember const* m(lst); m && m != obj; m = m->next)
    if (!strcmp(m->contact_name, obj->contact_name))
      return (true);
  return (false);
}

/**
 *  Check for host member.
 *
 *  @param[in] lst The object list to check.
 *  @param[in] obj The object to check.
 */
static bool member_is_already_in_list(hostsmember const* lst,
                                      hostsmember const* obj) {
  for (hostsmember const* m(lst); m && m != obj; m = m->next)
    if (!strcmp(m->host_name, obj->host_name))
      return (true);
  return (false);
}

/**
 *  Check for service member.
 *
 *  @param[in] lst The object list to check.
 *  @param[in] obj The object to check.
 */
static bool member_is_already_in_list(servicesmember const* lst,
                                      servicesmember const* obj) {
  for (servicesmember const* m(lst); m && m != obj; m = m->next)
    if (!strcmp(m->host_name, obj->host_name) &&
        !strcmp(m->service_description, obj->service_description))
      return (true);
  return (false);
}

/**
 *  Remove duplicate members.
 *
 *  @param[in] lst     The object list to check.
 *  @param[in] deleter The deleter to delete duplicate members.
 */
template <typename T>
static void remove_duplicate_members(T* lst, void (*deleter)(void*)) {
  T* last(lst);
  for (T* m(lst); m; m = m->next) {
    if (member_is_already_in_list(lst, m)) {
      last->next = m->next;
      m->next = 0;
      deleter(m);
      m = last;
    }
    last = m;
  }
}

/**
 *  Remove duplicate members for all objects.
 *
 *  @param[in] lst     The object list to check.
 *  @param[in] deleter The deleter to delete duplicate members.
 */
template <typename T>
static void remove_duplicate_members_for_object(T const* lst,
                                                void (*deleter)(void*)) {
  for (T const* obj(lst); obj; obj = obj->next)
    remove_duplicate_members(obj->members, deleter);
}

/**
 *  Read configuration with new parser.
 *
 *  @parser[out] g        Fill global variable.
 *  @param[in]  filename  The file path to parse.
 *  @parse[in]  options   The options to use.
 *
 *  @return True on succes, otherwise false.
 */
static bool newparser_read_config(global& g,
                                  std::string const& filename,
                                  unsigned int options) {
  bool ret(false);
  try {
    init_macros();
    // Parse configuration.
    configuration::state config;

    // tricks to bypass create log file.
    config.log_file("");

    {
      configuration::parser p(options);
      p.parse(filename, config);
    }

    // Parse retention.
    retention::state state;
    try {
      retention::parser p;
      p.parse(config.state_retention_file(), state);
    } catch (...) {
    }

    configuration::applier::state::instance().apply(config, state);

    g = get_globals();
    clear_volatile_macros_r(get_global_macros());
    free_macrox_names();
    ret = true;
  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
  }
  return (ret);
}

/**
 *  Read configuration with old parser.
 *
 *  @parser[out] g        Fill global variable.
 *  @param[in]   filename The file path to parse.
 *  @parse[in]   options  The options to use.
 *
 *  @return True on succes, otherwise false.
 */
static bool oldparser_read_config(global& g,
                                  std::string const& filename,
                                  unsigned int options) {
  xrddefault_initialize_retention_data(filename.c_str());
  clear_volatile_macros_r(get_global_macros());
  free_macrox_names();
  init_object_skiplists();
  init_macros();
  int ret(read_main_config_file(filename.c_str()));
  if (ret == OK)
    ret = xodtemplate_read_config_data(filename.c_str(), options, false, false);
  if (ret == OK)
    ret = pre_flight_check();
  if (!check_result_path)
    check_result_path = string::dup(DEFAULT_CHECK_RESULT_PATH);
  if (!command_file)
    command_file = string::dup(DEFAULT_COMMAND_FILE);
  if (!debug_file)
    debug_file = string::dup(DEFAULT_DEBUG_FILE);
  if (!illegal_output_chars)
    illegal_output_chars = string::dup("`~$&|'\"<>");
  if (ret == OK) {
    remove_duplicate_members_for_object(contactgroup_list,
                                        &deleter::contactsmember);
    remove_duplicate_members_for_object(hostgroup_list, &deleter::hostsmember);
    remove_duplicate_members_for_object(servicegroup_list,
                                        &deleter::servicesmember);

    xrddefault_read_state_information();
    g = get_globals();
  }
  clear_volatile_macros_r(get_global_macros());
  free_macrox_names();
  free_object_skiplists();
  xrddefault_cleanup_retention_data(filename.c_str());
  return (ret == OK);
}

/**
 *  Check if the configuration parser works properly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  if (argc != 2)
    throw(engine_error() << "usage: " << argv[0] << " file.cfg");

  unsigned int options(configuration::parser::read_all);

  global oldcfg;
  if (!oldparser_read_config(oldcfg, argv[1], options))
    throw(engine_error() << "old parser can't parse " << argv[1]);

  global newcfg;
  if (!newparser_read_config(newcfg, argv[1], options))
    throw(engine_error() << "new parser can't parse " << argv[1]);

  bool ret(chkdiff(oldcfg, newcfg));

  // Delete downtimes.
  deleter::listmember(oldcfg.downtimes, &deleter::downtime);
  deleter::listmember(newcfg.downtimes, &deleter::downtime);

  // Delete comments.
  deleter::listmember(oldcfg.comments, &deleter::comment);
  deleter::listmember(newcfg.comments, &deleter::comment);
  return (!ret);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
