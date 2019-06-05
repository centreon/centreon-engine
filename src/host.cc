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

#include <cassert>
#include <iomanip>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/checks/viability_failure.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/hash_timed_event.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/sehandlers.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/timezone_locker.hh"
#include "com/centreon/engine/xpddefault.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::events;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

std::array<std::pair<uint32_t, std::string>, 3> const host::tab_host_states{
    {{NSLOG_HOST_UP, "UP"},
     {NSLOG_HOST_DOWN, "DOWN"},
     {NSLOG_HOST_UNREACHABLE, "UNREACHABLE"}}};

host_map com::centreon::engine::host::hosts;

/*
 *  @param[in] name                          Host name.
 *  @param[in] display_name                  Display name.
 *  @param[in] alias                         Host alias.
 *  @param[in] address                       Host address.
 *  @param[in] check_period                  Check period.
 *  @param[in] initial_state                 Initial host state.
 *  @param[in] check_interval                Normal check interval.
 *  @param[in] retry_interval                Retry check interval.
 *  @param[in] max_attempts                  Max check attempts.
 *  @param[in] notify_up                     Does this host notify when
 *                                           up ?
 *  @param[in] notify_down                   Does this host notify when
 *                                           down ?
 *  @param[in] notify_unreachable            Does this host notify when
 *                                           unreachable ?
 *  @param[in] notify_flapping               Does this host notify for
 *                                           flapping ?
 *  @param[in] notify_downtime               Does this host notify for
 *                                           downtimes ?
 *  @param[in] notification_interval         Notification interval.
 *  @param[in] first_notification_delay      First notification delay.
 *  @param[in] notification_period           Notification period.
 *  @param[in] notifications_enabled         Whether notifications are
 *                                           enabled for this host.
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
 *  @param[in] process_perfdata              Should host perfdata be
 *                                           processed ?
 *  @param[in] check_freshness               Whether or not freshness
 *                                           check is enabled.
 *  @param[in] freshness_threshold           Freshness threshold.
 *  @param[in] notes                         Notes.
 *  @param[in] notes_url                     URL.
 *  @param[in] action_url                    Action URL.
 *  @param[in] icon_image                    Icon image.
 *  @param[in] icon_image_alt                Alternative icon image.
 *  @param[in] vrml_image                    VRML image.
 *  @param[in] statusmap_image               Status-map image.
 *  @param[in] x_2d                          2D x-coord.
 *  @param[in] y_2d                          2D y-coord.
 *  @param[in] have_2d_coords                Whether host has 2D coords.
 *  @param[in] x_3d                          3D x-coord.
 *  @param[in] y_3d                          3D y-coord.
 *  @param[in] z_3d                          3D z-coord.
 *  @param[in] have_3d_coords                Whether host has 3D coords.
 *  @param[in] should_be_drawn               Whether this host should be
 *                                           drawn.
 *  @param[in] retain_status_information     Should Engine retain status
 *                                           information of this host ?
 *  @param[in] retain_nonstatus_information  Should Engine retain
 *                                           non-status information of
 *                                           this host ?
 *  @param[in] obsess_over_host              Should we obsess over this
 *                                           host ?
 *  @param[in] timezone                      The timezone to apply to the host
 */
host::host(uint64_t host_id,
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
           bool notifications_enabled,
           std::string const& check_command,
           int checks_enabled,
           int accept_passive_checks,
           std::string const& event_handler,
           int event_handler_enabled,
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
           std::string const& timezone)
    : notifier{HOST_NOTIFICATION,
               !display_name.empty() ? display_name : name,
               check_command,
               checks_enabled > 0,
               initial_state,
               check_interval,
               retry_interval,
               max_attempts,
               first_notification_delay,
               notification_period,
               notifications_enabled,
               check_period,
               event_handler,
               notes,
               notes_url,
               action_url,
               icon_image,
               icon_image_alt,
               flap_detection_enabled,
               low_flap_threshold,
               high_flap_threshold,
               check_freshness,
               timezone} {
  // Make sure we have the data we need.
  if (name.empty() || address.empty()) {
    logger(log_config_error, basic) << "Error: Host name or address is nullptr";
    throw(engine_error() << "Could not register host '" << name << "'");
  }
  if (host_id == 0) {
    logger(log_config_error, basic) << "Error: Host must contain a host id "
                                       "because it comes from a database";
    throw(engine_error() << "Could not register host '" << name << "'");
  }
  if (notification_interval < 0) {
    logger(log_config_error, basic)
        << "Error: Invalid notification_interval value for host '" << name
        << "'";
    throw(engine_error() << "Could not register host '" << name << "'");
  }
  if (freshness_threshold < 0) {
    logger(log_config_error, basic)
        << "Error: Invalid freshness_threshold value for host '" << name << "'";
    throw(engine_error() << "Could not register host '" << name << "'");
  }

  // Check if the host already exists.
  uint64_t id{host_id};
  if (is_host_exist(id)) {
    logger(log_config_error, basic)
        << "Error: Host '" << name << "' has already been defined";
    throw(engine_error() << "Could not register host '" << name << "'");
  }

  _should_be_scheduled = true;
  _acknowledgement_type = ACKNOWLEDGEMENT_NONE;
  _check_options = CHECK_OPTION_NONE;
  _modified_attributes = MODATTR_NONE;
  _state_type = HARD_STATE;

  // Duplicate string vars.
  _name = name;
  _address = address;
  _alias = !alias.empty() ? alias : name;
  _statusmap_image = statusmap_image;
  _vrml_image = vrml_image;

  _accept_passive_host_checks = (accept_passive_checks > 0);
  set_current_attempt(initial_state == HOST_UP ? 1 : max_attempts);
  _current_state = initial_state;
  _event_handler_enabled = (event_handler_enabled > 0);
  _flap_detection_on_down = (flap_detection_on_down > 0);
  _flap_detection_on_unreachable = (flap_detection_on_unreachable > 0);
  _flap_detection_on_up = (flap_detection_on_up > 0);
  _freshness_threshold = freshness_threshold;
  _have_2d_coords = (have_2d_coords > 0);
  _have_3d_coords = (have_3d_coords > 0);
  _last_hard_state = initial_state;
  _last_state = initial_state;
  _notification_interval = notification_interval;
  _out_notification_type = notifier::none;
  _out_notification_type |= (notify_down > 0 ? notifier::down : 0);
  _out_notification_type |= (notify_downtime > 0 ? notifier::downtime : 0);
  _out_notification_type |= (notify_flapping > 0 ? notifier::flapping : 0);
  _out_notification_type |= (notify_up > 0 ? notifier::recovery : 0);
  _out_notification_type |=
      (notify_unreachable > 0 ? notifier::unreachable : 0);
  _obsess_over_host = (obsess_over_host > 0);
  _process_performance_data = (process_perfdata > 0);
  _retain_nonstatus_information = (retain_nonstatus_information > 0);
  _retain_status_information = (retain_status_information > 0);
  _should_be_drawn = (should_be_drawn > 0);
  _stalk_on_down = (stalk_on_down > 0);
  _stalk_on_unreachable = (stalk_on_unreachable > 0);
  _stalk_on_up = (stalk_on_up > 0);
  _x_2d = x_2d;
  _x_3d = x_3d;
  _y_2d = y_2d;
  _y_3d = y_3d;
  _z_3d = z_3d;
}

void host::add_child_link(host* child) {
  // Make sure we have the data we need.
  if (!child)
    throw(engine_error() << "add child link called with nullptr ptr");

  child_hosts.insert({child->get_name(),
                      std::make_shared<com::centreon::engine::host>(*child)});

  // Notify event broker.
  timeval tv(get_broker_timestamp(nullptr));
  broker_relation_data(NEBTYPE_PARENT_ADD, NEBFLAG_NONE, NEBATTR_NONE, this,
                       nullptr, child, nullptr, &tv);
}

void host::add_parent_host(std::string const& host_name) {
  // Make sure we have the data we need.
  if (host_name.empty()) {
    logger(log_config_error, basic)
        << "add child link called with bad host_name";
    throw(engine_error() << "add child link called with bad host_name");
  }

  // A host cannot be a parent/child of itself.
  if (_name == host_name) {
    logger(log_config_error, basic)
        << "Error: Host '" << _name << "' cannot be a child/parent of itself";
    throw(engine_error() << "host is child/parent itself");
  }

  parent_hosts.insert({host_name, nullptr});
}

std::string const& host::get_name() const {
  return _name;
}

void host::set_name(std::string const& name) {
  _name = name;
}

std::string const& host::get_alias() const {
  return _alias;
}

void host::set_alias(std::string const& alias) {
  _alias = alias;
}

std::string const& host::get_address() const {
  return _address;
}

void host::set_address(std::string const& address) {
  _address = address;
}

bool host::get_flap_detection_on_up() const {
  return _flap_detection_on_up;
}

void host::set_flap_detection_on_up(bool flap_detection_on_up) {
  _flap_detection_on_up = flap_detection_on_up;
}

bool host::get_flap_detection_on_down() const {
  return _flap_detection_on_down;
}

void host::set_flap_detection_on_down(bool flap_detection_on_down) {
  _flap_detection_on_down = flap_detection_on_down;
}

bool host::get_flap_detection_on_unreachable() const {
  return _flap_detection_on_unreachable;
}

void host::set_flap_detection_on_unreachable(bool detection) {
  _flap_detection_on_unreachable = detection;
}

bool host::get_stalk_on_down() const {
  return _stalk_on_down;
}

void host::set_stalk_on_down(bool stalk) {
  _stalk_on_down = stalk;
}

bool host::get_stalk_on_unreachable() const {
  return _stalk_on_unreachable;
}

void host::set_stalk_on_unreachable(bool stalk) {
  _stalk_on_unreachable = stalk;
}

bool host::get_stalk_on_up() const {
  return _stalk_on_up;
}

void host::set_stalk_on_up(bool stalk) {
  _stalk_on_up = stalk;
}

int host::get_freshness_threshold() const {
  return _freshness_threshold;
}

void host::set_freshness_threshold(int freshness_threshold) {
  _freshness_threshold = freshness_threshold;
}

bool host::get_process_performance_data() const {
  return _process_performance_data;
}

void host::set_process_performance_data(bool process_performance_data) {
  _process_performance_data = process_performance_data;
}

int host::get_accept_passive_host_checks() const {
  return _accept_passive_host_checks;
}

void host::set_accept_passive_host_checks(bool accept_passive_host_checks) {
  _accept_passive_host_checks = accept_passive_host_checks;
}

bool host::get_event_handler_enabled() const {
  return _event_handler_enabled;
}

void host::set_event_handler_enabled(bool event_handler_enabled) {
  _event_handler_enabled = event_handler_enabled;
}

int host::get_retain_status_information() const {
  return _retain_status_information;
}

void host::set_retain_status_information(int retain_status_information) {
  _retain_status_information = retain_status_information;
}

int host::get_retain_nonstatus_information() const {
  return _retain_nonstatus_information;
}

void host::set_retain_nonstatus_information(int retain_nonstatus_information) {
  _retain_nonstatus_information = retain_nonstatus_information;
}

bool host::get_failure_prediction_enabled() const {
  return _failure_prediction_enabled;
}

void host::set_failure_prediction_enabled(bool failure_prediction_enabled) {
  _failure_prediction_enabled = failure_prediction_enabled;
}

int host::get_obsess_over_host() const {
  return _obsess_over_host;
}

void host::set_obsess_over_host(int obsess_over_host) {
  _obsess_over_host = obsess_over_host;
}

std::string const& host::get_vrml_image() const {
  return _vrml_image;
}

void host::set_vrml_image(std::string const& image) {
  _vrml_image = image;
}

std::string const& host::get_statusmap_image() const {
  return _statusmap_image;
}

void host::set_statusmap_image(std::string const& image) {
  _statusmap_image = image;
}

bool host::get_have_2d_coords() const {
  return _have_2d_coords;
}

void host::set_have_2d_coords(bool has_coords) {
  _have_2d_coords = has_coords;
}

bool host::get_have_3d_coords() const {
  return _have_3d_coords;
}

void host::set_have_3d_coords(bool has_coords) {
  _have_3d_coords = has_coords;
}

