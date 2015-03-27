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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/host.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostsmember.hh"
#include "com/centreon/engine/objects/servicesmember.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       host const& obj1,
       host const& obj2) throw () {
  return (is_equal(obj1.name, obj2.name)
          && is_equal(obj1.display_name, obj2.display_name)
          && is_equal(obj1.alias, obj2.alias)
          && is_equal(obj1.address, obj2.address)
          && is_equal(obj1.parent_hosts, obj2.parent_hosts)
          // Children do not need to be tested, they are
          // created as parent back links.
          // Services do not need to be tested, they are
          // created as services back links.
          && is_equal(obj1.host_check_command, obj2.host_check_command)
          && obj1.initial_state == obj2.initial_state
          && obj1.check_interval == obj2.check_interval
          && obj1.retry_interval == obj2.retry_interval
          && obj1.max_attempts == obj2.max_attempts
          && obj1.check_timeout == obj2.check_timeout
          && is_equal(obj1.event_handler, obj2.event_handler)
          && is_equal(obj1.check_period, obj2.check_period)
          && obj1.flap_detection_enabled == obj2.flap_detection_enabled
          && obj1.low_flap_threshold == obj2.low_flap_threshold
          && obj1.high_flap_threshold == obj2.high_flap_threshold
          && obj1.flap_detection_on_up == obj2.flap_detection_on_up
          && obj1.flap_detection_on_down == obj2.flap_detection_on_down
          && obj1.flap_detection_on_unreachable == obj2.flap_detection_on_unreachable
          && obj1.stalk_on_up == obj2.stalk_on_up
          && obj1.stalk_on_down == obj2.stalk_on_down
          && obj1.stalk_on_unreachable == obj2.stalk_on_unreachable
          && obj1.check_freshness == obj2.check_freshness
          && obj1.freshness_threshold == obj2.freshness_threshold
          && obj1.checks_enabled == obj2.checks_enabled
          && obj1.accept_passive_host_checks == obj2.accept_passive_host_checks
          && obj1.event_handler_enabled == obj2.event_handler_enabled
          && obj1.retain_status_information == obj2.retain_status_information
          && obj1.retain_nonstatus_information == obj2.retain_nonstatus_information
          && obj1.obsess_over_host == obj2.obsess_over_host
          && obj1.should_be_drawn == obj2.should_be_drawn
          && is_equal(obj1.custom_variables, obj2.custom_variables)
          && obj1.check_type == obj2.check_type
          && obj1.current_state == obj2.current_state
          && obj1.last_state == obj2.last_state
          && obj1.last_hard_state == obj2.last_hard_state
          && is_equal(obj1.plugin_output, obj2.plugin_output)
          && is_equal(obj1.long_plugin_output, obj2.long_plugin_output)
          && is_equal(obj1.perf_data, obj2.perf_data)
          && obj1.state_type == obj2.state_type
          && obj1.current_attempt == obj2.current_attempt
          && obj1.current_event_id == obj2.current_event_id
          && obj1.last_event_id == obj2.last_event_id
          && obj1.current_problem_id == obj2.current_problem_id
          && obj1.last_problem_id == obj2.last_problem_id
          && obj1.latency == obj2.latency
          && obj1.execution_time == obj2.execution_time
          && obj1.is_executing == obj2.is_executing
          && obj1.check_options == obj2.check_options
          && obj1.next_check == obj2.next_check
          && obj1.should_be_scheduled == obj2.should_be_scheduled
          && obj1.last_check == obj2.last_check
          && obj1.last_state_change == obj2.last_state_change
          && obj1.last_hard_state_change == obj2.last_hard_state_change
          && obj1.last_time_up == obj2.last_time_up
          && obj1.last_time_down == obj2.last_time_down
          && obj1.last_time_unreachable == obj2.last_time_unreachable
          && obj1.has_been_checked == obj2.has_been_checked
          && obj1.is_being_freshened == obj2.is_being_freshened
          && is_equal(obj1.state_history, obj2.state_history, MAX_STATE_HISTORY_ENTRIES)
          && obj1.state_history_index == obj2.state_history_index
          && obj1.last_state_history_update == obj2.last_state_history_update
          && obj1.is_flapping == obj2.is_flapping
          && obj1.percent_state_change == obj2.percent_state_change
          && obj1.total_services == obj2.total_services
          && obj1.total_service_check_interval == obj2.total_service_check_interval
          && obj1.modified_attributes == obj2.modified_attributes
          && obj1.circular_path_checked == obj2.circular_path_checked
          && obj1.contains_circular_path == obj2.contains_circular_path
          && is_equal(obj1.timezone, obj2.timezone));
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       host const& obj1,
       host const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump host content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The host to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, host const& obj) {
  char const* evt_str(NULL);
  if (obj.event_handler_ptr)
    evt_str = chkstr(obj.event_handler_ptr->name);
  char const* cmd_str(NULL);
  if (obj.check_command_ptr)
    cmd_str = chkstr(obj.check_command_ptr->name);
  char const* chk_period_str(NULL);
  if (obj.check_period_ptr)
    chk_period_str = chkstr(obj.check_period_ptr->name);
  char const* hstgrp_str(NULL);
  if (obj.hostgroups_ptr)
    hstgrp_str = chkstr(static_cast<hostgroup const*>(obj.hostgroups_ptr->object_ptr)->group_name);

  os << "host {\n"
    "  name:                                 " << chkstr(obj.name) << "\n"
    "  display_name:                         " << chkstr(obj.display_name) << "\n"
    "  alias:                                " << chkstr(obj.alias) << "\n"
    "  address:                              " << chkstr(obj.address) << "\n"
    "  parent_hosts:                         " << chkobj(obj.parent_hosts) << "\n"
    "  child_hosts:                          " << chkobj(obj.child_hosts) << "\n"
    "  services:                             " << chkobj(obj.services) << "\n"
    "  host_check_command:                   " << chkstr(obj.host_check_command) << "\n"
    "  initial_state:                        " << obj.initial_state << "\n"
    "  check_interval:                       " << obj.check_interval << "\n"
    "  retry_interval:                       " << obj.retry_interval << "\n"
    "  max_attempts:                         " << obj.max_attempts << "\n"
    "  check_timeout                         " << obj.check_timeout << "\n"
    "  event_handler:                        " << chkstr(obj.event_handler) << "\n"
    "  check_period:                         " << chkstr(obj.check_period) << "\n"
    "  flap_detection_enabled:               " << obj.flap_detection_enabled << "\n"
    "  low_flap_threshold:                   " << obj.low_flap_threshold << "\n"
    "  high_flap_threshold:                  " << obj.high_flap_threshold << "\n"
    "  flap_detection_on_up:                 " << obj.flap_detection_on_up << "\n"
    "  flap_detection_on_down:               " << obj.flap_detection_on_down << "\n"
    "  flap_detection_on_unreachable:        " << obj.flap_detection_on_unreachable << "\n"
    "  stalk_on_up:                          " << obj.stalk_on_up << "\n"
    "  stalk_on_down:                        " << obj.stalk_on_down << "\n"
    "  stalk_on_unreachable:                 " << obj.stalk_on_unreachable << "\n"
    "  check_freshness:                      " << obj.check_freshness << "\n"
    "  freshness_threshold:                  " << obj.freshness_threshold << "\n"
    "  checks_enabled:                       " << obj.checks_enabled << "\n"
    "  accept_passive_host_checks:           " << obj.accept_passive_host_checks << "\n"
    "  event_handler_enabled:                " << obj.event_handler_enabled << "\n"
    "  retain_status_information:            " << obj.retain_status_information << "\n"
    "  retain_nonstatus_information:         " << obj.retain_nonstatus_information << "\n"
    "  obsess_over_host:                     " << obj.obsess_over_host << "\n"
    "  should_be_drawn:                      " << obj.should_be_drawn << "\n"
    "  check_type:                           " << obj.check_type << "\n"
    "  current_state:                        " << obj.current_state << "\n"
    "  last_state:                           " << obj.last_state << "\n"
    "  last_hard_state:                      " << obj.last_hard_state << "\n"
    "  plugin_output:                        " << chkstr(obj.plugin_output) << "\n"
    "  long_plugin_output:                   " << chkstr(obj.long_plugin_output) << "\n"
    "  perf_data:                            " << chkstr(obj.perf_data) << "\n"
    "  state_type:                           " << obj.state_type << "\n"
    "  current_attempt:                      " << obj.current_attempt << "\n"
    "  current_event_id:                     " << obj.current_event_id << "\n"
    "  last_event_id:                        " << obj.last_event_id << "\n"
    "  current_problem_id:                   " << obj.current_problem_id << "\n"
    "  last_problem_id:                      " << obj.last_problem_id << "\n"
    "  latency:                              " << obj.latency << "\n"
    "  execution_time:                       " << obj.execution_time << "\n"
    "  is_executing:                         " << obj.is_executing << "\n"
    "  check_options:                        " << obj.check_options << "\n"
    "  next_check:                           " << string::ctime(obj.next_check) << "\n"
    "  should_be_scheduled:                  " << obj.should_be_scheduled << "\n"
    "  last_check:                           " << string::ctime(obj.last_check) << "\n"
    "  last_state_change:                    " << string::ctime(obj.last_state_change) << "\n"
    "  last_hard_state_change:               " << string::ctime(obj.last_hard_state_change) << "\n"
    "  last_time_up:                         " << string::ctime(obj.last_time_up) << "\n"
    "  last_time_down:                       " << string::ctime(obj.last_time_down) << "\n"
    "  last_time_unreachable:                " << string::ctime(obj.last_time_unreachable) << "\n"
    "  has_been_checked:                     " << obj.has_been_checked << "\n"
    "  is_being_freshened:                   " << obj.is_being_freshened << "\n"
    "  timezone:                             " << chkstr(obj.timezone) << "\n";

  os << "  state_history:                        ";
  for (unsigned int i(0), end(sizeof(obj.state_history) / sizeof(obj.state_history[0]));
       i < end;
       ++i)
    os << obj.state_history[i] << (i + 1 < end ? ", " : "\n");

  os <<
    "  state_history_index:                  " << obj.state_history_index << "\n"
    "  last_state_history_update:            " << string::ctime(obj.last_state_history_update) << "\n"
    "  is_flapping:                          " << obj.is_flapping << "\n"
    "  percent_state_change:                 " << obj.percent_state_change << "\n"
    "  total_services:                       " << obj.total_services << "\n"
    "  total_service_check_interval:         " << obj.total_service_check_interval << "\n"
    "  modified_attributes:                  " << obj.modified_attributes << "\n"
    "  circular_path_checked:                " << obj.circular_path_checked << "\n"
    "  contains_circular_path:               " << obj.contains_circular_path << "\n"
    "  event_handler_ptr:                    " << chkstr(evt_str) << "\n"
    "  check_command_ptr:                    " << chkstr(cmd_str) << "\n"
    "  check_period_ptr:                     " << chkstr(chk_period_str) << "\n"
    "  hostgroups_ptr:                       " << chkstr(hstgrp_str) << "\n"
    << (obj.custom_variables ? chkobj(obj.custom_variables) : "")
    << "}\n";
  return (os);
}

