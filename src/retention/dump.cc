/*
** Copyright 2011-2013,2015-2016 Centreon
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

#include <fstream>
#include <iomanip>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/downtimes/downtime.hh"
#include "com/centreon/engine/downtimes/host_downtime.hh"
#include "com/centreon/engine/downtimes/service_downtime.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/retention/dump.hh"

using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::retention;

/**
 *  Dump retention of comment.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The comment to dump.
 *
 *  @return The output stream.
 */
std::ostream& dump::comment(std::ostream& os, com::centreon::engine::comment const& obj) {
  if (obj.get_comment_type() == com::centreon::engine::comment::host)
    os << "hostcomment {\n";
  else
    os << "servicecomment {\n";
  os << "host_name=" << obj.get_host_name() << "\n";
  if (obj.get_comment_type() == com::centreon::engine::comment::service)
    os << "service_description=" << obj.get_service_description() << "\n";
  os << "author=" << obj.get_author() << "\n"
    "comment_data=" << obj.get_comment_data() << "\n"
    "comment_id=" << obj.get_comment_id() << "\n"
    "entry_time=" << static_cast<unsigned long>(obj.get_entry_time()) << "\n"
    "expire_time=" << static_cast<unsigned long>(obj.get_expire_time()) << "\n"
    "expires=" << obj.get_expires() << "\n"
    "persistent=" << obj.get_persistent() << "\n"
    "source=" << obj.get_source() << "\n"
    "entry_type=" << obj.get_entry_type() << "\n"
    "}\n";
  return os;
}

/**
 *  Dump retention of comments.
 *
 *  @param[out] os The output stream.
 *
 *  @return The output stream.
 */
std::ostream& dump::comments(std::ostream& os) {
  for (comment_map::iterator
         it(comment::comments.begin()),
         end(comment::comments.end());
       it != end;
       ++it)
    dump::comment(os, *it->second);
  return os;
}

/**
 *  Dump retention of contact.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The contact to dump.
 *
 *  @return The output stream.
 */
std::ostream& dump::contact(std::ostream& os, com::centreon::engine::contact const& obj) {
  os << "contact {\n"
    "contact_name=" << obj.get_name() << "\n"
    "host_notification_period=" << obj.get_host_notification_period() << "\n"
    "host_notifications_enabled=" << obj.get_host_notifications_enabled() << "\n"
    "last_host_notification=" << static_cast<unsigned long>(obj.get_last_host_notification()) << "\n"
    "last_service_notification=" << static_cast<unsigned long>(obj.get_last_service_notification()) << "\n"
    "modified_attributes=" << (obj.get_modified_attributes() & ~0L) << "\n"
    "modified_host_attributes=" << (obj.get_modified_host_attributes() & ~config->retained_contact_host_attribute_mask()) << "\n"
    "modified_service_attributes=" << (obj.get_modified_service_attributes() & ~config->retained_contact_service_attribute_mask()) << "\n"
    "service_notification_period=" << obj.get_service_notification_period() << "\n"
    "service_notifications_enabled=" << obj.get_service_notifications_enabled() << "\n";
  dump::customvariables(os, obj.get_custom_variables());
  os << "}\n";
  return os;
}

/**
 *  Dump retention of contacts.
 *
 *  @param[out] os The output stream.
 *
 *  @return The output stream.
 */
std::ostream& dump::contacts(std::ostream& os) {
  for (contact_map::const_iterator
         it{contact::contacts.begin()},
         end{contact::contacts.end()};
       it != end; ++it)
    dump::contact(os, *it->second.get());

  return os;
}

/**
 *  Dump retention of custom variables.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The custom variables to dump.
 *
 *  @return The output stream.
 */
std::ostream& dump::customvariables(
                std::ostream& os,
                map_customvar const& obj) {
  for (std::pair<std::string, std::shared_ptr<customvariable>> const& cv : obj)
    os << "_" << cv.first << "=" << cv.second->has_been_modified() << ","
       << cv.second->get_value() << "\n";
  return os;
}

/**
 *  Dump retention of downtime.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The downtime to dump.
 *
 *  @return The output stream.
 */
std::ostream& dump::scheduled_downtime(std::ostream& os, downtime const& obj) {
  obj.retention(os);
  return os;
}

/**
 *  Dump retention of downtimes.
 *
 *  @param[out] os The output stream.
 *
 *  @return The output stream.
 */
std::ostream& dump::downtimes(std::ostream& os) {
  for (std::pair<time_t, std::shared_ptr<downtime>> const& obj : downtimes::downtime_manager::instance().get_scheduled_downtimes())
    dump::scheduled_downtime(os, *obj.second);
  return os;
}