int host::get_x_2d() const {
  return _x_2d;
}

void host::set_x_2d(int x) {
  _x_2d = x;
}

int host::get_y_2d() const {
  return _y_2d;
}

void host::set_y_2d(int y) {
  _y_2d = y;
}

int host::get_x_3d() const {
  return _x_3d;
}

void host::set_x_3d(int x) {
  _x_3d = x;
}

int host::get_y_3d() const {
  return _y_3d;
}

void host::set_y_3d(int y) {
  _y_3d = y;
}

int host::get_z_3d() const {
  return _z_3d;
}

void host::set_z_3d(int z) {
  _z_3d = z;
}

int host::get_should_be_drawn() const {
  return _should_be_drawn;
}

void host::set_should_be_drawn(int should_be_drawn) {
  _should_be_drawn = should_be_drawn;
}

int host::get_acknowledgement_type() const {
  return _acknowledgement_type;
}

void host::set_acknowledgement_type(int acknowledgement_type) {
  _acknowledgement_type = acknowledgement_type;
}

int host::get_last_state() const {
  return _last_state;
}

void host::set_last_state(int last_state) {
  _last_state = last_state;
}

int host::get_last_hard_state() const {
  return _last_hard_state;
}

void host::set_last_hard_state(int last_hard_state) {
  _last_hard_state = last_hard_state;
}

int host::get_state_type() const {
  return _state_type;
}

void host::set_state_type(int state_type) {
  _state_type = state_type;
}

unsigned long host::get_current_event_id() const {
  return _current_event_id;
}

void host::set_current_event_id(unsigned long current_event_id) {
  _current_event_id = current_event_id;
}

unsigned long host::get_last_event_id() const {
  return _last_event_id;
}

void host::set_last_event_id(unsigned long last_event_id) {
  _last_event_id = last_event_id;
}

unsigned long host::get_current_problem_id() const {
  return _current_problem_id;
}

void host::set_current_problem_id(unsigned long current_problem_id) {
  _current_problem_id = current_problem_id;
}

unsigned long host::get_last_problem_id() const {
  return _last_problem_id;
}

void host::set_last_problem_id(unsigned long last_problem_id) {
  _last_problem_id = last_problem_id;
}

double host::get_latency() const {
  return _latency;
}

void host::set_latency(double latency) {
  _latency = latency;
}

double host::get_execution_time() const {
  return _execution_time;
}

void host::set_execution_time(double execution_time) {
  _execution_time = execution_time;
}

bool host::get_is_executing() const {
  return _is_executing;
}

void host::set_is_executing(bool is_executing) {
  _is_executing = is_executing;
}

int host::get_check_options() const {
  return _check_options;
}

void host::set_check_options(int check_options) {
  _check_options = check_options;
}

time_t host::get_next_check() const {
  return _next_check;
}

void host::set_next_check(time_t next_check) {
  _next_check = next_check;
}

int host::get_should_be_scheduled() const {
  return _should_be_scheduled;
}

void host::set_should_be_scheduled(int should_be_scheduled) {
  _should_be_scheduled = should_be_scheduled;
}

time_t host::get_last_check() const {
  return _last_check;
}

void host::set_last_check(time_t last_check) {
  _last_check = last_check;
}

time_t host::get_last_state_change() const {
  return _last_state_change;
}

void host::set_last_state_change(time_t last_state_change) {
  _last_state_change = last_state_change;
}

time_t host::get_last_hard_state_change() const {
  return _last_hard_state_change;
}

void host::set_last_hard_state_change(time_t last_hard_state_change) {
  _last_hard_state_change = last_hard_state_change;
}

time_t host::get_last_time_down() const {
  return _last_time_down;
}

void host::set_last_time_down(time_t last_time) {
  _last_time_down = last_time;
}

time_t host::get_last_time_unreachable() const {
  return _last_time_unreachable;
}

void host::set_last_time_unreachable(time_t last_time) {
  _last_time_unreachable = last_time;
}

time_t host::get_last_time_up() const {
  return _last_time_up;
}

void host::set_last_time_up(time_t last_time) {
  _last_time_up = last_time;
}

bool host::get_has_been_checked() const {
  return _has_been_checked;
}

void host::set_has_been_checked(bool has_been_checked) {
  _has_been_checked = has_been_checked;
}

bool host::get_is_being_freshened() const {
  return _is_being_freshened;
}

void host::set_is_being_freshened(bool is_being_freshened) {
  _is_being_freshened = is_being_freshened;
}

int host::get_current_notification_number() const {
  return _current_notification_number;
}

void host::set_current_notification_number(int current_notification_number) {
  _current_notification_number = current_notification_number;
}

int host::get_no_more_notifications() const {
  return _no_more_notifications;
}

void host::set_no_more_notifications(int no_more_notifications) {
  _no_more_notifications = no_more_notifications;
}

//
// void host::set_current_notification_id(unsigned long current_notification_id)
// {
//  _current_notification_id = current_notification_id;
//}

int host::get_check_flapping_recovery_notification() const {
  return _check_flapping_recovery_notification;
}

void host::set_check_flapping_recovery_notification(
    int check_flapping_recovery_notification) {
  _check_flapping_recovery_notification = check_flapping_recovery_notification;
}

int host::get_scheduled_downtime_depth() const {
  return _scheduled_downtime_depth;
}

void host::set_scheduled_downtime_depth(int scheduled_downtime_depth) {
  _scheduled_downtime_depth = scheduled_downtime_depth;
}

int host::get_pending_flex_downtime() const {
  return _pending_flex_downtime;
}

void host::set_pending_flex_downtime(int pending_flex_downtime) {
  _pending_flex_downtime = pending_flex_downtime;
}

unsigned int host::get_state_history_index() const {
  return _state_history_index;
}

void host::set_state_history_index(unsigned int state_history_index) {
  _state_history_index = state_history_index;
}

time_t host::get_last_state_history_update() const {
  return _last_state_history_update;
}

void host::set_last_state_history_update(time_t last_state_history_update) {
  _last_state_history_update = last_state_history_update;
}

bool host::get_is_flapping() const {
  return _is_flapping;
}

void host::set_is_flapping(bool is_flapping) {
  _is_flapping = is_flapping;
}

unsigned long host::get_flapping_comment_id() const {
  return _flapping_comment_id;
}

void host::set_flapping_comment_id(unsigned long flapping_comment_id) {
  _flapping_comment_id = flapping_comment_id;
}

double host::get_percent_state_change() const {
  return _percent_state_change;
}

void host::set_percent_state_change(double percent_state_change) {
  _percent_state_change = percent_state_change;
}

int host::get_total_services() const {
  return _total_services;
}

void host::set_total_services(int total_services) {
  _total_services = total_services;
}

unsigned long host::get_total_service_check_interval() const {
  return _total_service_check_interval;
}

void host::set_total_service_check_interval(
    unsigned long total_service_check_interval) {
  _total_service_check_interval = total_service_check_interval;
}

int host::get_circular_path_checked() const {
  return _circular_path_checked;
}

void host::set_circular_path_checked(int check_level) {
  _circular_path_checked = check_level;
}

bool host::get_contains_circular_path() const {
  return _contains_circular_path;
}