/**
 *  Add a new host definition.
 *
 *  @param[in] name                          Host name.
 *  @param[in] display_name                  Display name.
 *  @param[in] alias                         Host alias.
 *  @param[in] address                       Host address.
 *  @param[in] check_period                  Check period.
 *  @param[in] initial_state                 Initial host state.
 *  @param[in] check_interval                Normal check interval.
 *  @param[in] retry_interval                Retry check interval.
 *  @param[in] max_attempts                  Max check attempts.
 *  @param[in] check_timeout                 The check timeout.
 *  @param[in] check_command                 Active check command name.
 *  @param[in] checks_enabled                Are active checks enabled ?
 *  @param[in] accept_passive_checks         Can we submit passive check
 *                                           results ?
 *  @param[in] event_handler                 Event handler command name.
 *  @param[in] event_handler_enabled         Whether event handler is
 *                                           enabled or not.
 *  @param[in] flap_detection_enabled        Whether flap detection is
 *                                           enabled or not.
 *  @param[in] low_flap_threshold            Low flap threshold.
 *  @param[in] high_flap_threshold           High flap threshold.
 *  @param[in] flap_detection_on_up          Is flap detection enabled
 *                                           for up state ?
 *  @param[in] flap_detection_on_down        Is flap detection enabled
 *                                           for down state ?
 *  @param[in] flap_detection_on_unreachable Is flap detection enabled
 *                                           for unreachable state ?
 *  @param[in] stalk_on_up                   Stalk on up ?
 *  @param[in] stalk_on_down                 Stalk on down ?
 *  @param[in] stalk_on_unreachable          Stalk on unreachable ?
 *  @param[in] check_freshness               Whether or not freshness
 *                                           check is enabled.
 *  @param[in] freshness_threshold           Freshness threshold.
 *  @param[in] should_be_drawn               Whether this host should be
 *                                           drawn.
 *  @param[in] retain_status_information     Should Engine retain status
 *                                           information of this host ?
 *  @param[in] retain_nonstatus_information  Should Engine retain
 *                                           non-status information of
 *                                           this host ?
 *  @param[in] obsess_over_host              Should we obsess over this
 *                                           host ?
 *  @param[in] timezone                      Host timezone.
 *
 *  @return New host.
 */