/**
 *  Dump header retention.
 *
 *  @param[out] os The output stream.
 *
 *  @return The output stream.
 */
std::ostream& dump::header(std::ostream& os) {
  os << "##############################################\n"
        "#    CENTREON ENGINE STATE RETENTION FILE    #\n"
        "#                                            #\n"
        "# THIS FILE IS AUTOMATICALLY GENERATED BY    #\n"
        "# CENTREON ENGINE. DO NOT MODIFY THIS FILE ! #\n"
        "##############################################\n";
  return os;
}

/**
 *  Dump retention of host.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The host to dump.
 *
 *  @return The output stream.
 */
std::ostream& dump::host(std::ostream& os, com::centreon::engine::host const& obj) {
  os << "host {\n"
    "host_name=" << obj.get_name() << "\n"
    "host_id=" << obj.get_host_id() << "\n"
    "acknowledgement_type=" << obj.get_acknowledgement_type() << "\n"
    "active_checks_enabled=" << obj.get_checks_enabled() << "\n"
    "check_command=" << obj.get_check_command() << "\n"
    "check_execution_time=" << std::setprecision(3) << std::fixed << obj.get_execution_time() << "\n"
    "check_latency=" << std::setprecision(3) << std::fixed << obj.get_latency() << "\n"
    "check_options=" << obj.get_check_options() << "\n"
    "check_period=" << obj.get_check_period() << "\n"
    "check_type=" << obj.get_check_type() << "\n"
    "current_attempt=" << obj.get_current_attempt() << "\n"
    "current_event_id=" << obj.get_current_event_id() << "\n"
    "current_notification_id=" << obj.get_current_notification_id() << "\n"
    "current_notification_number=" << obj.get_notification_number() << "\n"
    "current_problem_id=" << obj.get_current_problem_id() << "\n"
    "current_state=" << obj.get_current_state() << "\n"
    "event_handler=" << obj.get_event_handler() << "\n"
    "event_handler_enabled=" << obj.get_event_handler_enabled() << "\n"
    "flap_detection_enabled=" << obj.get_flap_detection_enabled() << "\n"
    "has_been_checked=" << obj.get_has_been_checked() << "\n"
    "is_flapping=" << obj.get_is_flapping() << "\n"
    "last_acknowledgement=" << obj.get_last_acknowledgement() << "\n"
    "last_check=" << static_cast<unsigned long>(obj.get_last_check()) << "\n"
    "last_event_id=" << obj.get_last_event_id() << "\n"
    "last_hard_state=" << obj.get_last_hard_state() << "\n"
    "last_hard_state_change=" << static_cast<unsigned long>(obj.get_last_hard_state_change()) << "\n"
    "last_notification=" << static_cast<unsigned long>(obj.get_last_notification()) << "\n"
    "last_problem_id=" << obj.get_last_problem_id() << "\n"
    "last_state=" << obj.get_last_state() << "\n"
    "last_state_change=" << static_cast<unsigned long>(obj.get_last_state_change()) << "\n"
    "last_time_down=" << static_cast<unsigned long>(obj.get_last_time_down()) << "\n"
    "last_time_unreachable=" << static_cast<unsigned long>(obj.get_last_time_unreachable()) << "\n"
    "last_time_up=" << static_cast<unsigned long>(obj.get_last_time_up()) << "\n"
    "long_plugin_output=" << obj.get_long_plugin_output() << "\n"
    "max_attempts=" << obj.get_max_attempts() << "\n"
    "modified_attributes=" << (obj.get_modified_attributes() & ~config->retained_host_attribute_mask()) << "\n"
    "next_check=" << static_cast<unsigned long>(obj.get_next_check()) << "\n"
    "normal_check_interval=" << obj.get_check_interval() << "\n"
    "notification_period=" << obj.get_notification_period() << "\n"
    "notifications_enabled=" << obj.get_notifications_enabled() << "\n"
    "notified_on_down=" << obj.get_notified_on(notifier::down) << "\n"
    "notified_on_unreachable=" << obj.get_notified_on(notifier::unreachable) << "\n"
    "obsess_over_host=" << obj.get_obsess_over() << "\n"
    "passive_checks_enabled=" << obj.get_accept_passive_checks() << "\n"
    "percent_state_change=" << std::setprecision(2) << std::fixed << obj.get_percent_state_change() << "\n"
    "performance_data=" << obj.get_perf_data() << "\n"
    "plugin_output=" << obj.get_plugin_output() << "\n"
    "problem_has_been_acknowledged=" << obj.get_problem_has_been_acknowledged() << "\n"
    "process_performance_data=" << obj.get_process_performance_data() << "\n"
    "retry_check_interval=" << obj.get_check_interval() << "\n"
    "state_type=" << obj.get_state_type ()<< "\n";

  os << "state_history=";
  for (unsigned int x(0); x < obj.get_state_history().size(); ++x)
    os << (x > 0 ? "," : "") << obj.get_state_history()[(x + obj.get_state_history_index()) % MAX_STATE_HISTORY_ENTRIES];
  os << "\n";

  dump::customvariables(os, obj.custom_variables);
  os << "}\n";
  return os;
}