void host::set_contains_circular_path(bool contains_circular_path) {
  _contains_circular_path = contains_circular_path;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool host::operator==(host const& other) throw() {
  return get_name() == other.get_name() &&
         get_display_name() == other.get_display_name() &&
         get_alias() == other.get_alias() &&
         get_address() == other.get_address() &&
         ((parent_hosts.size() == other.parent_hosts.size()) &&
          std::equal(parent_hosts.begin(), parent_hosts.end(),
                     other.parent_hosts.begin()))
         // Children do not need to be tested, they are
         // created as parent back links.
         // Services do not need to be tested, they are
         // created as services back links.
         && get_check_command() == other.get_check_command() &&
         get_initial_state() == other.get_initial_state() &&
         get_check_interval() == other.get_check_interval() &&
         get_retry_interval() == other.get_retry_interval() &&
         get_max_attempts() == other.get_max_attempts() &&
         get_event_handler() == other.get_event_handler() &&
         ((contact_groups.size() == other.contact_groups.size()) &&
          std::equal(contact_groups.begin(), contact_groups.end(),
                     other.contact_groups.begin())) &&
         ((contacts.size() == other.contacts.size()) &&
          std::equal(contacts.begin(), contacts.end(),
                     other.contacts.begin())) &&
         get_notification_interval() == other.get_notification_interval() &&
         get_first_notification_delay() ==
             other.get_first_notification_delay() &&
         get_notify_on() == other.get_notify_on() &&
         get_notification_period() == other.get_notification_period() &&
         get_check_period() == other.get_check_period() &&
         get_flap_detection_enabled() == other.get_flap_detection_enabled() &&
         get_low_flap_threshold() == other.get_low_flap_threshold() &&
         get_high_flap_threshold() == other.get_high_flap_threshold() &&
         get_flap_detection_on_up() == other.get_flap_detection_on_up() &&
         get_flap_detection_on_down() == other.get_flap_detection_on_down() &&
         get_flap_detection_on_unreachable() ==
             other.get_flap_detection_on_unreachable() &&
         get_stalk_on_up() == other.get_stalk_on_up() &&
         get_stalk_on_down() == other.get_stalk_on_down() &&
         get_stalk_on_unreachable() == other.get_stalk_on_unreachable() &&
         get_check_freshness() == other.get_check_freshness() &&
         get_freshness_threshold() == other.get_freshness_threshold() &&
         get_process_performance_data() ==
             other.get_process_performance_data() &&
         get_checks_enabled() == other.get_checks_enabled() &&
         get_accept_passive_host_checks() ==
             other.get_accept_passive_host_checks() &&
         get_event_handler_enabled() == other.get_event_handler_enabled() &&
         get_retain_status_information() ==
             other.get_retain_status_information() &&
         get_retain_nonstatus_information() ==
             other.get_retain_nonstatus_information() &&
         get_obsess_over_host() == other.get_obsess_over_host() &&
         get_notes() == other.get_notes() &&
         get_notes_url() == other.get_notes_url() &&
         get_action_url() == other.get_action_url() &&
         get_icon_image() == other.get_icon_image() &&
         get_icon_image_alt() == other.get_icon_image_alt() &&
         get_vrml_image() == other.get_vrml_image() &&
         get_statusmap_image() == other.get_statusmap_image() &&
         get_have_2d_coords() == other.get_have_2d_coords() &&
         get_x_2d() == other.get_x_2d() && get_y_2d() == other.get_y_2d() &&
         get_have_3d_coords() == other.get_have_3d_coords() &&
         get_x_3d() == other.get_x_3d() && get_y_3d() == other.get_y_3d() &&
         get_z_3d() == other.get_z_3d() &&
         get_should_be_drawn() == other.get_should_be_drawn() &&
         custom_variables == other.custom_variables &&
         get_problem_has_been_acknowledged() ==
             other.get_problem_has_been_acknowledged() &&
         get_acknowledgement_type() == other.get_acknowledgement_type() &&
         get_check_type() == other.get_check_type() &&
         get_current_state() == other.get_current_state() &&
         get_last_state() == other.get_last_state() &&
         get_last_hard_state() == other.get_last_hard_state() &&
         get_plugin_output() == other.get_plugin_output() &&
         get_long_plugin_output() == other.get_long_plugin_output() &&
         get_perf_data() == other.get_perf_data() &&
         get_state_type() == other.get_state_type() &&
         get_current_attempt() == other.get_current_attempt() &&
         get_current_event_id() == other.get_current_event_id() &&
         get_last_event_id() == other.get_last_event_id() &&
         get_current_problem_id() == other.get_current_problem_id() &&
         get_last_problem_id() == other.get_last_problem_id() &&
         get_latency() == other.get_latency() &&
         get_execution_time() == other.get_execution_time() &&
         get_is_executing() == other.get_is_executing() &&
         get_check_options() == other.get_check_options() &&
         get_notifications_enabled() == other.get_notifications_enabled() &&
         get_last_notification() == other.get_last_notification() &&
         get_next_notification() == other.get_next_notification() &&
         get_next_check() == other.get_next_check() &&
         get_should_be_scheduled() == other.get_should_be_scheduled() &&
         get_last_check() == other.get_last_check() &&
         get_last_state_change() == other.get_last_state_change() &&
         get_last_hard_state_change() == other.get_last_hard_state_change() &&
         get_last_time_up() == other.get_last_time_up() &&
         get_last_time_down() == other.get_last_time_down() &&
         get_last_time_unreachable() == other.get_last_time_unreachable() &&
         get_has_been_checked() == other.get_has_been_checked() &&
         get_is_being_freshened() == other.get_is_being_freshened() &&
         get_notified_on() == other.get_notified_on() &&
         get_current_notification_number() ==
             other.get_current_notification_number() &&
         get_no_more_notifications() == other.get_no_more_notifications() &&
         get_current_notification_id() == other.get_current_notification_id() &&
         get_check_flapping_recovery_notification() ==
             other.get_check_flapping_recovery_notification() &&
         get_scheduled_downtime_depth() ==
             other.get_scheduled_downtime_depth() &&
         get_pending_flex_downtime() == other.get_pending_flex_downtime() &&
         is_equal(state_history, other.state_history,
                  MAX_STATE_HISTORY_ENTRIES) &&
         get_state_history_index() == other.get_state_history_index() &&
         get_last_state_history_update() ==
             other.get_last_state_history_update() &&
         get_is_flapping() == other.get_is_flapping() &&
         get_flapping_comment_id() == other.get_flapping_comment_id() &&
         get_percent_state_change() == other.get_percent_state_change() &&
         get_total_services() == other.get_total_services() &&
         get_total_service_check_interval() ==
             other.get_total_service_check_interval() &&
         get_modified_attributes() == other.get_modified_attributes() &&
         get_circular_path_checked() == other.get_circular_path_checked() &&
         get_contains_circular_path() == other.get_contains_circular_path();
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool host::operator!=(host const& other) throw() {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& os, host_map const& obj) {
  for (host_map::const_iterator it(obj.begin()), end(obj.end()); it != end;
       ++it) {
    os << it->first;
    if (next(it) != end)
      os << ", ";
    else
      os << "";
  }
  return os;
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
  com::centreon::engine::hostgroup const* hg =
      static_cast<hostgroup const*>(obj.hostgroups_ptr->object_ptr);

  std::string evt_str;
  if (obj.event_handler_ptr)
    evt_str = obj.event_handler_ptr->get_name();
  std::string cmd_str;
  if (obj.check_command_ptr)
    cmd_str = obj.check_command_ptr->get_name();
  std::string chk_period_str;
  if (obj.check_period_ptr)
    chk_period_str = obj.check_period_ptr->get_name();
  std::string notif_period_str;
  if (obj.notification_period_ptr)
    notif_period_str = obj.notification_period_ptr->get_name();

  std::string cg_oss;
  std::string c_oss;
  std::string p_oss;
  std::string child_oss;

  if (obj.contact_groups.empty())
    cg_oss = "\"NULL\"";
  else {
    std::ostringstream oss;
    oss << obj.contact_groups;
    cg_oss = oss.str();
  }
  if (obj.contacts.empty())
    c_oss = "\"NULL\"";
  else {
    std::ostringstream oss;
    oss << obj.contacts;
    c_oss = oss.str();
  }
  if (obj.parent_hosts.empty())
    c_oss = "\"NULL\"";
  else {
    std::ostringstream oss;
    oss << obj.parent_hosts;
    c_oss = oss.str();
  }

  if (obj.child_hosts.empty())
    child_oss = "\"NULL\"";
  else {
    std::ostringstream oss;
    oss << obj.child_hosts;
    child_oss = oss.str();
  }

  os << "host {\n"
        "  name:                                 "
     << obj.get_name()
     << "\n"
        "  display_name:                         "
     << obj.get_display_name()
     << "\n"
        "  alias:                                "
     << obj.get_alias()
     << "\n"
        "  address:                              "
     << obj.get_address()
     << "\n"
        "  parent_hosts:                         "
     << p_oss
     << "\n"
        "  child_hosts:                          "
     << child_oss
     << "\n"
        "  services:                             "
     << obj.services
     << "\n"
        "  host_check_command:                   "
     << obj.get_check_command()
     << "\n"
        "  initial_state:                        "
     << obj.get_initial_state()
     << "\n"
        "  check_interval:                       "
     << obj.get_check_interval()
     << "\n"
        "  retry_interval:                       "
     << obj.get_retry_interval()
     << "\n"
        "  max_attempts:                         "
     << obj.get_max_attempts()
     << "\n"
        "  event_handler:                        "
     << obj.get_event_handler()
     << "\n"
        "  contact_groups:                       "
     << cg_oss
     << "\n"
        "  contacts:                             "
     << c_oss
     << "\n"
        "  notification_interval:                "
     << obj.get_notification_interval()
     << "\n"
        "  first_notification_delay:             "
     << obj.get_first_notification_delay()
     << "\n"
        "  notify_on_down:                       "
     << obj.get_notify_on(notifier::down)
     << "\n"
        "  notify_on_unreachable:                "
     << obj.get_notify_on(notifier::unreachable)
     << "\n"
        "  notify_on_recovery:                   "
     << obj.get_notify_on(notifier::recovery)
     << "\n"
        "  notify_on_flapping:                   "
     << obj.get_notify_on(notifier::flapping)
     << "\n"
        "  notify_on_downtime:                   "
     << obj.get_notify_on(notifier::downtime)
     << "\n"
        "  notification_period:                  "
     << obj.get_notification_period()
     << "\n"
        "  check_period:                         "
     << obj.get_check_period()
     << "\n"
        "  flap_detection_enabled:               "
     << obj.get_flap_detection_enabled()
     << "\n"
        "  low_flap_threshold:                   "
     << obj.get_low_flap_threshold()
     << "\n"
        "  high_flap_threshold:                  "
     << obj.get_high_flap_threshold()
     << "\n"
        "  flap_detection_on_up:                 "
     << obj.get_flap_detection_on_up()
     << "\n"
        "  flap_detection_on_down:               "
     << obj.get_flap_detection_on_down()
     << "\n"
        "  flap_detection_on_unreachable:        "
     << obj.get_flap_detection_on_unreachable()
     << "\n"
        "  stalk_on_up:                          "
     << obj.get_stalk_on_up()
     << "\n"
        "  stalk_on_down:                        "
     << obj.get_stalk_on_down()
     << "\n"
        "  stalk_on_unreachable:                 "
     << obj.get_stalk_on_unreachable()
     << "\n"
        "  check_freshness:                      "
     << obj.get_check_freshness()
     << "\n"
        "  freshness_threshold:                  "
     << obj.get_freshness_threshold()
     << "\n"
        "  process_performance_data:             "
     << obj.get_process_performance_data()
     << "\n"
        "  checks_enabled:                       "
     << obj.get_checks_enabled()
     << "\n"
        "  accept_passive_host_checks:           "
     << obj.get_accept_passive_host_checks()
     << "\n"
        "  event_handler_enabled:                "
     << obj.get_event_handler_enabled()
     << "\n"
        "  retain_status_information:            "
     << obj.get_retain_status_information()
     << "\n"
        "  retain_nonstatus_information:         "
     << obj.get_retain_nonstatus_information()
     << "\n"
        "  obsess_over_host:                     "
     << obj.get_obsess_over_host()
     << "\n"
        "  notes:                                "
     << obj.get_notes()
     << "\n"
        "  notes_url:                            "
     << obj.get_notes_url()
     << "\n"
        "  action_url:                           "
     << obj.get_action_url()
     << "\n"
        "  icon_image:                           "
     << obj.get_icon_image()
     << "\n"
        "  icon_image_alt:                       "
     << obj.get_icon_image_alt()
     << "\n"
        "  vrml_image:                           "
     << obj.get_vrml_image()
     << "\n"
        "  statusmap_image:                      "
     << obj.get_statusmap_image()
     << "\n"
        "  have_2d_coords:                       "
     << obj.get_have_2d_coords()
     << "\n"
        "  x_2d:                                 "
     << obj.get_x_2d()
     << "\n"
        "  y_2d:                                 "
     << obj.get_y_2d()
     << "\n"
        "  have_3d_coords:                       "
     << obj.get_have_3d_coords()
     << "\n"
        "  x_3d:                                 "
     << obj.get_x_3d()
     << "\n"
        "  y_3d:                                 "
     << obj.get_y_3d()
     << "\n"
        "  z_3d:                                 "
     << obj.get_z_3d()
     << "\n"
        "  should_be_drawn:                      "
     << obj.get_should_be_drawn()
     << "\n"
        "  problem_has_been_acknowledged:        "
     << obj.get_problem_has_been_acknowledged()
     << "\n"
        "  acknowledgement_type:                 "
     << obj.get_acknowledgement_type()
     << "\n"
        "  check_type:                           "
     << obj.get_check_type()
     << "\n"
        "  current_state:                        "
     << obj.get_current_state()
     << "\n"
        "  last_state:                           "
     << obj.get_last_state()
     << "\n"
        "  last_hard_state:                      "
     << obj.get_last_hard_state()
     << "\n"
        "  plugin_output:                        "
     << obj.get_plugin_output()
     << "\n"
        "  long_plugin_output:                   "
     << obj.get_long_plugin_output()
     << "\n"
        "  perf_data:                            "
     << obj.get_perf_data()
     << "\n"
        "  state_type:                           "
     << obj.get_state_type()
     << "\n"
        "  current_attempt:                      "
     << obj.get_current_attempt()
     << "\n"
        "  current_event_id:                     "
     << obj.get_current_event_id()
     << "\n"
        "  last_event_id:                        "
     << obj.get_last_event_id()
     << "\n"
        "  current_problem_id:                   "
     << obj.get_current_problem_id()
     << "\n"
        "  last_problem_id:                      "
     << obj.get_last_problem_id()
     << "\n"
        "  latency:                              "
     << obj.get_latency()
     << "\n"
        "  execution_time:                       "
     << obj.get_execution_time()
     << "\n"
        "  is_executing:                         "
     << obj.get_is_executing()
     << "\n"
        "  check_options:                        "
     << obj.get_check_options()
     << "\n"
        "  notifications_enabled:                "
     << obj.get_notifications_enabled()
     << "\n"
        "  last_host_notification:               "
     << string::ctime(obj.get_last_notification())
     << "\n"
        "  next_host_notification:               "
     << string::ctime(obj.get_next_notification())
     << "\n"
        "  next_check:                           "
     << string::ctime(obj.get_next_check())
     << "\n"
        "  should_be_scheduled:                  "
     << obj.get_should_be_scheduled()
     << "\n"
        "  last_check:                           "
     << string::ctime(obj.get_last_check())
     << "\n"
        "  last_state_change:                    "
     << string::ctime(obj.get_last_state_change())
     << "\n"
        "  last_hard_state_change:               "
     << string::ctime(obj.get_last_hard_state_change())
     << "\n"
        "  last_time_up:                         "
     << string::ctime(obj.get_last_time_up())
     << "\n"
        "  last_time_down:                       "
     << string::ctime(obj.get_last_time_down())
     << "\n"
        "  last_time_unreachable:                "
     << string::ctime(obj.get_last_time_unreachable())
     << "\n"
        "  has_been_checked:                     "
     << obj.get_has_been_checked()
     << "\n"
        "  is_being_freshened:                   "
     << obj.get_is_being_freshened()
     << "\n"
        "  notified_on_down:                     "
     << obj.get_notified_on(notifier::down)
     << "\n"
        "  notified_on_unreachable:              "
     << obj.get_notified_on(notifier::unreachable)
     << "\n"
        "  current_notification_number:          "
     << obj.get_current_notification_number()
     << "\n"
        "  no_more_notifications:                "
     << obj.get_no_more_notifications()
     << "\n"
        "  current_notification_id:              "
     << obj.get_current_notification_id()
     << "\n"
        "  check_flapping_recovery_notification: "
     << obj.get_check_flapping_recovery_notification()
     << "\n"
        "  scheduled_downtime_depth:             "
     << obj.get_scheduled_downtime_depth()
     << "\n"
        "  pending_flex_downtime:                "
     << obj.get_pending_flex_downtime() << "\n";

  os << "  state_history:                        ";
  for (unsigned int i(0),
       end(sizeof(obj.state_history) / sizeof(obj.state_history[0]));
       i < end; ++i)
    os << obj.state_history[i] << (i + 1 < end ? ", " : "\n");

  os << "  state_history_index:                  "
     << obj.get_state_history_index()
     << "\n"
        "  last_state_history_update:            "
     << string::ctime(obj.get_last_state_history_update())
     << "\n"
        "  is_flapping:                          "
     << obj.get_is_flapping()
     << "\n"
        "  flapping_comment_id:                  "
     << obj.get_flapping_comment_id()
     << "\n"
        "  percent_state_change:                 "
     << obj.get_percent_state_change()
     << "\n"
        "  total_services:                       "
     << obj.get_total_services()
     << "\n"
        "  total_service_check_interval:         "
     << obj.get_total_service_check_interval()
     << "\n"
        "  modified_attributes:                  "
     << obj.get_modified_attributes()
     << "\n"
        "  circular_path_checked:                "
     << obj.get_circular_path_checked()
     << "\n"
        "  contains_circular_path:               "
     << obj.get_contains_circular_path()
     << "\n"
        "  event_handler_ptr:                    "
     << evt_str
     << "\n"
        "  check_command_ptr:                    "
     << cmd_str
     << "\n"
        "  check_period_ptr:                     "
     << chk_period_str
     << "\n"
        "  notification_period_ptr:              "
     << notif_period_str
     << "\n"
        "  hostgroups_ptr:                       "
     << hg->get_group_name() << "\n";

  for (std::pair<std::string, customvariable> const& cv : obj.custom_variables)
    os << cv.first << " ; ";

  os << "\n}\n";
  return os;
}

/**
 *  Tests whether a contact is a contact for a particular host.
 *
 *  @param[in] hst   Target host.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_contact_for_host(com::centreon::engine::host* hst, contact* cntct) {
  if (!hst || !cntct)
    return false;

  // Search all individual contacts of this host.
  for (contact_map::iterator it(hst->contacts.begin()),
       end(hst->contacts.end());
       it != end; ++it)
    if (it->second.get() == cntct)
      return true;

  for (contactgroup_map::iterator it(hst->contact_groups.begin()),
       end(hst->contact_groups.end());
       it != end; ++it)
    if (it->second->get_members().find(cntct->get_name()) ==
        it->second->get_members().end())
      return true;

  return false;
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
int is_host_immediate_child_of_host(com::centreon::engine::host* parent_host,
                                    com::centreon::engine::host* child_host) {
  // Not enough data.
  if (!child_host)
    return false;

  // Root/top-level hosts.
  if (!parent_host) {
    if (child_host->parent_hosts.size() == 0)
      return true;
  }
  // Mid-level/bottom hosts.
  else {
    for (host_map::iterator it(child_host->parent_hosts.begin()),
         end(child_host->parent_hosts.end());
         it != end; it++)
      if (it->second.get() == parent_host)
        return true;
  }

  return false;
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
int is_host_immediate_parent_of_host(com::centreon::engine::host* child_host,
                                     com::centreon::engine::host* parent_host) {
  if (is_host_immediate_child_of_host(parent_host, child_host))
    return true;
  return false;
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
int number_of_immediate_child_hosts(com::centreon::engine::host* hst) {
  int children(0);
  for (host_map::iterator it(host::hosts.begin()), end(host::hosts.end());
       it != end; ++it)
    if (is_host_immediate_child_of_host(hst, it->second.get()))
      ++children;
  return children;
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
int number_of_immediate_parent_hosts(com::centreon::engine::host* hst) {
  int parents(0);
  for (host_map::iterator it(host::hosts.begin()), end(host::hosts.end());
       it != end; ++it)
    if (is_host_immediate_parent_of_host(hst, it->second.get()))
      ++parents;
  return parents;
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
int number_of_total_child_hosts(com::centreon::engine::host* hst) {
  int children(0);
  for (host_map::iterator it(host::hosts.begin()), end(host::hosts.end());
       it != end; ++it)
    if (is_host_immediate_child_of_host(hst, it->second.get()))
      children += number_of_total_child_hosts(it->second.get()) + 1;
  return children;
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
int number_of_total_parent_hosts(com::centreon::engine::host* hst) {
  int parents(0);
  for (host_map::iterator it(host::hosts.begin()), end(host::hosts.end());
       it != end; ++it)
    if (is_host_immediate_parent_of_host(hst, it->second.get()))
      parents += number_of_total_parent_hosts(it->second.get()) + 1;
  return parents;
}

/**
 *  Get host by id.
 *
 *  @param[in] host_id The host id.
 *
 *  @return The struct host or throw exception if the
 *          host is not found.
 */
host& engine::find_host(uint64_t host_id) {
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it{state::instance().hosts().find(host_id)};
  if (it == state::instance().hosts().end())
    throw engine_error() << "Host '" << host_id << "' was not found";
  return *it->second;
}

/**
 *  Get if host exist.
 *
 *  @param[in] name The host name.
 *
 *  @return True if the host is found, otherwise false.
 */
bool engine::is_host_exist(uint64_t host_id) throw() {
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it(state::instance().hosts().find(host_id));
  return it != state::instance().hosts().end();
}

/**
 *  Get the id associated with a host.
 *
 *  @param[in] name  The name of the host.
 *
 *  @return  The host id or 0.
 */
uint64_t engine::get_host_id(std::string const& name) {
  std::map<std::string, host_other_properties>::const_iterator found =
      host_other_props.find(name);
  return found != host_other_props.end() ? found->second.host_id : 0;
}

/**
 *  Schedule acknowledgement expiration.
 *
 */
void host::schedule_acknowledgement_expiration() {
  if (_acknowledgement_timeout > 0 && _last_acknowledgement != (time_t)0) {
    schedule_new_event(EVENT_EXPIRE_HOST_ACK, false,
                       _last_acknowledgement + _acknowledgement_timeout, false,
                       0, nullptr, true, this, nullptr, 0);
  }
}

/**
 *  Log host event information.
 *  This function has been DEPRECATED.
 *
 *  @param[in] hst The host to log.
 *
 *  @return Return true on success.
 */
int host::log_event() {
  unsigned long log_options{NSLOG_HOST_UP};
  char const* state("UP");
  if (get_current_state() > 0 &&
      (unsigned int)get_current_state() < tab_host_states.size()) {
    log_options = tab_host_states[get_current_state()].first;
    state = tab_host_states[get_current_state()].second.c_str();
  }
  std::string const& state_type(tab_state_type[get_state_type()]);

  logger(log_options, basic)
      << "HOST ALERT: " << get_name() << ";" << state << ";" << state_type
      << ";" << get_current_attempt() << ";" << get_plugin_output();

  return OK;
}

/* process results of an asynchronous host check */
int host::handle_async_check_result_3x(check_result* queued_check_result) {
  time_t current_time;
  int result = STATE_OK;
  int reschedule_check = false;
  std::string old_plugin_output;
  struct timeval start_time_hires;
  struct timeval end_time_hires;
  double execution_time(0.0);

  logger(dbg_functions, basic) << "handle_async_host_check_result_3x()";

  /* make sure we have what we need */
  if (queued_check_result == NULL)
    return ERROR;

  time(&current_time);

  execution_time = (double)((double)(queued_check_result->finish_time.tv_sec -
                                     queued_check_result->start_time.tv_sec) +
                            (double)((queued_check_result->finish_time.tv_usec -
                                      queued_check_result->start_time.tv_usec) /
                                     1000.0) /
                                1000.0);
  if (execution_time < 0.0)
    execution_time = 0.0;

  logger(dbg_checks, more) << "** Handling async check result for host '"
                           << get_name() << "'...";

  logger(dbg_checks, most)
      << "\tCheck Type:         "
      << (queued_check_result->check_type == check_active ? "Active"
                                                          : "Passive")
      << "\n"
      << "\tCheck Options:      " << queued_check_result->check_options << "\n"
      << "\tScheduled Check?:   "
      << (queued_check_result->scheduled_check ? "Yes" : "No") << "\n"
      << "\tReschedule Check?:  "
      << (queued_check_result->reschedule_check ? "Yes" : "No") << "\n"
      << "\tShould Reschedule Current Host Check?:"
      << host_other_props[get_name()].should_reschedule_current_check
      << "\tExited OK?:         "
      << (queued_check_result->exited_ok ? "Yes" : "No") << "\n"
      << com::centreon::logging::setprecision(3)
      << "\tExec Time:          " << execution_time << "\n"
      << "\tLatency:            " << queued_check_result->latency << "\n"
      << "\treturn Status:      " << queued_check_result->return_code << "\n"
      << "\tOutput:             " << queued_check_result->output;

  /* decrement the number of host checks still out there... */
  if (queued_check_result->check_type == check_active &&
      currently_running_host_checks > 0)
    currently_running_host_checks--;

  /* skip this host check results if its passive and we aren't accepting passive
   * check results */
  if (queued_check_result->check_type == check_passive) {
    if (!config->accept_passive_host_checks()) {
      logger(dbg_checks, basic)
          << "Discarding passive host check result because passive host "
             "checks are disabled globally.";
      return ERROR;
    }
    if (!get_accept_passive_host_checks()) {
      logger(dbg_checks, basic)
          << "Discarding passive host check result because passive checks "
             "are disabled for this host.";
      return ERROR;
    }
  }

  /* clear the freshening flag (it would have been set if this host was
   * determined to be stale) */
  if (queued_check_result->check_options & CHECK_OPTION_FRESHNESS_CHECK)
    set_is_being_freshened(false);

  /* DISCARD INVALID FRESHNESS CHECK RESULTS */
  /* If a host goes stale, Engine will initiate a forced check in order
  ** to freshen it. There is a race condition whereby a passive check
  ** could arrive between the 1) initiation of the forced check and 2)
  ** the time when the forced check result is processed here. This would
  ** make the host fresh again, so we do a quick check to make sure the
  ** host is still stale before we accept the check result.
  */
  if ((queued_check_result->check_options & CHECK_OPTION_FRESHNESS_CHECK) &&
      is_host_result_fresh(this, current_time, false)) {
    logger(dbg_checks, basic)
        << "Discarding host freshness check result because the host is "
           "currently fresh (race condition avoided).";
    return OK;
  }

  /* was this check passive or active? */
  set_check_type((queued_check_result->check_type == check_active)
                     ? check_active
                     : check_passive);

  /* update check statistics for passive results */
  if (queued_check_result->check_type == check_passive)
    update_check_stats(PASSIVE_HOST_CHECK_STATS,
                       queued_check_result->start_time.tv_sec);

  /* should we reschedule the next check of the host? NOTE: this might be
   * overridden later... */
  reschedule_check = queued_check_result->reschedule_check;

  // Inherit the should reschedule flag from the host. It is used when
  // rescheduled checks were discarded because only one check can be executed
  // on the same host at the same time. The flag is then set in the host
  // and this check should be rescheduled regardless of what it was meant
  // to initially.
  if (host_other_props[get_name()].should_reschedule_current_check &&
      !queued_check_result->reschedule_check)
    reschedule_check = true;

  // Clear the should reschedule flag.
  host_other_props[get_name()].should_reschedule_current_check = false;

  /* check latency is passed to us for both active and passive checks */
  set_latency(queued_check_result->latency);

  /* update the execution time for this check (millisecond resolution) */
  set_execution_time(execution_time);

  /* set the checked flag */
  set_has_been_checked(true);

  /* clear the execution flag if this was an active check */
  if (queued_check_result->check_type == check_active)
    set_is_executing(false);

  /* get the last check time */
  set_last_check(queued_check_result->start_time.tv_sec);

  /* was this check passive or active? */
  set_check_type((queued_check_result->check_type == check_active)
                     ? check_active
                     : check_passive);

  /* save the old host state */
  set_last_state(get_current_state());
  if (get_state_type() == HARD_STATE)
    set_last_hard_state(get_current_state());

  /* save old plugin output */
  if (!get_plugin_output().empty())
    old_plugin_output = get_plugin_output();

  /* parse check output to get: (1) short output, (2) long output, (3) perf data
   */

  std::string output{queued_check_result->output};
  std::string plugin_output;
  std::string long_plugin_output;
  std::string perf_data;
  parse_check_output(output, plugin_output, long_plugin_output, perf_data, true,
                     true);
  set_plugin_output(plugin_output);
  set_long_plugin_output(long_plugin_output);
  set_perf_data(perf_data);

  /* make sure we have some data */
  if (get_plugin_output().empty()) {
    set_plugin_output("(No output returned from host check)");
  }

  /* replace semicolons in plugin output (but not performance data) with colons
   */
  std::string temp_str(get_plugin_output());
  std::replace(temp_str.begin(), temp_str.end(), ';', ':');
  set_plugin_output(temp_str);

  logger(dbg_checks, most)
      << "Parsing check output...\n"
      << "Short Output:\n"
      << (get_plugin_output().empty() ? "NULL" : get_plugin_output()) << "\n"
      << "Long Output:\n"
      << (get_long_plugin_output().empty() ? "NULL" : get_long_plugin_output())
      << "\n"
      << "Perf Data:\n"
      << (get_perf_data().empty() ? "NULL" : get_perf_data());

  /* get the unprocessed return code */
  /* NOTE: for passive checks, this is the final/processed state */
  result = queued_check_result->return_code;

  /* adjust return code (active checks only) */
  if (queued_check_result->check_type == check_active) {
    /* if there was some error running the command, just skip it (this shouldn't
     * be happening) */
    if (!queued_check_result->exited_ok) {
      logger(log_runtime_warning, basic)
          << "Warning:  Check of host '" << get_name()
          << "' did not exit properly!";

      set_plugin_output("(Host check did not exit properly)");
      set_long_plugin_output("");
      set_perf_data("");

      result = STATE_UNKNOWN;
    }

    /* make sure the return code is within bounds */
    else if (queued_check_result->return_code < 0 ||
             queued_check_result->return_code > 3) {
      logger(log_runtime_warning, basic)
          << "Warning: return (code of " << queued_check_result->return_code
          << " for check of host '" << get_name() << "' was out of bounds."
          << ((queued_check_result->return_code == 126 ||
               queued_check_result->return_code == 127)
                  ? " Make sure the plugin you're trying to run actually "
                    "exists."
                  : "");

      std::ostringstream oss;
      oss << "(Return code of " << queued_check_result->return_code
          << " is out of bounds"
          << ((queued_check_result->return_code == 126 ||
               queued_check_result->return_code == 127)
                  ? " - plugin may be missing"
                  : "")
          << ")";

      set_plugin_output(oss.str());
      set_long_plugin_output("");
      set_perf_data("");

      result = STATE_UNKNOWN;
    }

    /* a NULL host check command means we should assume the host is UP */
    if (get_check_command().empty()) {
      set_plugin_output("(Host assumed to be UP)");
      result = STATE_OK;
    }
  }

  /* translate return code to basic UP/DOWN state - the DOWN/UNREACHABLE state
   * determination is made later */
  /* NOTE: only do this for active checks - passive check results already have
   * the final state */
  if (queued_check_result->check_type == check_active) {
    /* if we're not doing aggressive host checking, let WARNING states indicate
     * the host is up (fake the result to be STATE_OK) */
    if (!config->use_aggressive_host_checking() && result == STATE_WARNING)
      result = STATE_OK;

    /* OK states means the host is UP */
    if (result == STATE_OK)
      result = HOST_UP;

    /* any problem state indicates the host is not UP */
    else
      result = HOST_DOWN;
  }

  /******************* PROCESS THE CHECK RESULTS ******************/

  /* process the host check result */
  process_host_check_result_3x(this, result,
                               const_cast<char*>(old_plugin_output.c_str()),
                               CHECK_OPTION_NONE, reschedule_check, true,
                               config->cached_host_check_horizon());

  logger(dbg_checks, more) << "** Async check result for host '" << get_name()
                           << "' handled: new state=" << get_current_state();

  /* high resolution start time for event broker */
  start_time_hires = queued_check_result->start_time;

  /* high resolution end time for event broker */
  gettimeofday(&end_time_hires, NULL);

  /* send data to event broker */
  broker_host_check(
      NEBTYPE_HOSTCHECK_PROCESSED, NEBFLAG_NONE, NEBATTR_NONE, this,
      get_check_type(), get_current_state(), get_state_type(), start_time_hires,
      end_time_hires, get_check_command().c_str(), get_latency(),
      get_execution_time(), config->host_check_timeout(),
      queued_check_result->early_timeout, queued_check_result->return_code,
      NULL, const_cast<char*>(get_plugin_output().c_str()),
      const_cast<char*>(get_long_plugin_output().c_str()),
      const_cast<char*>(get_perf_data().c_str()), NULL);
  return OK;
}

/* run a scheduled host check asynchronously */
int host::run_scheduled_check(int check_options, double latency) {
  int result = OK;
  time_t current_time = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int time_is_valid = true;

  logger(dbg_functions, basic) << "run_scheduled_host_check_3x()";

  logger(dbg_checks, basic)
      << "Attempting to run scheduled check of host '" << get_name()
      << "': check options=" << check_options << ", latency=" << latency;

  /* attempt to run the check */
  result = run_async_check(check_options, latency, true, true, &time_is_valid,
                           &preferred_time);

  /* an error occurred, so reschedule the check */
  if (result == ERROR) {
    logger(dbg_checks, more)
        << "Unable to run scheduled host check at this time";

    /* only attempt to (re)schedule checks that should get checked... */
    if (get_should_be_scheduled()) {
      /* get current time */
      time(&current_time);

      /* determine next time we should check the host if needed */
      /* if host has no check interval, schedule it again for 5 minutes from now
       */
      if (current_time >= preferred_time)
        preferred_time = current_time +
                         static_cast<time_t>((get_check_interval() <= 0)
                                                 ? 300
                                                 : (get_check_interval() *
                                                    config->interval_length()));

      // Make sure we rescheduled the next host check at a valid time.
      {
        timezone_locker lock(get_timezone());
        get_next_valid_time(preferred_time, &next_valid_time,
                            this->check_period_ptr);
      }

      /* the host could not be rescheduled properly - set the next check time
       * for next week */
      if (!time_is_valid && next_valid_time == preferred_time) {
        /*
          get_next_check()=(time_t)(next_valid_time+(60*60*24*365));
          get_should_be_scheduled()=false;
        */

        set_next_check((time_t)(next_valid_time + (60 * 60 * 24 * 7)));

        logger(log_runtime_warning, basic)
            << "Warning: Check of host '" << get_name()
            << "' could not be "
               "rescheduled properly.  Scheduling check for next week...";

        logger(dbg_checks, more)
            << "Unable to find any valid times to reschedule the next"
               " host check!";
      }
      /* this service could be rescheduled... */
      else {
        set_next_check(next_valid_time);
        set_should_be_scheduled(true);

        logger(dbg_checks, more)
            << "Rescheduled next host check for " << my_ctime(&next_valid_time);
      }
    }

    /* update the status log */
    update_status(false);

    /* reschedule the next host check - unless we couldn't find a valid next
     * check time */
    /* 10/19/07 EG - keep original check options */
    if (get_should_be_scheduled())
      schedule_check(get_next_check(), check_options);

    return ERROR;
  }
  return OK;
}

/* perform an asynchronous check of a host */
/* scheduled host checks will use this, as will some checks that result from
 * on-demand checks... */
int host::run_async_check(int check_options,
                          double latency,
                          int scheduled_check,
                          int reschedule_check,
                          int* time_is_valid,
                          time_t* preferred_time) {
  try {
    checks::checker::instance().run(this, check_options, latency,
                                    scheduled_check, reschedule_check,
                                    time_is_valid, preferred_time);
  } catch (checks::viability_failure const& e) {
    // Do not log viability failures.
    (void)e;
    return ERROR;
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "Error: " << e.what();
    return ERROR;
  }
  return OK;
}

/* schedules an immediate or delayed host check */
void host::schedule_check(time_t check_time, int options) {
  timed_event* temp_event = NULL;
  timed_event* new_event = NULL;
  int use_original_event = true;

  logger(dbg_functions, basic) << "schedule_host_check()";

  logger(dbg_checks, basic)
      << "Scheduling a "
      << (options & CHECK_OPTION_FORCE_EXECUTION ? "forced" : "non-forced")
      << ", active check of host '" << get_name() << "' @ "
      << my_ctime(&check_time);

  /* don't schedule a check if active checks of this host are disabled */
  if (!get_checks_enabled() && !(options & CHECK_OPTION_FORCE_EXECUTION)) {
    logger(dbg_checks, basic) << "Active checks are disabled for this host.";
    return;
  }
  /* allocate memory for a new event item */
  new_event = new timed_event;

  /* default is to use the new event */
  use_original_event = false;

#ifdef PERFORMANCE_INCREASE_BUT_VERY_BAD_IDEA_INDEED
  /* WARNING! 1/19/07 on-demand async host checks will end up causing mutliple
   * scheduled checks of a host to appear in the queue if the code below is
   * skipped */
  /* if(use_large_installation_tweaks==false)... skip code below */
#endif

  /* see if there are any other scheduled checks of this host in the queue */
  temp_event = quick_timed_event.find(hash_timed_event::low,
                                      hash_timed_event::host_check, this);

  /* we found another host check event for this host in the queue - what should
   * we do? */
  if (temp_event != NULL) {
    logger(dbg_checks, most)
        << "Found another host check event for this host @ "
        << my_ctime(&temp_event->run_time);

    /* use the originally scheduled check unless we decide otherwise */
    use_original_event = true;

    /* the original event is a forced check... */
    if ((temp_event->event_options & CHECK_OPTION_FORCE_EXECUTION)) {
      /* the new event is also forced and its execution time is earlier than the
       * original, so use it instead */
      if ((options & CHECK_OPTION_FORCE_EXECUTION) &&
          (check_time < temp_event->run_time)) {
        logger(dbg_checks, most)
            << "New host check event is forced and occurs before the "
               "existing event, so the new event be used instead.";
        use_original_event = false;
      }
    }

    /* the original event is not a forced check... */
    else {
      /* the new event is a forced check, so use it instead */
      if ((options & CHECK_OPTION_FORCE_EXECUTION)) {
        use_original_event = false;
        logger(dbg_checks, most)
            << "New host check event is forced, so it will be used "
               "instead of the existing event.";
      }

      /* the new event is not forced either and its execution time is earlier
         than the original, so use it instead */
      else if (check_time < temp_event->run_time) {
        use_original_event = false;
        logger(dbg_checks, most)
            << "New host check event occurs before the existing (older) "
               "event, so it will be used instead.";
      }

      /* the new event is older, so override the existing one */
      else {
        logger(dbg_checks, most)
            << "New host check event occurs after the existing event, "
               "so we'll ignore it.";
      }
    }

    /* the originally queued event won the battle, so keep it */
    if (use_original_event) {
      delete new_event;
    }

    /* else use the new event, so remove the old */
    else {
      remove_event(temp_event, &event_list_low, &event_list_low_tail);
      delete temp_event;
    }
  }

  /* save check options for retention purposes */
  set_check_options(options);

  /* use the new event */
  if (!use_original_event) {
    logger(dbg_checks, most) << "Scheduling new host check event.";

    /* set the next host check time */
    set_next_check(check_time);

    /* place the new event in the event queue */
    new_event->event_type = EVENT_HOST_CHECK;
    new_event->event_data = (void*)this;
    new_event->event_args = (void*)NULL;
    new_event->event_options = options;
    new_event->run_time = get_next_check();
    new_event->recurring = false;
    new_event->event_interval = 0L;
    new_event->timing_func = NULL;
    new_event->compensate_for_time_change = true;
    reschedule_event(new_event, &event_list_low, &event_list_low_tail);
  }

  else {
    /* reset the next check time (it may be out of sync) */
    if (temp_event != NULL)
      set_next_check(temp_event->run_time);

    logger(dbg_checks, most)
        << "Keeping original host check event (ignoring the new one).";
  }

  /* update the status log */
  update_status(false);
}

/* detects host flapping */
void host::check_for_flapping(int update,
                              int actual_check,
                              int allow_flapstart_notification) {
  int update_history = true;
  int is_flapping = false;
  unsigned int x = 0;
  unsigned int y = 0;
  int last_state_history_value = HOST_UP;
  unsigned long wait_threshold = 0L;
  double curved_changes = 0.0;
  double curved_percent_change = 0.0;
  time_t current_time = 0L;
  double low_threshold = 0.0;
  double high_threshold = 0.0;
  double low_curve_value = 0.75;
  double high_curve_value = 1.25;

  logger(dbg_functions, basic) << "host::check_for_flapping()";

  logger(dbg_flapping, more)
      << "Checking host '" << get_name() << "' for flapping...";

  time(&current_time);

  /* period to wait for updating archived state info if we have no state change
   */
  if (get_total_services() == 0)
    wait_threshold = static_cast<unsigned long>(get_notification_interval() *
                                                config->interval_length());
  else
    wait_threshold = static_cast<unsigned long>(
        (get_total_service_check_interval() * config->interval_length()) /
        get_total_services());

  update_history = update;

  /* should we update state history for this state? */
  if (update_history) {
    if (get_current_state() == HOST_UP && !get_flap_detection_on_up())
      update_history = false;
    if (get_current_state() == HOST_DOWN && !get_flap_detection_on_down())
      update_history = false;
    if (get_current_state() == HOST_UNREACHABLE &&
        !get_flap_detection_on_unreachable())
      update_history = false;
  }

  /* if we didn't have an actual check, only update if we've waited long enough
   */
  if (update_history && !actual_check &&
      static_cast<unsigned long>(
          current_time - get_last_state_history_update()) < wait_threshold) {
    update_history = false;
  }

  /* what thresholds should we use (global or host-specific)? */
  low_threshold = (get_low_flap_threshold() <= 0.0)
                      ? config->low_host_flap_threshold()
                      : get_low_flap_threshold();
  high_threshold = (get_high_flap_threshold() <= 0.0)
                       ? config->high_host_flap_threshold()
                       : get_high_flap_threshold();

  /* record current host state */
  if (update_history) {
    /* update the last record time */
    set_last_state_history_update(current_time);

    /* record the current state in the state history */
    this->state_history[get_state_history_index()] = get_current_state();

    /* increment state history index to next available slot */
    set_state_history_index(get_state_history_index() + 1);
    if (get_state_history_index() >= MAX_STATE_HISTORY_ENTRIES)
      set_state_history_index(0);
  }

  /* calculate overall changes in state */
  for (x = 0, y = get_state_history_index(); x < MAX_STATE_HISTORY_ENTRIES;
       x++) {
    if (x == 0) {
      last_state_history_value = this->state_history[y];
      y++;
      if (y >= MAX_STATE_HISTORY_ENTRIES)
        y = 0;
      continue;
    }

    if (last_state_history_value != this->state_history[y])
      curved_changes +=
          (((double)(x - 1) * (high_curve_value - low_curve_value)) /
           ((double)(MAX_STATE_HISTORY_ENTRIES - 2))) +
          low_curve_value;

    last_state_history_value = this->state_history[y];

    y++;
    if (y >= MAX_STATE_HISTORY_ENTRIES)
      y = 0;
  }

  /* calculate overall percent change in state */
  curved_percent_change = (double)(((double)curved_changes * 100.0) /
                                   (double)(MAX_STATE_HISTORY_ENTRIES - 1));

  set_percent_state_change(curved_percent_change);

  logger(dbg_flapping, most)
      << com::centreon::logging::setprecision(2) << "LFT=" << low_threshold
      << ", HFT=" << high_threshold << ", CPC=" << curved_percent_change
      << ", PSC=" << curved_percent_change << "%";

  /* don't do anything if we don't have flap detection enabled on a program-wide
   * basis */
  if (!config->enable_flap_detection())
    return;

  /* don't do anything if we don't have flap detection enabled for this host */
  if (!get_flap_detection_enabled())
    return;

  /* are we flapping, undecided, or what?... */

  /* we're undecided, so don't change the current flap state */
  if (curved_percent_change > low_threshold &&
      curved_percent_change < high_threshold)
    return;

  /* we're below the lower bound, so we're not flapping */
  else if (curved_percent_change <= low_threshold)
    is_flapping = false;

  /* else we're above the upper bound, so we are flapping */
  else if (curved_percent_change >= high_threshold)
    is_flapping = true;

  logger(dbg_flapping, more)
      << "Host " << (is_flapping ? "is" : "is not") << " flapping ("
      << curved_percent_change << "% state change).";

  /* did the host just start flapping? */
  if (is_flapping && !get_is_flapping())
    set_flap(curved_percent_change, high_threshold, low_threshold,
             allow_flapstart_notification);

  /* did the host just stop flapping? */
  else if (!is_flapping && get_is_flapping())
    clear_flap(curved_percent_change, high_threshold, low_threshold);
}

void host::set_flap(double percent_change,
                    double high_threshold,
                    double low_threshold,
                    int allow_flapstart_notification) {
  logger(dbg_functions, basic) << "set_host_flap()";

  logger(dbg_flapping, more) << "Host '" << get_name() << "' started flapping!";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_runtime_warning, basic)
      << com::centreon::logging::setprecision(1)
      << "HOST FLAPPING ALERT: " << get_name()
      << ";STARTED; Host appears to have started flapping (" << percent_change
      << "% change > " << high_threshold << "% threshold)";

  /* add a non-persistent comment to the host */
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1)
      << "Notifications for this host are being suppressed because it "
         "was detected as "
      << "having been flapping between different "
         "states ("
      << percent_change << "% change > " << high_threshold
      << "% threshold).  When the host state stabilizes and the "
      << "flapping stops, notifications will be re-enabled.";

  unsigned long comment_id;
  comment_id = get_flapping_comment_id();
  std::shared_ptr<comment> com{
      new comment(comment::host, comment::flapping, get_name(), "", time(NULL),
                  "(Centreon Engine Process)", oss.str(), false,
                  comment::internal, false, (time_t)0)};

  comment::comments.insert({com->get_comment_id(), com});

  comment_id = com->get_comment_id();
  set_flapping_comment_id(comment_id);

  /* set the flapping indicator */
  set_is_flapping(true);

  /* send data to event broker */
  broker_flapping_data(NEBTYPE_FLAPPING_START, NEBFLAG_NONE, NEBATTR_NONE,
                       HOST_FLAPPING, this, percent_change, high_threshold,
                       low_threshold, NULL);

  /* see if we should check to send a recovery notification out when flapping
   * stops */
  if (get_current_state() != HOST_UP && get_current_notification_number() > 0)
    set_check_flapping_recovery_notification(true);
  else
    set_check_flapping_recovery_notification(false);

  /* send a notification */
  if (allow_flapstart_notification)
    notify(NOTIFICATION_FLAPPINGSTART, NULL, NULL, NOTIFICATION_OPTION_NONE);
}