host* add_host(
        char const* name,
        char const* display_name,
        char const* alias,
        char const* address,
        char const* check_period,
        int initial_state,
        double check_interval,
        double retry_interval,
        int max_attempts,
        unsigned int check_timeout,
        char const* check_command,
        int checks_enabled,
        int accept_passive_checks,
        char const* event_handler,
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
        int check_freshness,
        int freshness_threshold,
        int should_be_drawn,
        int retain_status_information,
        int retain_nonstatus_information,
        int obsess_over_host,
        char const* timezone) {
  // Make sure we have the data we need.
  if (!name || !name[0] || !address || !address[0]) {
    logger(log_config_error, basic)
      << "Error: Host name or address is NULL";
    return (NULL);
  }
  if (max_attempts <= 0) {
    logger(log_config_error, basic)
      << "Error: Invalid max_check_attempts value for host '"
      << name << "'";
    return (NULL);
  }
  if (check_interval < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid check_interval value for host '"
      << name << "'";
    return (NULL);
  }
  if (freshness_threshold < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid freshness_threshold value for host '"
      << name << "'";
    return (NULL);
  }

  // Check if the host is already exist.
  std::string id(name);
  if (is_host_exist(id)) {
    logger(log_config_error, basic)
      << "Error: Host '" << name << "' has already been defined";
    return (NULL);
  }

  // Allocate memory for a new host.
  shared_ptr<host> obj(new host, deleter::host);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate string vars.
    obj->name = string::dup(name);
    obj->address = string::dup(address);
    obj->alias = string::dup(alias ? alias : name);
    obj->display_name = string::dup(display_name ? display_name : name);
    if (check_period)
      obj->check_period = string::dup(check_period);
    if (event_handler)
      obj->event_handler = string::dup(event_handler);
    if (check_command)
      obj->host_check_command = string::dup(check_command);
    if (timezone)
      obj->timezone = string::dup(timezone);

    // Duplicate non-string vars.
    obj->accept_passive_host_checks = (accept_passive_checks > 0);
    obj->check_freshness = (check_freshness > 0);
    obj->check_interval = check_interval;
    obj->check_options = CHECK_OPTION_NONE;
    obj->check_type = HOST_CHECK_ACTIVE;
    obj->checks_enabled = (checks_enabled > 0);
    obj->check_timeout = check_timeout;
    obj->current_attempt = (initial_state == HOST_UP) ? 1 : max_attempts;
    obj->current_state = initial_state;
    obj->event_handler_enabled = (event_handler_enabled > 0);
    obj->flap_detection_enabled = (flap_detection_enabled > 0);
    obj->flap_detection_on_down = (flap_detection_on_down > 0);
    obj->flap_detection_on_unreachable = (flap_detection_on_unreachable > 0);
    obj->flap_detection_on_up = (flap_detection_on_up > 0);
    obj->freshness_threshold = freshness_threshold;
    obj->high_flap_threshold = high_flap_threshold;
    obj->initial_state = initial_state;
    obj->last_hard_state = initial_state;
    obj->last_state = initial_state;
    obj->low_flap_threshold = low_flap_threshold;
    obj->max_attempts = max_attempts;
    obj->modified_attributes = MODATTR_NONE;
    obj->obsess_over_host = (obsess_over_host > 0);
    obj->retain_nonstatus_information = (retain_nonstatus_information > 0);
    obj->retain_status_information = (retain_status_information > 0);
    obj->retry_interval = retry_interval;
    obj->should_be_drawn = (should_be_drawn > 0);
    obj->should_be_scheduled = true;
    obj->stalk_on_down = (stalk_on_down > 0);
    obj->stalk_on_unreachable = (stalk_on_unreachable > 0);
    obj->stalk_on_up = (stalk_on_up > 0);
    obj->state_type = HARD_STATE;

    // STATE_OK = 0, so we don't need to set state_history (memset
    // is used before).
    // for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x)
    //   obj->state_history[x] = STATE_OK;

    // Add new items to the configuration state.
    state::instance().hosts()[id] = obj;

    // Add new items to the list.
    obj->next = host_list;
    host_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_host_data(
      NEBTYPE_HOST_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Get number of registered hosts.
 *
 *  @return Number of registered hosts.
 */
int get_host_count() {
  return (state::instance().hosts().size());
}

/**
 *  Determines whether or not a specific host is an immediate child of
 *  another host.
 *
 *  @param[in] parent_host Parent host.
 *  @param[in] child_host  Child host.
 *
 *  @return true or false.
 */
int is_host_immediate_child_of_host(
      host* parent_host,
      host* child_host) {
  // Not enough data.
  if (!child_host)
    return (false);

  // Root/top-level hosts.
  if (!parent_host) {
    if (!child_host->parent_hosts)
      return (true);
  }
  // Mid-level/bottom hosts.
  else {
    for (hostsmember* member(child_host->parent_hosts);
         member;
         member = member->next)
      if (member->host_ptr == parent_host)
        return (true);
  }

  return (false);
}

/**
 *  Determines whether or not a specific host is an immediate parent of
 *  another host.
 *
 *  @param[in] child_host  Child host.
 *  @param[in] parent_host Parent host.
 *
 *  @return true or false.
 */
int is_host_immediate_parent_of_host(
      host* child_host,
      host* parent_host) {
  if (is_host_immediate_child_of_host(parent_host, child_host) == true)
    return (true);
  return (false);
}

/**
 *  Returns a count of the immediate children for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of immediate child hosts.
 */
int number_of_immediate_child_hosts(host* hst) {
  int children(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_child_of_host(hst, tmp))
      ++children;
  return (children);
}

/**
 *  Get the number of immediate parent hosts for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of immediate parent hosts.
 */
int number_of_immediate_parent_hosts(host* hst) {
  int parents(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_parent_of_host(hst, tmp))
      ++parents;
  return (parents);
}

/**
 *  Returns a count of the total children for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of total child hosts.
 */
int number_of_total_child_hosts(host* hst) {
  int children(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_child_of_host(hst, tmp))
      children += number_of_total_child_hosts(tmp) + 1;
  return (children);
}

/**
 *  Get the total number of parent hosts for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of total parent hosts.
 */
int number_of_total_parent_hosts(host* hst) {
  int parents(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_parent_of_host(hst, tmp))
      parents += number_of_total_parent_hosts(tmp) + 1;
  return (parents);
}

/**
 *  Get host by name.
 *
 *  @param[in] name The host name.
 *
 *  @return The struct host or throw exception if the
 *          host is not found.
 */
host& engine::find_host(std::string const& name) {
  umap<std::string, shared_ptr<host_struct> >::const_iterator
    it(state::instance().hosts().find(name));
  if (it == state::instance().hosts().end())
    throw (engine_error() << "Host '" << name << "' was not found");
  return (*it->second);
}

/**
 *  Get if host exist.
 *
 *  @param[in] name The host name.
 *
 *  @return True if the host is found, otherwise false.
 */
bool engine::is_host_exist(std::string const& name) throw () {
  umap<std::string, shared_ptr<host_struct> >::const_iterator
    it(state::instance().hosts().find(name));
  return (it != state::instance().hosts().end());
}