/**
 *  Dump retention of hosts.
 *
 *  @param[out] os The output stream.
 *
 *  @return The output stream.
 */
std::ostream& dump::hosts(std::ostream& os) {
  for (host_map::iterator
         it(com::centreon::engine::host::hosts.begin()),
         end(com::centreon::engine::host::hosts.end());
       it != end;
       ++it)
    dump::host(os, *it->second);
  return os;
}

/**
 *  Dump retention of info.
 *
 *  @param[out] os The output stream.
 *
 *  @return The output stream.
 */
std::ostream& dump::info(std::ostream& os) {
  os << "info {\n"
    "created=" << static_cast<unsigned long>(time(NULL)) << "\n"
    "}\n";
  return os;
}

/**
 *  Dump retention of program.
 *
 *  @param[out] os The output stream.
 *
 *  @return The output stream.
 */
std::ostream& dump::program(std::ostream& os) {
  os << "program {\n"
    "active_host_checks_enabled=" << config->execute_host_checks() << "\n"
    "active_service_checks_enabled=" << config->execute_service_checks() << "\n"
    "check_host_freshness=" << config->check_host_freshness() << "\n"
    "check_service_freshness=" << config->check_service_freshness() << "\n"
    "enable_event_handlers=" << config->enable_event_handlers() << "\n"
    "enable_flap_detection=" << config->enable_flap_detection() << "\n"
    "enable_notifications=" << config->enable_notifications() << "\n"
    "global_host_event_handler=" << config->global_host_event_handler().c_str() << "\n"
    "global_service_event_handler=" << config->global_service_event_handler().c_str() << "\n"
    "modified_host_attributes=" << (modified_host_process_attributes & ~config->retained_process_host_attribute_mask()) << "\n"
    "modified_service_attributes=" << (modified_service_process_attributes & ~config->retained_process_host_attribute_mask()) << "\n"
    "next_comment_id=" << comment::get_next_comment_id() << "\n"
    "next_event_id=" << next_event_id << "\n"
    "next_notification_id=" << next_notification_id << "\n"
    "next_problem_id=" << next_problem_id << "\n"
    "obsess_over_hosts=" << config->obsess_over_hosts() << "\n"
    "obsess_over_services=" << config->obsess_over_services() << "\n"
    "passive_host_checks_enabled=" << config->accept_passive_host_checks() << "\n"
    "passive_service_checks_enabled=" << config->accept_passive_service_checks() << "\n"
    "process_performance_data=" << config->process_performance_data() << "\n"
    "}\n";
  return os;
}

/**
 *  Save all data.
 *
 *  @param[in] path The file path to use to save.
 *
 *  @return True on success, otherwise false.
 */
bool dump::save(std::string const& path) {
  if (!config->retain_state_information())
    return true;

  // send data to event broker
  broker_retention_data(
    NEBTYPE_RETENTIONDATA_STARTSAVE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);

  bool ret(false);
  try {
    std::ofstream stream(
                    path.c_str(),
                    std::ios::binary | std::ios::trunc);
    if (!stream.is_open())
      throw (engine_error() << "Cannot open retention file '"
             << config->state_retention_file() << "'");
    dump::header(stream);
    dump::info(stream);
    dump::program(stream);
    dump::hosts(stream);
    dump::services(stream);
    dump::contacts(stream);
    dump::comments(stream);
    dump::downtimes(stream);

    ret = true;
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << e.what();
  }

  // send data to event broker.
  broker_retention_data(
    NEBTYPE_RETENTIONDATA_ENDSAVE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);
  return ret;
}

/**
 *  Dump retention of service.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The service to dump.
 *
 *  @return The output stream.
 */