/* handles a host that has stopped flapping */
void host::clear_flap(double percent_change,
                      double high_threshold,
                      double low_threshold) {
  logger(dbg_functions, basic) << "host::clear_flap()";

  logger(dbg_flapping, basic)
      << "Host '" << get_name() << "' stopped flapping.";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_info_message, basic)
      << com::centreon::logging::setprecision(1)
      << "HOST FLAPPING ALERT: " << get_name()
      << ";STOPPED; Host appears to have stopped flapping (" << percent_change
      << "% change < " << low_threshold << "% threshold)";

  /* delete the comment we added earlier */
  if (get_flapping_comment_id() != 0)
    comment::delete_comment(get_flapping_comment_id());
  set_flapping_comment_id(0);

  /* clear the flapping indicator */
  set_is_flapping(false);

  /* send data to event broker */
  broker_flapping_data(NEBTYPE_FLAPPING_STOP, NEBFLAG_NONE,
                       NEBATTR_FLAPPING_STOP_NORMAL, HOST_FLAPPING, this,
                       percent_change, high_threshold, low_threshold, NULL);

  /* send a notification */
  notify(NOTIFICATION_FLAPPINGSTOP, NULL, NULL, NOTIFICATION_OPTION_NONE);

  /* should we send a recovery notification? */
  if (get_check_flapping_recovery_notification() &&
      get_current_state() == HOST_UP)
    notify(NOTIFICATION_NORMAL, NULL, NULL, NOTIFICATION_OPTION_NONE);

  /* clear the recovery notification flag */
  set_check_flapping_recovery_notification(false);
}

/* updates host status info */
void host::update_status(bool aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (!aggregated_dump)
    broker_host_status(NEBTYPE_HOSTSTATUS_UPDATE, NEBFLAG_NONE, NEBATTR_NONE,
                       this, nullptr);
}

/**
 *  Check if acknowledgement on host expired.
 *
 */
void host::check_for_expired_acknowledgement() {
  if (get_problem_has_been_acknowledged()) {
    if (_acknowledgement_timeout > 0) {
      time_t now(time(nullptr));
      if (_last_acknowledgement + _acknowledgement_timeout >= now) {
        logger(log_info_message, basic)
            << "Acknowledgement of host '" << get_name() << "' just expired";
        set_problem_has_been_acknowledged(false);
        set_acknowledgement_type(ACKNOWLEDGEMENT_NONE);
        update_status(false);
      }
    }
  }
}

/* checks viability of sending a host notification */
int host::check_notification_viability(unsigned int type, int options) {
  time_t current_time;
  time_t timeperiod_start;

  logger(dbg_functions, basic) << "check_host_notification_viability()";

  /* forced notifications bust through everything */
  if (options & NOTIFICATION_OPTION_FORCED) {
    logger(dbg_notifications, more)
        << "This is a forced host notification, so we'll send it out.";
    return OK;
  }

  /* get current time */
  time(&current_time);

  /* are notifications enabled? */
  if (!config->enable_notifications()) {
    logger(dbg_notifications, more)
        << "Notifications are disabled, so host notifications will not "
           "be sent out.";
    return ERROR;
  }

  // See if the host can have notifications sent out at this time.
  {
    timezone_locker lock(get_timezone());
    if (check_time_against_period(current_time,
                                  this->notification_period_ptr) == ERROR) {
      logger(dbg_notifications, more)
          << "This host shouldn't have notifications sent out at "
             "this time.";

      // If this is a normal notification, calculate the next acceptable
      // notification time, once the next valid time range arrives...
      if (type == NOTIFICATION_NORMAL) {
        get_next_valid_time(current_time, &timeperiod_start,
                            this->notification_period_ptr);

        // It looks like there is no notification time defined, so
        // schedule next one far into the future (one year)...
        if (timeperiod_start == (time_t)0)
          set_next_notification((time_t)(current_time + (60 * 60 * 24 * 365)));
        // Else use the next valid notification time.
        else
          set_next_notification(timeperiod_start);

        time_t time = get_next_notification();
        logger(dbg_notifications, more)
            << "Next possible notification time: " << my_ctime(&time);
      }
      return ERROR;
    }
  }

  /* are notifications temporarily disabled for this host? */
  if (!get_notifications_enabled()) {
    logger(dbg_notifications, more)
        << "Notifications are temporarily disabled for this host, "
           "so we won't send one out.";
    return ERROR;
  }

  /*********************************************/
  /*** SPECIAL CASE FOR CUSTOM NOTIFICATIONS ***/
  /*********************************************/

  /* custom notifications are good to go at this point... */
  if (type == NOTIFICATION_CUSTOM) {
    if (get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
          << "We shouldn't send custom notification during "
             "scheduled downtime.";
      return ERROR;
    }
    return OK;
  }

  /****************************************/
  /*** SPECIAL CASE FOR ACKNOWLEGEMENTS ***/
  /****************************************/

  /*
   * acknowledgements only have to pass three general filters, although they
   * have another test of their own...
   */
  if (type == NOTIFICATION_ACKNOWLEDGEMENT) {
    /* don't send an acknowledgement if there isn't a problem... */
    if (get_current_state() == HOST_UP) {
      logger(dbg_notifications, more)
          << "The host is currently UP, so we won't send "
             "an acknowledgement.";
      return ERROR;
    }

    /*
     * acknowledgement viability test passed, so the notification can be sent
     * out
     */
    return OK;
  }

  /*****************************************/
  /*** SPECIAL CASE FOR FLAPPING ALERTS ***/
  /*****************************************/

  /* flapping notifications only have to pass three general filters */
  if (type == NOTIFICATION_FLAPPINGSTART || type == NOTIFICATION_FLAPPINGSTOP ||
      type == NOTIFICATION_FLAPPINGDISABLED) {
    /* don't send a notification if we're not supposed to... */
    if (!get_notify_on(notifier::flapping)) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about FLAPPING events for this host.";
      return ERROR;
    }

    /* don't send notifications during scheduled downtime */
    if (get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about FLAPPING events during "
             "scheduled downtime.";
      return ERROR;
    }

    /* flapping viability test passed, so the notification can be sent out */
    return OK;
  }

  /*****************************************/
  /*** SPECIAL CASE FOR DOWNTIME ALERTS ***/
  /*****************************************/

  /* flapping notifications only have to pass three general filters */
  if (type == NOTIFICATION_DOWNTIMESTART || type == NOTIFICATION_DOWNTIMEEND ||
      type == NOTIFICATION_DOWNTIMECANCELLED) {
    /* don't send a notification if we're not supposed to... */
    if (!get_notify_on(notifier::downtime)) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about DOWNTIME events for this host.";
      return ERROR;
    }

    /* don't send notifications during scheduled downtime */
    if (get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about DOWNTIME events during "
             "scheduled downtime!";
      return ERROR;
    }

    /* downtime viability test passed, so the notification can be sent out */
    return OK;
  }

  /****************************************/
  /*** NORMAL NOTIFICATIONS ***************/
  /****************************************/

  /* is this a hard problem/recovery? */
  if (get_state_type() == SOFT_STATE) {
    logger(dbg_notifications, more)
        << "This host is in a soft state, so we won't send "
           "a notification out.";
    return ERROR;
  }

  /* has this problem already been acknowledged? */
  if (get_problem_has_been_acknowledged()) {
    logger(dbg_notifications, more)
        << "This host problem has already been acknowledged, "
           "so we won't send a notification out!";
    return ERROR;
  }

  /* check notification dependencies */
  if (check_host_dependencies(this, hostdependency::notification) ==
      DEPENDENCIES_FAILED) {
    logger(dbg_notifications, more)
        << "Notification dependencies for this host have failed, "
           "so we won't sent a notification out!";
    return ERROR;
  }

  /* see if we should notify about problems with this host */
  if (get_current_state() == HOST_UNREACHABLE &&
      !get_notify_on(notifier::unreachable)) {
    logger(dbg_notifications, more)
        << "We shouldn't notify about UNREACHABLE status for this host.";
    return ERROR;
  }
  if (get_current_state() == HOST_DOWN && !get_notify_on(notifier::down)) {
    logger(dbg_notifications, more)
        << "We shouldn't notify about DOWN states for this host.";
    return ERROR;
  }
  if (get_current_state() == HOST_UP) {
    if (!get_notify_on(notifier::recovery)) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about RECOVERY states for this host.";
      return ERROR;
    }
    /* No notification received */
    if (get_notified_on() == 0) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about this recovery.";
      return ERROR;
    }
  }

  /* see if enough time has elapsed for first notification */
  if (type == NOTIFICATION_NORMAL &&
      (get_current_notification_number() == 0 ||
       (get_current_state() == HOST_UP && !_recovery_been_sent))) {
    /* get the time at which a notification should have been sent */
    time_t& initial_notif_time{_initial_notif_time};

    /* if not set, set it to now */
    if (!initial_notif_time)
      initial_notif_time = time(nullptr);

    double notification_delay =
        (get_current_state() != HOST_UP ? get_first_notification_delay()
                                        : _recovery_notification_delay) *
        config->interval_length();

    if (current_time <
        (time_t)(initial_notif_time + (time_t)(notification_delay))) {
      if (get_current_state() == HOST_UP)
        logger(dbg_notifications, more)
            << "Not enough time has elapsed since the host changed to an "
               "UP state (or since program start), so we shouldn't notify "
               "about this problem yet.";
      else
        logger(dbg_notifications, more)
            << "Not enough time has elapsed since the host changed to a "
               "non-UP state (or since program start), so we shouldn't notify "
               "about this problem yet.";
      return ERROR;
    }
  }

  /* if this host is currently flapping, don't send the notification */
  if (get_is_flapping()) {
    logger(dbg_notifications, more)
        << "This host is currently flapping, so we won't "
           "send notifications.";
    return ERROR;
  }

  /*
   * if this host is currently in a scheduled downtime period,
   * don't send the notification
   */
  if (get_scheduled_downtime_depth() > 0) {
    logger(dbg_notifications, more)
        << "This host is currently in a scheduled downtime, "
           "so we won't send notifications.";
    return ERROR;
  }

  /***** RECOVERY NOTIFICATIONS ARE GOOD TO GO AT THIS POINT *****/
  if (get_current_state() == HOST_UP)
    return OK;

  /* check if we shouldn't renotify contacts about the host problem */
  if (get_no_more_notifications()) {
    logger(dbg_notifications, more)
        << "We shouldn't re-notify contacts about this host problem.";
    return ERROR;
  }

  /* check if its time to re-notify the contacts about the host... */
  if (current_time < get_next_notification()) {
    logger(dbg_notifications, more)
        << "Its not yet time to re-notify the contacts "
           "about this host problem...";
    time_t time = get_next_notification();
    logger(dbg_notifications, more)
        << "Next acceptable notification time: " << my_ctime(&time);
    return ERROR;
  }

  return OK;
}