std::ostream& dump::service(std::ostream& os, class service const& obj) {
  std::string hostname;
  if (obj.get_host_ptr())
    hostname = obj.get_host_ptr()->get_name();

  os << "service {\n"
    "host_name=" << obj.get_hostname() << "\n"
    "service_description=" << obj.get_description() << "\n"
    "host_id=" << service::services[{hostname, obj.get_description()}]->get_host_id() << "\n"
    "service_id=" << service::services[{hostname, obj.get_description()}]->get_service_id() << "\n"
    "acknowledgement_type=" << obj.get_acknowledgement_type() << "\n"
    "active_checks_enabled=" << obj.get_checks_enabled() << "\n"
    "check_command=" << obj.get_check_command() << "\n"
    "check_execution_time=" << std::setprecision(3) << std::fixed << obj.get_execution_time() << "\n"
    "check_flapping_recovery_notification=" << obj.get_check_flapping_recovery_notification() << "\n"
    "check_latency=" << std::setprecision(3) << std::fixed << obj.get_latency() << "\n"
    "check_options=" << obj.get_check_options() << "\n"
    "check_period=" << obj.get_check_period() << "\n"
    "check_type=" << obj.get_check_type() << "\n"
    "current_attempt=" << obj.get_current_attempt() << "\n"
    "current_event_id=" << obj.get_current_event_id() << "\n"
    "current_notification_id=" << obj.get_current_notification_id() << "\n"
    "current_notification_number=" << obj.get_notification_number() << "\n"
    "current_problem_id=" << obj.get_current_problem_id() << "\n"
    "current_state=" << obj.get_current_state() << "\n"
    "event_handler=" << obj.get_event_handler() << "\n"
    "event_handler_enabled=" << obj.get_event_handler_enabled() << "\n"
    "flap_detection_enabled=" << obj.get_flap_detection_enabled() << "\n"
    "has_been_checked=" << obj.get_has_been_checked() << "\n"
    "is_flapping=" << obj.get_is_flapping() << "\n"
    "last_acknowledgement=" << obj.get_last_acknowledgement() << "\n"
    "last_check=" << static_cast<unsigned long>(obj.get_last_check()) << "\n"
    "last_event_id=" << obj.get_last_event_id() << "\n"
    "last_hard_state=" << obj.get_last_hard_state() << "\n"
    "last_hard_state_change=" << static_cast<unsigned long>(obj.get_last_hard_state_change()) << "\n"
    "last_notification=" << static_cast<unsigned long>(obj.get_last_notification()) << "\n"
    "last_problem_id=" << obj.get_last_problem_id() << "\n"
    "last_state=" << obj.get_last_state() << "\n"
    "last_state_change=" << static_cast<unsigned long>(obj.get_last_state_change()) << "\n"
    "last_time_critical=" << static_cast<unsigned long>(obj.get_last_time_critical()) << "\n"
    "last_time_ok=" << static_cast<unsigned long>(obj.get_last_time_ok()) << "\n"
    "last_time_unknown=" << static_cast<unsigned long>(obj.get_last_time_unknown()) << "\n"
    "last_time_warning=" << static_cast<unsigned long>(obj.get_last_time_warning()) << "\n"
    "long_plugin_output=" << obj.get_long_plugin_output() << "\n"
    "max_attempts=" << obj.get_max_attempts() << "\n"
    "modified_attributes=" << (obj.get_modified_attributes() & ~config->retained_host_attribute_mask()) << "\n"
    "next_check=" << static_cast<unsigned long>(obj.get_next_check()) << "\n"
    "normal_check_interval=" << obj.get_check_interval() << "\n"
    "notification_period=" << obj.get_notification_period() << "\n"
    "notifications_enabled=" << obj.get_notifications_enabled() << "\n"
    "notified_on_critical=" << obj.get_notified_on(notifier::critical) << "\n"
    "notified_on_unknown=" << obj.get_notified_on(notifier::unknown) << "\n"
    "notified_on_warning=" << obj.get_notified_on(notifier::warning) << "\n"
    "obsess_over_service=" << obj.get_obsess_over() << "\n"
    "passive_checks_enabled=" << obj.get_accept_passive_checks() << "\n"
    "percent_state_change=" << std::setprecision(2) << std::fixed
                            << obj.get_percent_state_change() << "\n"
    "performance_data=" << obj.get_perf_data() << "\n"
    "plugin_output=" << obj.get_plugin_output() << "\n"
    "problem_has_been_acknowledged=" << obj.get_problem_has_been_acknowledged() << "\n"
    "process_performance_data=" << obj.get_process_performance_data() << "\n"
    "retry_check_interval=" << obj.get_retry_interval() << "\n"
    "state_type=" << obj.get_state_type() << "\n";

  os << "state_history=";
  for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x)
    os << (x > 0 ? "," : "") << obj.get_state_history()[(x + obj.get_state_history_index()) % MAX_STATE_HISTORY_ENTRIES];
  os << "\n";

  dump::customvariables(os, obj.custom_variables);
  os << "}\n";
  return os;
}

/**
 *  Dump retention of services.
 *
 *  @param[out] os The output stream.
 *
 *  @return The output stream.
 */
std::ostream& dump::services(std::ostream& os) {
  for (service_map::iterator
         it(service::services.begin()),
         end(service::services.end());
       it != end;
       ++it)
    dump::service(os, *it->second);
  return os;
}