/* top level host state handler - occurs after every host check (soft/hard and
 * active/passive) */
int host::handle_state() {
  int state_change = false;
  time_t current_time = 0L;

  logger(dbg_functions, basic) << "handle_host_state()";

  /* get current time */
  time(&current_time);

  /* obsess over this host check */
  obsessive_compulsive_host_check_processor(this);

  /* update performance data */
  update_performance_data();

  /* record latest time for current state */
  switch (get_current_state()) {
    case HOST_UP:
      set_last_time_up(current_time);
      break;

    case HOST_DOWN:
      set_last_time_down(current_time);
      break;

    case HOST_UNREACHABLE:
      set_last_time_unreachable(current_time);
      break;

    default:
      break;
  }

  /* has the host state changed? */
  if (get_last_state() != get_current_state() ||
      get_last_hard_state() != get_current_state() ||
      (get_current_state() == HOST_UP && get_state_type() == SOFT_STATE))
    state_change = true;

  /* if the host state has changed... */
  if (state_change == true) {
    /* update last state change times */
    if (get_state_type() == SOFT_STATE ||
        get_last_state() != get_current_state())
      set_last_state_change(current_time);
    if (get_state_type() == HARD_STATE)
      set_last_hard_state_change(current_time);

    /* update the event id */
    set_last_event_id(get_current_event_id());
    set_current_event_id(next_event_id);
    next_event_id++;

    /* update the problem id when transitioning to a problem state */
    if (get_last_state() == HOST_UP) {
      /* don't reset last problem id, or it will be zero the next time a problem
       * is encountered */
      /*this->get_last_problem_id=this->get_current_problem_id; */
      set_current_problem_id(next_problem_id);
      next_problem_id++;
    }

    /* clear the problem id when transitioning from a problem state to an UP
     * state */
    if (get_current_state() == HOST_UP) {
      set_last_problem_id(get_current_problem_id());
      set_current_problem_id(0L);
    }

    /* reset the acknowledgement flag if necessary */
    if (get_acknowledgement_type() == ACKNOWLEDGEMENT_NORMAL) {
      set_problem_has_been_acknowledged(false);
      set_acknowledgement_type(ACKNOWLEDGEMENT_NONE);

      /* remove any non-persistant comments associated with the ack */
      comment::delete_host_acknowledgement_comments(this);
    } else if (get_acknowledgement_type() == ACKNOWLEDGEMENT_STICKY &&
               get_current_state() == HOST_UP) {
      set_problem_has_been_acknowledged(false);
      set_acknowledgement_type(ACKNOWLEDGEMENT_NONE);

      /* remove any non-persistant comments associated with the ack */
      comment::delete_host_acknowledgement_comments(this);
    }

    /* reset the next and last notification times */
    set_last_notification((time_t)0);
    set_next_notification((time_t)0);

    /* reset notification suppression option */
    set_no_more_notifications(false);

    /* write the host state change to the main log file */
    if (get_state_type() == HARD_STATE ||
        (get_state_type() == SOFT_STATE && config->log_host_retries() == true))
      log_event();

    /* check for start of flexible (non-fixed) scheduled downtime */
    /* CHANGED 08-05-2010 EG flex downtime can now start on soft states */
    /*if(this->state_type==HARD_STATE) */
    downtime_manager::instance().check_pending_flex_host_downtime(this);

    if (get_current_state() == HOST_UP) {
      _recovery_been_sent = false;
      _initial_notif_time = 0;
    }

    /* notify contacts about the recovery or problem if its a "hard" state */
    if (get_state_type() == HARD_STATE)
      notify(NOTIFICATION_NORMAL, NULL, NULL, NOTIFICATION_OPTION_NONE);

    /* handle the host state change */
    handle_host_event(this);

    /* the host just recovered, so reset the current host attempt */
    if (get_current_state() == HOST_UP)
      set_current_attempt(1);

    /* the host recovered, so reset the current notification number and state
     * flags (after the recovery notification has gone out) */
    if (get_current_state() == HOST_UP && _recovery_been_sent) {
      _current_notification_number = 0;
      set_notified_on(notifier::none);
    }
  }

  /* else the host state has not changed */
  else {
    bool old_recovery_been_sent{_recovery_been_sent};

    /* notify contacts if needed */
    if ((get_current_state() != HOST_UP ||
         (get_current_state() == HOST_UP && !_recovery_been_sent)) &&
        get_state_type() == HARD_STATE)
      notify(NOTIFICATION_NORMAL, NULL, NULL, NOTIFICATION_OPTION_NONE);

    /* the host recovered, so reset the current notification number and state
     * flags (after the recovery notification has gone out) */
    if (!old_recovery_been_sent && _recovery_been_sent &&
        get_current_state() == HOST_UP) {
      _current_notification_number = 0;
      set_notified_on(notifier::none);
    }

    /* if we're in a soft state and we should log host retries, do so now... */
    if (get_state_type() == SOFT_STATE && config->log_host_retries() == true)
      log_event();
  }

  return OK;
}

/* updates host performance data */
void host::update_performance_data() {
  /* should we be processing performance data for anything? */
  if (!config->process_performance_data())
    return;

  /* should we process performance data for this host? */
  if (!get_process_performance_data())
    return;

  /* process the performance data! */
  xpddefault_update_host_performance_data(this);
}

/* checks viability of performing a host check */
int host::verify_check_viability(int check_options,
                                 int* time_is_valid,
                                 time_t* new_time) {
  int result = OK;
  int perform_check = true;
  time_t current_time = 0L;
  time_t preferred_time = 0L;
  int check_interval = 0;

  logger(dbg_functions, basic) << "check_host_check_viability_3x()";

  /* get the check interval to use if we need to reschedule the check */
  if (this->get_state_type() == SOFT_STATE &&
      this->get_current_state() != HOST_UP)
    check_interval = static_cast<int>(this->get_retry_interval() *
                                      config->interval_length());
  else
    check_interval = static_cast<int>(this->get_check_interval() *
                                      config->interval_length());

  /* make sure check interval is positive - otherwise use 5 minutes out for next
   * check */
  if (check_interval <= 0)
    check_interval = 300;

  /* get the current time */
  time(&current_time);

  /* initialize the next preferred check time */
  preferred_time = current_time;

  /* can we check the host right now? */
  if (!(check_options & CHECK_OPTION_FORCE_EXECUTION)) {
    /* if checks of the host are currently disabled... */
    if (!this->get_checks_enabled()) {
      preferred_time = current_time + check_interval;
      perform_check = false;
    }

    // Make sure this is a valid time to check the host.
    {
      timezone_locker lock(get_timezone());
      if (check_time_against_period(static_cast<unsigned long>(current_time),
                                    this->check_period_ptr) == ERROR) {
        preferred_time = current_time;
        if (time_is_valid)
          *time_is_valid = false;
        perform_check = false;
      }
    }

    /* check host dependencies for execution */
    if (check_host_dependencies(this, hostdependency::execution) ==
        DEPENDENCIES_FAILED) {
      preferred_time = current_time + check_interval;
      perform_check = false;
    }
  }

  /* pass back the next viable check time */
  if (new_time)
    *new_time = preferred_time;

  result = (perform_check) ? OK : ERROR;
  return result;
}

void host::grab_macros_r(nagios_macros* mac) {
  grab_host_macros_r(mac, this);
}

/* notify a specific contact that an entire host is down or up */
int host::notify_contact(nagios_macros* mac,
                         contact* cntct,
                         int type,
                         char const* not_author,
                         char const* not_data,
                         int options,
                         int escalated) {
  char* command_name = nullptr;
  char* raw_command = nullptr;
  char* processed_command = nullptr;
  int early_timeout = false;
  double exectime;
  struct timeval start_time;
  struct timeval end_time;
  struct timeval method_start_time;
  struct timeval method_end_time;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  int neb_result;

  logger(dbg_functions, basic) << "notify_contact_of_host()";
  logger(dbg_notifications, most)
      << "** Attempting to notifying contact '" << cntct->get_name() << "'...";

  /*
   * check viability of notifying this user about the host
   * acknowledgements are no longer excluded from this test -
   * added 8/19/02 Tom Bertelson
   */
  if (check_contact_host_notification_viability(cntct, this, type, options) ==
      ERROR)
    return ERROR;

  logger(dbg_notifications, most)
      << "** Notifying contact '" << cntct->get_name() << "'";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* send data to event broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_contact_notification_data(
      NEBTYPE_CONTACTNOTIFICATION_START, NEBFLAG_NONE, NEBATTR_NONE,
      HOST_NOTIFICATION, type, start_time, end_time, (void*)this, cntct,
      not_author, not_data, escalated, nullptr);
  if (NEBERROR_CALLBACKCANCEL == neb_result)
    return ERROR;
  else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
    return OK;

  /* process all the notification commands this user has */
  for (std::shared_ptr<commands::command> const& cmd :
       cntct->get_host_notification_commands()) {
    /* get start time */
    gettimeofday(&method_start_time, nullptr);

    /* send data to event broker */
    method_end_time.tv_sec = 0L;
    method_end_time.tv_usec = 0L;
    neb_result = broker_contact_notification_method_data(
        NEBTYPE_CONTACTNOTIFICATIONMETHOD_START, NEBFLAG_NONE, NEBATTR_NONE,
        HOST_NOTIFICATION, type, method_start_time, method_end_time,
        (void*)this, cntct, cmd->get_command_line().c_str(), not_author,
        not_data, escalated, nullptr);
    if (NEBERROR_CALLBACKCANCEL == neb_result)
      break;
    else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
      continue;

    /* get the raw command line */
    get_raw_command_line_r(mac, cmd.get(), cmd->get_command_line().c_str(),
                           &raw_command, macro_options);
    if (raw_command == nullptr)
      continue;

    logger(dbg_notifications, most)
        << "Raw notification command: " << raw_command;

    /* process any macros contained in the argument */
    process_macros_r(mac, raw_command, &processed_command, macro_options);
    if (processed_command == nullptr)
      continue;

    /* get the command name */
    command_name = string::dup(cmd->get_command_line());

    /* run the notification command... */

    logger(dbg_notifications, most)
        << "Processed notification command: " << processed_command;

    /* log the notification to program log file */
    if (config->log_notifications() == true) {
      char const* host_state_str("UP");
      if ((unsigned int)this->get_current_state() < tab_host_states.size())
        // sizeof(tab_host_state_str) / sizeof(*tab_host_state_str))
        host_state_str =
            tab_host_states[this->get_current_state()].second.c_str();

      char const* notification_str("");
      if ((unsigned int)type < tab_notification_str.size())
        notification_str = tab_notification_str[type].c_str();

      std::string info;
      switch (type) {
        case NOTIFICATION_CUSTOM:
          notification_str = "CUSTOM";

        case NOTIFICATION_ACKNOWLEDGEMENT:
          info.append(";")
              .append(not_author ? not_author : "")
              .append(";")
              .append(not_data ? not_data : "");
          break;
      }

      std::string host_notification_state;
      if (strcmp(notification_str, "NORMAL") == 0)
        host_notification_state.append(host_state_str);
      else
        host_notification_state.append(notification_str)
            .append(" (")
            .append(host_state_str)
            .append(")");

      logger(log_host_notification, basic)
          << "HOST NOTIFICATION: " << cntct->get_name() << ';'
          << this->get_name() << ';' << host_notification_state << ";"
          << cmd->get_name() << ';' << this->get_plugin_output() << info;
    }

    /* run the notification command */
    try {
      my_system_r(mac, processed_command, config->notification_timeout(),
                  &early_timeout, &exectime, nullptr, 0);
    } catch (std::exception const& e) {
      logger(log_runtime_error, basic)
          << "Error: can't execute host notification '" << cntct->get_name()
          << "' : " << e.what();
    }

    /* check to see if the notification timed out */
    if (early_timeout == true) {
      logger(log_host_notification | log_runtime_warning, basic)
          << "Warning: Contact '" << cntct->get_name()
          << "' host notification command '" << processed_command
          << "' timed out after " << config->notification_timeout()
          << " seconds";
    }

    /* free memory */
    delete[] raw_command;
    delete[] processed_command;

    /* get end time */
    gettimeofday(&method_end_time, nullptr);

    /* send data to event broker */
    broker_contact_notification_method_data(
        NEBTYPE_CONTACTNOTIFICATIONMETHOD_END, NEBFLAG_NONE, NEBATTR_NONE,
        HOST_NOTIFICATION, type, method_start_time, method_end_time,
        (void*)this, cntct, cmd->get_command_line().c_str(), not_author,
        not_data, escalated, nullptr);
  }

  /* get end time */
  gettimeofday(&end_time, nullptr);

  /* update the contact's last host notification time */
  cntct->set_last_host_notification(start_time.tv_sec);

  /* send data to event broker */
  broker_contact_notification_data(
      NEBTYPE_CONTACTNOTIFICATION_END, NEBFLAG_NONE, NEBATTR_NONE,
      HOST_NOTIFICATION, type, start_time, end_time, (void*)this, cntct,
      not_author, not_data, escalated, nullptr);

  return OK;
}

void host::update_notification_flags() {
  /* update notifications flags */
  if (get_current_state() == HOST_DOWN)
    add_notified_on(notifier::down);
  else if (get_current_state() == HOST_UNREACHABLE)
    add_notified_on(notifier::unreachable);
}

/* calculates next acceptable re-notification time for a host */
time_t host::get_next_notification_time(time_t offset) {
  bool have_escalated_interval{false};

  logger(dbg_functions, basic) << "host::get_next_notification_time()";
  logger(dbg_notifications, most)
      << "Calculating next valid notification time...";

  /* default notification interval */
  double interval_to_use{get_notification_interval()};

  logger(dbg_notifications, most) << "Default interval: " << interval_to_use;

  /*
   * check all the host escalation entries for valid matches for this host
   * (at its current notification number)
   */
  for (std::shared_ptr<escalation>& e : get_escalations()) {
    /* interval < 0 means to use non-escalated interval */
    if (e->get_notification_interval() < 0.0)
      continue;

    /* skip this entry if it isn't appropriate */
    if (!is_valid_escalation_for_notification(e, NOTIFICATION_OPTION_NONE))
      continue;

    logger(dbg_notifications, most)
        << "Found a valid escalation w/ interval of "
        << e->get_notification_interval();

    /*
     * if we haven't used a notification interval from an escalation yet,
     * use this one
     */
    if (!have_escalated_interval) {
      have_escalated_interval = true;
      interval_to_use = e->get_notification_interval();
    }

    /* else use the shortest of all valid escalation intervals  */
    else if (e->get_notification_interval() < interval_to_use)
      interval_to_use = e->get_notification_interval();

    logger(dbg_notifications, most) << "New interval: " << interval_to_use;
  }

  /* if interval is 0, no more notifications should be sent */
  if (interval_to_use == 0.0)
    set_no_more_notifications(true);
  else
    set_no_more_notifications(false);

  logger(dbg_notifications, most)
      << "Interval used for calculating next valid notification time: "
      << interval_to_use;

  /* calculate next notification time */
  time_t next_notification{static_cast<time_t>(
      offset + interval_to_use * config->interval_length())};

  return next_notification;
}

/* disables flap detection for a specific host */
void host::disable_flap_detection() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic) << "disable_host_flap_detection()";

  logger(dbg_functions, more)
      << "Disabling flap detection for host '" << get_name() << "'.";

  /* nothing to do... */
  if (!get_flap_detection_enabled())
    return;

  /* set the attribute modified flag */
  _modified_attributes |= attr;

  /* set the flap detection enabled flag */
  set_flap_detection_enabled(false);

  /* send data to event broker */
  broker_adaptive_host_data(NEBTYPE_ADAPTIVEHOST_UPDATE, NEBFLAG_NONE,
                            NEBATTR_NONE, this, CMD_NONE, attr,
                            get_modified_attributes(), NULL);

  /* handle the details... */
  handle_host_flap_detection_disabled(this);
}

/* enables flap detection for a specific host */
void host::enable_flap_detection() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic) << "host::enable_flap_detection()";

  logger(dbg_flapping, more)
      << "Enabling flap detection for host '" << get_name() << "'.";

  /* nothing to do... */
  if (get_flap_detection_enabled())
    return;

  /* set the attribute modified flag */
  _modified_attributes |= attr;

  /* set the flap detection enabled flag */
  set_flap_detection_enabled(true);

  /* send data to event broker */
  broker_adaptive_host_data(NEBTYPE_ADAPTIVEHOST_UPDATE, NEBFLAG_NONE,
                            NEBATTR_NONE, this, CMD_NONE, attr,
                            get_modified_attributes(), nullptr);

  /* check for flapping */
  check_for_flapping(false, false, true);

  /* update host status */
  update_status(false);
}

/*
 * checks to see if a host escalation entry is a match for the current host
 * notification
 */
bool host::is_valid_escalation_for_notification(std::shared_ptr<escalation> e,
                                                int options) const {
  int notification_number;
  time_t current_time;

  logger(dbg_functions, basic)
      << "host::is_valid_escalation_for_notification()";

  /* get the current time */
  time(&current_time);

  /*
   * if this is a recovery, really we check for who got notified about a
   * previous problem
   */
  if (get_current_state() == HOST_UP)
    notification_number = get_current_notification_number() - 1;
  else
    notification_number = get_current_notification_number();

  /* find the host this escalation entry is associated with */
  if (e->notifier_ptr != this)
    return false;

  /*** EXCEPTION ***/
  /* broadcast options go to everyone, so this escalation is valid */
  if (options & NOTIFICATION_OPTION_BROADCAST)
    return true;

  /* skip this escalation if it happens later */
  if (e->get_first_notification() > notification_number)
    return false;

  /* skip this escalation if it has already passed */
  if (e->get_last_notification() != 0 &&
      e->get_last_notification() < notification_number)
    return false;

  /*
   * skip this escalation if it has a timeperiod and the current time
   * isn't valid
   */
  if (!e->get_escalation_period().empty() &&
      check_time_against_period(current_time, e->escalation_period_ptr) ==
          ERROR)
    return false;

  /* skip this escalation if the state options don't match */
  if (get_current_state() == HOST_UP && !e->get_escalate_on(notifier::recovery))
    return false;
  else if (get_current_state() == HOST_DOWN &&
           !e->get_escalate_on(notifier::down))
    return false;
  else if (get_current_state() == HOST_UNREACHABLE &&
           !e->get_escalate_on(notifier::unreachable))
    return false;

  return true;
}
