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
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/objects.hh"
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

host_map host::hosts;
host_id_map host::hosts_by_id;

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
           enum host::host_state initial_state,
           uint32_t check_interval,
           uint32_t retry_interval,
           int max_attempts,
           int notify_up,
           int notify_down,
           int notify_unreachable,
           int notify_flapping,
           int notify_downtime,
           uint32_t notification_interval,
           uint32_t first_notification_delay,
           uint32_t recovery_notification_delay,
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
           bool process_perfdata,
           bool check_freshness,
           int freshness_threshold,
           std::string const& notes,
           std::string const& notes_url,
           std::string const& action_url,
           std::string const& icon_image,
           std::string const& icon_image_alt,
           std::string const& vrml_image,
           std::string const& statusmap_image,
           double x_2d,
           double y_2d,
           bool have_2d_coords,
           double x_3d,
           double y_3d,
           double z_3d,
           bool have_3d_coords,
           bool should_be_drawn,
           bool retain_status_information,
           bool retain_nonstatus_information,
           bool obsess_over_host,
           std::string const& timezone)
    : notifier{host_notification,
               !display_name.empty() ? display_name : name,
               check_command,
               checks_enabled,
               accept_passive_checks,
               check_interval,
               retry_interval,
               notification_interval,
               max_attempts,
               (notify_up > 0 ? up : 0) |
               (notify_down > 0 ? down : 0) |
               (notify_downtime > 0 ? downtime : 0) |
               (notify_flapping > 0 ? (flappingstart | flappingstop | flappingdisabled) : 0) |
               (notify_unreachable > 0 ? unreachable : 0),
               (stalk_on_down > 0 ? down : 0) |
               (stalk_on_unreachable > 0 ? unreachable : 0) |
               (stalk_on_up > 0 ? up : 0),
               first_notification_delay,
               recovery_notification_delay,
               notification_period,
               notifications_enabled,
               check_period,
               event_handler,
               event_handler_enabled,
               notes,
               notes_url,
               action_url,
               icon_image,
               icon_image_alt,
               flap_detection_enabled,
               low_flap_threshold,
               high_flap_threshold,
               check_freshness,
               freshness_threshold,
               obsess_over_host,
               timezone,
               retain_status_information > 0,
               retain_nonstatus_information > 0},
    _id{host_id},
    _name{name},
    _address{address},
    _process_performance_data{process_perfdata},
    _statusmap_image{statusmap_image},
    _vrml_image{vrml_image},
    _have_2d_coords{have_2d_coords > 0},
    _have_3d_coords{have_3d_coords > 0},
     _x_2d{x_2d},
     _x_3d{x_3d},
     _y_2d{y_2d},
     _y_3d{y_3d},
     _z_3d{z_3d},
    _should_be_drawn{should_be_drawn > 0},
    _should_reschedule_current_check{false},
    _last_time_down{0},
    _last_time_unreachable{0},
    _last_time_up{0},
    _last_state_history_update{0},
    _total_services{0},
    _total_service_check_interval{0},
    _circular_path_checked{false},
    _contains_circular_path{false},
    _last_state{initial_state},
    _last_hard_state{initial_state},
    _current_state{initial_state},
    _initial_state{initial_state} {
  // Make sure we have the data we need.
  if (name.empty() || address.empty()) {
    logger(log_config_error, basic) << "Error: Host name or address is nullptr";
    throw engine_error() << "Could not register host '" << name << "'";
  }
  if (host_id == 0) {
    logger(log_config_error, basic) << "Error: Host must contain a host id "
                                       "because it comes from a database";
    throw engine_error() << "Could not register host '" << name << "'";
  }

  // Check if the host already exists.
  uint64_t id{host_id};
  if (is_host_exist(id)) {
    logger(log_config_error, basic)
        << "Error: Host '" << name << "' has already been defined";
    throw engine_error() << "Could not register host '" << name << "'";
  }

  // Duplicate string vars.
  _alias = !alias.empty() ? alias : name;

  set_current_attempt(initial_state ==  host::state_up ? 1 : max_attempts);
  set_modified_attributes(MODATTR_NONE);
  set_state_type(hard);

  set_flap_type((flap_detection_on_down > 0 ? down : 0) |
                (flap_detection_on_unreachable > 0 ? unreachable : 0) |
                (flap_detection_on_up > 0 ? up : 0));
}

uint64_t host::get_host_id(void) const {
  return _id;
}

void host::set_host_id(uint64_t id) {
  _id = id;
}

void host::add_child_host(host* child) {
  // Make sure we have the data we need.
  if (!child)
    throw engine_error() << "add child link called with nullptr ptr";

  child_hosts.insert({child->get_name(), child});

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
    throw engine_error() << "add child link called with bad host_name";
  }

  // A host cannot be a parent/child of itself.
  if (_name == host_name) {
    logger(log_config_error, basic)
        << "Error: Host '" << _name << "' cannot be a child/parent of itself";
    throw engine_error() << "host is child/parent itself";
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

bool host::get_process_performance_data() const {
  return _process_performance_data;
}

void host::set_process_performance_data(bool process_performance_data) {
  _process_performance_data = process_performance_data;
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

double host::get_x_2d() const {
  return _x_2d;
}

void host::set_x_2d(double x) {
  _x_2d = x;
}

double host::get_y_2d() const {
  return _y_2d;
}

void host::set_y_2d(double y) {
  _y_2d = y;
}

double host::get_x_3d() const {
  return _x_3d;
}

void host::set_x_3d(double x) {
  _x_3d = x;
}

double host::get_y_3d() const {
  return _y_3d;
}

void host::set_y_3d(double y) {
  _y_3d = y;
}

double host::get_z_3d() const {
  return _z_3d;
}

void host::set_z_3d(double z) {
  _z_3d = z;
}

int host::get_should_be_drawn() const {
  return _should_be_drawn;
}

void host::set_should_be_drawn(int should_be_drawn) {
  _should_be_drawn = should_be_drawn;
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

bool host::get_should_reschedule_current_check() const {
  return _should_reschedule_current_check;
}

void host::set_should_reschedule_current_check(bool should_reschedule) {
  _should_reschedule_current_check = should_reschedule;
}

time_t host::get_last_state_history_update() const {
  return _last_state_history_update;
}

void host::set_last_state_history_update(time_t last_state_history_update) {
  _last_state_history_update = last_state_history_update;
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

enum host::host_state host::get_current_state() const {
  return _current_state;
}

void host::set_current_state(enum host::host_state current_state) {
  _current_state = current_state;
}

enum host::host_state host::get_last_state() const {
  return _last_state;
}

void host::set_last_state(enum host::host_state last_state) {
  _last_state = last_state;
}

enum host::host_state host::get_last_hard_state() const {
  return _last_hard_state;
}

void host::set_last_hard_state(enum host::host_state last_hard_state) {
  _last_hard_state = last_hard_state;
}

enum host::host_state host::get_initial_state() const {
  return _initial_state;
}

void host::set_initial_state(enum host::host_state current_state) {
  _initial_state = current_state;
}

bool host::recovered() const {
  return _current_state == host::state_up;
}

int host::get_current_state_int() const {
  return static_cast<int>(_current_state);
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
//bool host::operator==(host const& other) throw() {
//  return get_name() == other.get_name() &&
//         get_display_name() == other.get_display_name() &&
//         get_alias() == other.get_alias() &&
//         get_address() == other.get_address() &&
//         ((parent_hosts.size() == other.parent_hosts.size()) &&
//          std::equal(parent_hosts.begin(), parent_hosts.end(),
//                     other.parent_hosts.begin()))
//         // Children do not need to be tested, they are
//         // created as parent back links.
//         // Services do not need to be tested, they are
//         // created as services back links.
//         && get_check_command() == other.get_check_command() &&
//         get_initial_state() == other.get_initial_state() &&
//         get_check_interval() == other.get_check_interval() &&
//         get_retry_interval() == other.get_retry_interval() &&
//         get_max_attempts() == other.get_max_attempts() &&
//         get_event_handler() == other.get_event_handler() &&
//         (get_contactgroups().size() == other.get_contactgroups().size() &&
//          std::equal(get_contactgroups().begin(), get_contactgroups().end(),
//                     other.get_contactgroups().begin())) &&
//         (get_contacts().size() == other.get_contacts().size() &&
//          std::equal(get_contacts().begin(), get_contacts().end(),
//                     other.get_contacts().begin())) &&
//         get_notification_interval() == other.get_notification_interval() &&
//         get_first_notification_delay() ==
//             other.get_first_notification_delay() &&
//         get_recovery_notification_delay() ==
//             other.get_recovery_notification_delay() &&
//         get_notify_on() == other.get_notify_on() &&
//         get_notification_period() == other.get_notification_period() &&
//         get_check_period() == other.get_check_period() &&
//         get_flap_detection_enabled() == other.get_flap_detection_enabled() &&
//         get_low_flap_threshold() == other.get_low_flap_threshold() &&
//         get_high_flap_threshold() == other.get_high_flap_threshold() &&
//         get_flap_type() == other.get_flap_detection_on() &&
//         get_stalk_type() == other.get_stalk_on() &&
//         get_check_freshness() == other.get_check_freshness() &&
//         get_freshness_threshold() == other.get_freshness_threshold() &&
//         get_process_performance_data() ==
//             other.get_process_performance_data() &&
//         get_checks_enabled() == other.get_checks_enabled() &&
//         get_accept_passive_checks() ==
//             other.get_accept_passive_checks() &&
//         get_event_handler_enabled() == other.get_event_handler_enabled() &&
//         get_retain_status_information() ==
//             other.get_retain_status_information() &&
//         get_retain_nonstatus_information() ==
//             other.get_retain_nonstatus_information() &&
//         get_obsess_over() == other.get_obsess_over() &&
//         get_notes() == other.get_notes() &&
//         get_notes_url() == other.get_notes_url() &&
//         get_action_url() == other.get_action_url() &&
//         get_icon_image() == other.get_icon_image() &&
//         get_icon_image_alt() == other.get_icon_image_alt() &&
//         get_vrml_image() == other.get_vrml_image() &&
//         get_statusmap_image() == other.get_statusmap_image() &&
//         get_have_2d_coords() == other.get_have_2d_coords() &&
//         get_x_2d() == other.get_x_2d() && get_y_2d() == other.get_y_2d() &&
//         get_have_3d_coords() == other.get_have_3d_coords() &&
//         get_x_3d() == other.get_x_3d() && get_y_3d() == other.get_y_3d() &&
//         get_z_3d() == other.get_z_3d() &&
//         get_should_be_drawn() == other.get_should_be_drawn() &&
//         custom_variables == other.custom_variables &&
//         get_problem_has_been_acknowledged() ==
//             other.get_problem_has_been_acknowledged() &&
//         get_acknowledgement_type() == other.get_acknowledgement_type() &&
//         get_check_type() == other.get_check_type() &&
//         get_current_state() == other.get_current_state() &&
//         get_last_state() == other.get_last_state() &&
//         get_last_hard_state() == other.get_last_hard_state() &&
//         get_plugin_output() == other.get_plugin_output() &&
//         get_long_plugin_output() == other.get_long_plugin_output() &&
//         get_perf_data() == other.get_perf_data() &&
//         get_state_type() == other.get_state_type() &&
//         get_current_attempt() == other.get_current_attempt() &&
//         get_current_event_id() == other.get_current_event_id() &&
//         get_last_event_id() == other.get_last_event_id() &&
//         get_current_problem_id() == other.get_current_problem_id() &&
//         get_last_problem_id() == other.get_last_problem_id() &&
//         get_latency() == other.get_latency() &&
//         get_execution_time() == other.get_execution_time() &&
//         get_is_executing() == other.get_is_executing() &&
//         get_check_options() == other.get_check_options() &&
//         get_notifications_enabled() == other.get_notifications_enabled() &&
//         get_last_notification() == other.get_last_notification() &&
//         get_next_notification() == other.get_next_notification() &&
//         get_next_check() == other.get_next_check() &&
//         get_should_be_scheduled() == other.get_should_be_scheduled() &&
//         get_last_check() == other.get_last_check() &&
//         get_last_state_change() == other.get_last_state_change() &&
//         get_last_hard_state_change() == other.get_last_hard_state_change() &&
//         get_last_time_up() == other.get_last_time_up() &&
//         get_last_time_down() == other.get_last_time_down() &&
//         get_last_time_unreachable() == other.get_last_time_unreachable() &&
//         get_has_been_checked() == other.get_has_been_checked() &&
//         get_is_being_freshened() == other.get_is_being_freshened() &&
//         get_notified_on() == other.get_notified_on() &&
//         get_no_more_notifications() == other.get_no_more_notifications() &&
//         get_current_notification_id() == other.get_current_notification_id() &&
//         get_scheduled_downtime_depth() ==
//             other.get_scheduled_downtime_depth() &&
//         get_pending_flex_downtime() == other.get_pending_flex_downtime() &&
//         std::equal(get_state_history().begin(), get_state_history().end(),
//                     other.get_state_history().begin()) &&
//         get_state_history_index() == other.get_state_history_index() &&
//         get_last_state_history_update() ==
//             other.get_last_state_history_update() &&
//         get_is_flapping() == other.get_is_flapping() &&
//         get_flapping_comment_id() == other.get_flapping_comment_id() &&
//         get_percent_state_change() == other.get_percent_state_change() &&
//         get_total_services() == other.get_total_services() &&
//         get_total_service_check_interval() ==
//             other.get_total_service_check_interval() &&
//         get_modified_attributes() == other.get_modified_attributes() &&
//         get_circular_path_checked() == other.get_circular_path_checked() &&
//         get_contains_circular_path() == other.get_contains_circular_path();
//}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
//bool host::operator!=(host const& other) throw() {
//  return !operator==(other);
//}

std::ostream& operator<<(std::ostream& os, host_map_unsafe const& obj) {
  for (host_map_unsafe::const_iterator it{obj.begin()}, end{obj.end()}; it != end;
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
  hostgroup* hg{obj.get_parent_groups().front()};

  std::string evt_str;
  if (obj.get_event_handler_ptr())
    evt_str = obj.get_event_handler_ptr()->get_name();
  std::string cmd_str;
  if (obj.get_check_command_ptr())
    cmd_str = obj.get_check_command_ptr()->get_name();
  std::string chk_period_str;
  if (obj.check_period_ptr)
    chk_period_str = obj.check_period_ptr->get_name();

  std::string notif_period_str;
  if (obj.get_notification_period_ptr())
    notif_period_str = obj.get_notification_period_ptr()->get_name();

  std::string cg_oss;
  std::string c_oss;
  std::string p_oss;
  std::string child_oss;

  if (obj.get_contactgroups().empty())
    cg_oss = "\"NULL\"";
  else {
    std::ostringstream oss;
    oss << obj.get_contactgroups();
    cg_oss = oss.str();
  }
  if (obj.get_contacts().empty())
    c_oss = "\"NULL\"";
  else {
    std::ostringstream oss;
    oss << obj.get_contacts();
    c_oss = oss.str();
  }
  if (obj.parent_hosts.empty())
    p_oss = "\"NULL\"";
  else {
    std::ostringstream oss;
    oss << obj.parent_hosts;
    p_oss = oss.str();
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
        "  recovery_notification_delay:          "
     << obj.get_recovery_notification_delay()
     << "\n"
        "  notify_on_down:                       "
     << obj.get_notify_on(notifier::down)
     << "\n"
        "  notify_on_unreachable:                "
     << obj.get_notify_on(notifier::unreachable)
     << "\n"
        "  notify_on_recovery:                   "
     << obj.get_notify_on(notifier::up)
     << "\n"
        "  notify_on_flappingstart:              "
     << obj.get_notify_on(notifier::flappingstart)
     << "\n"
        "  notify_on_flappingstop:               "
     << obj.get_notify_on(notifier::flappingstop)
     << "\n"
        "  notify_on_flappingdisabled:           "
     << obj.get_notify_on(notifier::flappingdisabled)
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
     << obj.get_flap_detection_on(notifier::up)
     << "\n"
        "  flap_detection_on_down:               "
     << obj.get_flap_detection_on(notifier::down)
     << "\n"
        "  flap_detection_on_unreachable:        "
     << obj.get_flap_detection_on(notifier::unreachable)
     << "\n"
        "  stalk_on_up:                          "
     << obj.get_stalk_on(notifier::up)
     << "\n"
        "  stalk_on_down:                        "
     << obj.get_stalk_on(notifier::down)
     << "\n"
        "  stalk_on_unreachable:                 "
     << obj.get_stalk_on(notifier::unreachable)
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
        "  accept_passive_checks:                "
     << obj.get_accept_passive_checks()
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
     << obj.get_obsess_over()
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
        "  no_more_notifications:                "
     << obj.get_no_more_notifications()
     << "\n"
        "  current_notification_id:              "
     << obj.get_current_notification_id()
     << "\n"
        "  scheduled_downtime_depth:             "
     << obj.get_scheduled_downtime_depth()
     << "\n"
        "  pending_flex_downtime:                "
     << obj.get_pending_flex_downtime() << "\n";

  os << "  state_history:                        ";
  for (size_t i{0}, end{obj.get_state_history().size()}; i < end; ++i)
    os << obj.get_state_history()[i] << (i + 1 < end ? ", " : "\n");

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
     << (hg ? hg->get_group_name() : "") << "\n";

  for (std::pair<std::string, std::shared_ptr<customvariable>> const& cv : obj.custom_variables)
    os << cv.first << " ; ";

  os << "\n}\n";
  return os;
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
    if (child_host->parent_hosts.empty())
      return true;
  }
  // Mid-level/bottom hosts.
  else {
    host_map_unsafe::const_iterator it{child_host->parent_hosts.find(parent_host->get_name())};
    return it != child_host->parent_hosts.end();
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
  for (host_map::iterator it{host::hosts.begin()}, end{host::hosts.end()};
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
  for (host_map::iterator it{host::hosts.begin()}, end{host::hosts.end()};
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
  for (host_map::iterator it{host::hosts.begin()}, end{host::hosts.end()};
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
  for (host_map::iterator it{host::hosts.begin()}, end{host::hosts.end()};
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
  host_id_map::const_iterator it{host::hosts_by_id.find(host_id)};
  if (it == host::hosts_by_id.end())
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
  host_id_map::const_iterator it(host::hosts_by_id.find(host_id));
  return it != host::hosts_by_id.end();
}

/**
 *  Get the id associated with a host.
 *
 *  @param[in] name  The name of the host.
 *
 *  @return  The host id or 0.
 */
uint64_t engine::get_host_id(std::string const& name) {
  host_map::const_iterator found{host::hosts.find(name)};
  return found != host::hosts.end() ? found->second->get_host_id() : 0u;
}

/**
 *  Schedule acknowledgement expiration.
 *
 */
void host::schedule_acknowledgement_expiration() {
  if (get_acknowledgement_timeout() > 0 && get_last_acknowledgement() != (time_t)0) {
    schedule_new_event(EVENT_EXPIRE_HOST_ACK, false,
                       get_last_acknowledgement() + get_acknowledgement_timeout(), false,
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
  enum service::service_state svc_res{service::state_ok};
  enum host::host_state hst_res{host::state_up};
  int reschedule_check{false};
  std::string old_plugin_output;
  struct timeval start_time_hires;
  struct timeval end_time_hires;
  double execution_time{0.0};

  logger(dbg_functions, basic) << "handle_async_host_check_result_3x()";

  /* make sure we have what we need */
  if (queued_check_result == nullptr)
    return ERROR;

  time_t current_time{std::time(nullptr)};

  execution_time = (double)((double)(queued_check_result->get_finish_time().tv_sec -
                                     queued_check_result->get_start_time().tv_sec) +
                            (double)((queued_check_result->get_finish_time().tv_usec -
                                      queued_check_result->get_start_time().tv_usec) /
                                     1000.0) /
                                1000.0);
  if (execution_time < 0.0)
    execution_time = 0.0;

  logger(dbg_checks, more) << "** Handling async check result for host '"
                           << get_name() << "'...";

  logger(dbg_checks, most)
      << "\tCheck Type:         "
      << (queued_check_result->get_check_type() == check_active ? "Active"
                                                          : "Passive")
      << "\n"
      << "\tCheck Options:      " << queued_check_result->get_check_options() << "\n"
      << "\tReschedule Check?:  "
      << (queued_check_result->get_reschedule_check() ? "Yes" : "No") << "\n"
      << "\tShould Reschedule Current Host Check?:"
      << host::hosts[get_name()]->get_should_reschedule_current_check()
      << "\tExited OK?:         "
      << (queued_check_result->get_exited_ok() ? "Yes" : "No") << "\n"
      << com::centreon::logging::setprecision(3)
      << "\tExec Time:          " << execution_time << "\n"
      << "\tLatency:            " << queued_check_result->get_latency() << "\n"
      << "\treturn Status:      " << queued_check_result->get_return_code() << "\n"
      << "\tOutput:             " << queued_check_result->get_output();

  /* decrement the number of host checks still out there... */
  if (queued_check_result->get_check_type() == check_active &&
      currently_running_host_checks > 0)
    currently_running_host_checks--;

  /* skip this host check results if its passive and we aren't accepting passive
   * check results */
  if (queued_check_result->get_check_type() == check_passive) {
    if (!config->accept_passive_host_checks()) {
      logger(dbg_checks, basic)
          << "Discarding passive host check result because passive host "
             "checks are disabled globally.";
      return ERROR;
    }
    if (!get_accept_passive_checks()) {
      logger(dbg_checks, basic)
          << "Discarding passive host check result because passive checks "
             "are disabled for this host.";
      return ERROR;
    }
  }

  /* clear the freshening flag (it would have been set if this host was
   * determined to be stale) */
  if (queued_check_result->get_check_options() & CHECK_OPTION_FRESHNESS_CHECK)
    set_is_being_freshened(false);

  /* DISCARD INVALID FRESHNESS CHECK RESULTS */
  /* If a host goes stale, Engine will initiate a forced check in order
  ** to freshen it. There is a race condition whereby a passive check
  ** could arrive between the 1) initiation of the forced check and 2)
  ** the time when the forced check result is processed here. This would
  ** make the host fresh again, so we do a quick check to make sure the
  ** host is still stale before we accept the check result.
  */
  if ((queued_check_result->get_check_options() & CHECK_OPTION_FRESHNESS_CHECK) &&
      is_result_fresh(current_time, false)) {
    logger(dbg_checks, basic)
        << "Discarding host freshness check result because the host is "
           "currently fresh (race condition avoided).";
    return OK;
  }

  /* was this check passive or active? */
  set_check_type((queued_check_result->get_check_type() == check_active)
                     ? check_active
                     : check_passive);

  /* update check statistics for passive results */
  if (queued_check_result->get_check_type() == check_passive)
    update_check_stats(PASSIVE_HOST_CHECK_STATS,
                       queued_check_result->get_start_time().tv_sec);

  /* should we reschedule the next check of the host? NOTE: this might be
   * overridden later... */
  reschedule_check = queued_check_result->get_reschedule_check();

  // Inherit the should reschedule flag from the host. It is used when
  // rescheduled checks were discarded because only one check can be executed
  // on the same host at the same time. The flag is then set in the host
  // and this check should be rescheduled regardless of what it was meant
  // to initially.
  if (host::hosts[get_name()]->get_should_reschedule_current_check() &&
      !queued_check_result->get_reschedule_check())
    reschedule_check = true;

  // Clear the should reschedule flag.
  host::hosts[get_name()]->set_should_reschedule_current_check(false);

  /* check latency is passed to us for both active and passive checks */
  set_latency(queued_check_result->get_latency());

  /* update the execution time for this check (millisecond resolution) */
  set_execution_time(execution_time);

  /* set the checked flag */
  set_has_been_checked(true);

  /* clear the execution flag if this was an active check */
  if (queued_check_result->get_check_type() == check_active)
    set_is_executing(false);

  /* get the last check time */
  set_last_check(queued_check_result->get_start_time().tv_sec);

  /* was this check passive or active? */
  set_check_type((queued_check_result->get_check_type() == check_active)
                     ? check_active
                     : check_passive);

  /* save the old host state */
  set_last_state(get_current_state());
  if (get_state_type() == hard)
    set_last_hard_state(get_current_state());

  /* save old plugin output */
  if (!get_plugin_output().empty())
    old_plugin_output = get_plugin_output();

  /* parse check output to get: (1) short output, (2) long output, (3) perf data
   */

  std::string output{queued_check_result->get_output()};
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
  svc_res = static_cast<enum service::service_state>(queued_check_result->get_return_code());
  hst_res = static_cast<enum host::host_state>(queued_check_result->get_return_code());

  /* adjust return code (active checks only) */
  if (queued_check_result->get_check_type() == check_active) {
    /* if there was some error running the command, just skip it (this shouldn't
     * be happening) */
    if (!queued_check_result->get_exited_ok()) {
      logger(log_runtime_warning, basic)
          << "Warning:  Check of host '" << get_name()
          << "' did not exit properly!";

      set_plugin_output("(Host check did not exit properly)");
      set_long_plugin_output("");
      set_perf_data("");

      svc_res =  service::state_unknown;
    }

    /* make sure the return code is within bounds */
    else if (queued_check_result->get_return_code() < 0 ||
             queued_check_result->get_return_code() > 3) {
      logger(log_runtime_warning, basic)
          << "Warning: return (code of " << queued_check_result->get_return_code()
          << " for check of host '" << get_name() << "' was out of bounds."
          << ((queued_check_result->get_return_code() == 126 ||
               queued_check_result->get_return_code() == 127)
                  ? " Make sure the plugin you're trying to run actually "
                    "exists."
                  : "");

      std::ostringstream oss;
      oss << "(Return code of " << queued_check_result->get_return_code()
          << " is out of bounds"
          << ((queued_check_result->get_return_code() == 126 ||
               queued_check_result->get_return_code() == 127)
                  ? " - plugin may be missing"
                  : "")
          << ")";

      set_plugin_output(oss.str());
      set_long_plugin_output("");
      set_perf_data("");

      svc_res =  service::state_unknown;
    }

    /* a NULL host check command means we should assume the host is UP */
    if (get_check_command().empty()) {
      set_plugin_output("(Host assumed to be UP)");
      svc_res =  service::state_ok;
    }
  }

  /* translate return code to basic UP/DOWN state - the DOWN/UNREACHABLE state
   * determination is made later */
  /* NOTE: only do this for active checks - passive check results already have
   * the final state */
  if (queued_check_result->get_check_type() == check_active) {
    /* if we're not doing aggressive host checking, let WARNING states indicate
     * the host is up (fake the result to be state_ok) */
    if (!config->use_aggressive_host_checking() && svc_res ==  service::state_warning)
      svc_res = service::state_ok;

    /* OK states means the host is UP */
    if (svc_res == service::state_ok)
      hst_res = host::state_up;

    /* any problem state indicates the host is not UP */
    else
      hst_res =  host::state_down;
  }

  /******************* PROCESS THE CHECK RESULTS ******************/

  /* process the host check result */
  process_check_result_3x(hst_res, old_plugin_output, CHECK_OPTION_NONE,
                          reschedule_check, true,
                          config->cached_host_check_horizon());

  logger(dbg_checks, more) << "** Async check result for host '" << get_name()
                           << "' handled: new state=" << get_current_state();

  /* high resolution start time for event broker */
  start_time_hires = queued_check_result->get_start_time();

  /* high resolution end time for event broker */
  gettimeofday(&end_time_hires, nullptr);

  /* send data to event broker */
  broker_host_check(
      NEBTYPE_HOSTCHECK_PROCESSED, NEBFLAG_NONE, NEBATTR_NONE, this,
      get_check_type(), get_current_state(), get_state_type(), start_time_hires,
      end_time_hires, get_check_command().c_str(), get_latency(),
      get_execution_time(), config->host_check_timeout(),
      queued_check_result->get_early_timeout(), queued_check_result->get_return_code(),
      nullptr, const_cast<char*>(get_plugin_output().c_str()),
      const_cast<char*>(get_long_plugin_output().c_str()),
      const_cast<char*>(get_perf_data().c_str()), nullptr);
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
      current_time = time(nullptr);

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
  timed_event* temp_event = nullptr;
  timed_event* new_event = nullptr;
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
  temp_event = timed_event::find_event(timed_event::low, EVENT_HOST_CHECK, this);

  /* we found another host check event for this host in the queue - what should
   * we do? */
  if (temp_event != nullptr) {
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
      remove_event(new_event, timed_event::low);
      delete new_event;
    }

    /* else use the new event, so remove the old */
    else {
      remove_event(temp_event, timed_event::low);
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
    new_event->event_args = (void*)nullptr;
    new_event->event_options = options;
    new_event->run_time = get_next_check();
    new_event->recurring = false;
    new_event->event_interval = 0L;
    new_event->timing_func = nullptr;
    new_event->compensate_for_time_change = true;
    reschedule_event(new_event, timed_event::low);
  }

  else {
    /* reset the next check time (it may be out of sync) */
    if (temp_event != nullptr)
      set_next_check(temp_event->run_time);

    logger(dbg_checks, most)
        << "Keeping original host check event (ignoring the new one).";
  }

  /* update the status log */
  update_status(false);
}

/* detects host flapping */
void host::check_for_flapping(bool update,
                              bool actual_check,
                              bool allow_flapstart_notification) {
  bool update_history;
  bool is_flapping = false;
  unsigned int x = 0;
  unsigned int y = 0;
  int last_state_history_value = host::state_up;
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
    if (get_current_state() ==  host::state_up && !get_flap_detection_on(up))
      update_history = false;
    if (get_current_state() ==  host::state_down && !get_flap_detection_on(down))
      update_history = false;
    if (get_current_state() ==  host::state_unreachable &&
        !get_flap_detection_on(host::unreachable))
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
    get_state_history()[get_state_history_index()] = get_current_state();

    /* increment state history index to next available slot */
    set_state_history_index(get_state_history_index() + 1);
    if (get_state_history_index() >= MAX_STATE_HISTORY_ENTRIES)
      set_state_history_index(0);
  }

  /* calculate overall changes in state */
  for (x = 0, y = get_state_history_index(); x < MAX_STATE_HISTORY_ENTRIES;
       x++) {
    if (x == 0) {
      last_state_history_value = get_state_history()[y];
      y++;
      if (y >= MAX_STATE_HISTORY_ENTRIES)
        y = 0;
      continue;
    }

    if (last_state_history_value != get_state_history()[y])
      curved_changes +=
          (((double)(x - 1) * (high_curve_value - low_curve_value)) /
           ((double)(MAX_STATE_HISTORY_ENTRIES - 2))) +
          low_curve_value;

    last_state_history_value = get_state_history()[y];

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
                    bool allow_flapstart_notification) {
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

  std::shared_ptr<comment> com{
      new comment(comment::host, comment::flapping, _name, "", time(nullptr),
                  "(Centreon Engine Process)", oss.str(), false,
                  comment::internal, false, (time_t)0)};

  comment::comments.insert({com->get_comment_id(), com});

  uint64_t comment_id{com->get_comment_id()};
  set_flapping_comment_id(comment_id);

  /* set the flapping indicator */
  set_is_flapping(true);

  /* send data to event broker */
  broker_flapping_data(NEBTYPE_FLAPPING_START, NEBFLAG_NONE, NEBATTR_NONE,
                       HOST_FLAPPING, this, percent_change, high_threshold,
                       low_threshold, nullptr);

  /* send a notification */
  if (allow_flapstart_notification)
    notify(reason_flappingstart, "", "", notifier::notification_option_none);
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
                       percent_change, high_threshold, low_threshold, nullptr);

  /* send a notification */
  notify(reason_flappingstop, "", "", notifier::notification_option_none);

  /* Send a recovery notification if needed */
  notify(reason_recovery, "", "", notifier::notification_option_none);
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
    if (get_acknowledgement_timeout() > 0) {
      time_t now(time(nullptr));
      if (get_last_acknowledgement() + get_acknowledgement_timeout() >= now) {
        logger(log_info_message, basic)
            << "Acknowledgement of host '" << get_name() << "' just expired";
        set_problem_has_been_acknowledged(false);
        set_acknowledgement_type(ACKNOWLEDGEMENT_NONE);
        update_status(false);
      }
    }
  }
}

///* checks viability of sending a host notification */
//bool host::check_notification_viability(reason_type type, int options) {
//  time_t current_time;
//  time_t timeperiod_start;
//
//  logger(dbg_functions, basic) << "host::check_notification_viability()";
//
//  /* get current time */
//  time(&current_time);
//
//  timeperiod* temp_period{notification_period_ptr};
//
//  // See if the host can have notifications sent out at this time.
//  {
//    timezone_locker lock(get_timezone());
//    if (check_time_against_period(current_time,
//                                  temp_period) == false) {
//      logger(dbg_notifications, more)
//          << "This host shouldn't have notifications sent out at "
//             "this time.";
//
//      // If this is a normal notification, calculate the next acceptable
//      // notification time, once the next valid time range arrives...
//      if (type == notification_normal) {
//        get_next_valid_time(current_time, &timeperiod_start,
//                            temp_period);
//
//        // It looks like there is no notification time defined, so
//        // schedule next one far into the future (one year)...
//        if (timeperiod_start == (time_t)0)
//          set_next_notification((time_t)(current_time + (60 * 60 * 24 * 365)));
//        // Else use the next valid notification time.
//        else
//          set_next_notification(timeperiod_start);
//
//        time_t time = get_next_notification();
//        logger(dbg_notifications, more)
//            << "Next possible notification time: " << my_ctime(&time);
//      }
//      return false;
//    }
//  }
//
//  /*********************************************/
//  /*** SPECIAL CASE FOR CUSTOM NOTIFICATIONS ***/
//  /*********************************************/
//
//  /* custom notifications are good to go at this point... */
//  if (type == notification_custom) {
//    if (get_scheduled_downtime_depth() > 0) {
//      logger(dbg_notifications, more)
//          << "We shouldn't send custom notification during "
//             "scheduled downtime.";
//      return false;
//    }
//    return true;
//  }
//
//  /****************************************/
//  /*** SPECIAL CASE FOR ACKNOWLEGEMENTS ***/
//  /****************************************/
//
//  /*
//   * acknowledgements only have to pass three general filters, although they
//   * have another test of their own...
//   */
//  if (type == notification_acknowledgement) {
//    /* don't send an acknowledgement if there isn't a problem... */
//    if (get_current_state() ==  host::state_up) {
//      logger(dbg_notifications, more)
//          << "The host is currently UP, so we won't send "
//             "an acknowledgement.";
//      return false;
//    }
//
//    /*
//     * acknowledgement viability test passed, so the notification can be sent
//     * out
//     */
//    return true;
//  }
//
//  /*****************************************/
//  /*** SPECIAL CASE FOR FLAPPING ALERTS ***/
//  /*****************************************/
//
//  /* flapping notifications only have to pass three general filters */
//  if (type == notification_flappingstart || type == notification_flappingstop ||
//      type == notification_flappingdisabled) {
//    /* don't send a notification if we're not supposed to... */
//    if (!get_notify_on(flapping)) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about FLAPPING events for this host.";
//      return false;
//    }
//
//    /* don't send notifications during scheduled downtime */
//    if (get_scheduled_downtime_depth() > 0) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about FLAPPING events during "
//             "scheduled downtime.";
//      return false;
//    }
//
//    /* flapping viability test passed, so the notification can be sent out */
//    return true;
//  }
//
//  /*****************************************/
//  /*** SPECIAL CASE FOR DOWNTIME ALERTS ***/
//  /*****************************************/
//
//  /* flapping notifications only have to pass three general filters */
//  if (type == notification_downtimestart || type == notification_downtimeend ||
//      type == notification_downtimecancelled) {
//    /* don't send a notification if we're not supposed to... */
//    if (!get_notify_on(downtime)) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about DOWNTIME events for this host.";
//      return false;
//    }
//
//    /* don't send notifications during scheduled downtime */
//    if (get_scheduled_downtime_depth() > 0) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about DOWNTIME events during "
//             "scheduled downtime!";
//      return false;
//    }
//
//    /* downtime viability test passed, so the notification can be sent out */
//    return true;
//  }
//
//  /****************************************/
//  /*** NORMAL NOTIFICATIONS ***************/
//  /****************************************/
//
//  /* is this a hard problem/recovery? */
//  if (get_state_type() == soft) {
//    logger(dbg_notifications, more)
//        << "This host is in a soft state, so we won't send "
//           "a notification out.";
//    return false;
//  }
//
//  /* has this problem already been acknowledged? */
//  if (get_problem_has_been_acknowledged()) {
//    logger(dbg_notifications, more)
//        << "This host problem has already been acknowledged, "
//           "so we won't send a notification out!";
//    return false;
//  }
//
//  /* check notification dependencies */
//  if (authorized_by_dependencies(hostdependency::notification) ==
//      DEPENDENCIES_FAILED) {
//    logger(dbg_notifications, more)
//        << "Notification dependencies for this host have failed, "
//           "so we won't sent a notification out!";
//    return false;
//  }
//
//  /* see if we should notify about problems with this host */
//  if (get_current_state() ==  host::state_unreachable &&
//      !get_notify_on(unreachable)) {
//    logger(dbg_notifications, more)
//        << "We shouldn't notify about UNREACHABLE status for this host.";
//    return false;
//  }
//  if (get_current_state() ==  host::state_down && !get_notify_on(down)) {
//    logger(dbg_notifications, more)
//        << "We shouldn't notify about DOWN states for this host.";
//    return false;
//  }
//  if (get_current_state() ==  host::state_up) {
//    if (!get_notify_on(recovery)) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about RECOVERY states for this host.";
//      return false;
//    }
//    /* No notification received */
//    if (get_notified_on() == 0) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about this recovery.";
//      return false;
//    }
//  }
//
//  /* see if enough time has elapsed for first notification */
//  if (type == notification_normal &&
//      (get_current_notification_number() == 0 ||
//       (get_current_state() ==  host::state_up && !_recovery_been_sent))) {
//    /* get the time at which a notification should have been sent */
//    time_t& initial_notif_time{_initial_notif_time};
//
//    /* if not set, set it to now */
//    if (!initial_notif_time)
//      initial_notif_time = time(nullptr);
//
//    double notification_delay =
//        (get_current_state() !=  host::state_up ? get_first_notification_delay()
//                                        : _recovery_notification_delay) *
//        config->interval_length();
//
//    if (current_time <
//        (time_t)(initial_notif_time + (time_t)(notification_delay))) {
//      if (get_current_state() ==  host::state_up)
//        logger(dbg_notifications, more)
//            << "Not enough time has elapsed since the host changed to an "
//               "UP state (or since program start), so we shouldn't notify "
//               "about this problem yet.";
//      else
//        logger(dbg_notifications, more)
//            << "Not enough time has elapsed since the host changed to a "
//               "non-UP state (or since program start), so we shouldn't notify "
//               "about this problem yet.";
//      return false;
//    }
//  }
//
//  /* if this host is currently flapping, don't send the notification */
//  if (get_is_flapping()) {
//    logger(dbg_notifications, more)
//        << "This host is currently flapping, so we won't "
//           "send notifications.";
//    return false;
//  }
//
//  /*
//   * if this host is currently in a scheduled downtime period,
//   * don't send the notification
//   */
//  if (get_scheduled_downtime_depth() > 0) {
//    logger(dbg_notifications, more)
//        << "This host is currently in a scheduled downtime, "
//           "so we won't send notifications.";
//    return false;
//  }
//
//  /***** RECOVERY NOTIFICATIONS ARE GOOD TO GO AT THIS POINT *****/
//  if (get_current_state() ==  host::state_up)
//    return true;
//
//  /* check if we shouldn't renotify contacts about the host problem */
//  if (get_no_more_notifications()) {
//    logger(dbg_notifications, more)
//        << "We shouldn't re-notify contacts about this host problem.";
//    return false;
//  }
//
//  /* check if its time to re-notify the contacts about the host... */
//  if (current_time < get_next_notification()) {
//    logger(dbg_notifications, more)
//        << "Its not yet time to re-notify the contacts "
//           "about this host problem...";
//    time_t time = get_next_notification();
//    logger(dbg_notifications, more)
//        << "Next acceptable notification time: " << my_ctime(&time);
//    return false;
//  }
//
//  return true;
//}

/* top level host state handler - occurs after every host check (soft/hard and
 * active/passive) */
int host::handle_state() {
  bool state_change = false;
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
    case  host::state_up:
      set_last_time_up(current_time);
      break;

    case  host::state_down:
      set_last_time_down(current_time);
      break;

    case  host::state_unreachable:
      set_last_time_unreachable(current_time);
      break;

    default:
      break;
  }

  /* has the host state changed? */
  if (get_last_state() != get_current_state() ||
      get_last_hard_state() != get_current_state() ||
      (get_current_state() ==  host::state_up && get_state_type() == soft))
    state_change = true;

  /* if the host state has changed... */
  if (state_change) {
    /* update last state change times */
    if (get_state_type() == soft ||
        get_last_state() != get_current_state())
      set_last_state_change(current_time);
    if (get_state_type() == hard)
      set_last_hard_state_change(current_time);

    /* update the event id */
    set_last_event_id(get_current_event_id());
    set_current_event_id(next_event_id);
    next_event_id++;

    /* update the problem id when transitioning to a problem state */
    if (get_last_state() ==  host::state_up) {
      /* don't reset last problem id, or it will be zero the next time a problem
       * is encountered */
      /*this->get_last_problem_id=this->get_current_problem_id; */
      set_current_problem_id(next_problem_id);
      next_problem_id++;
    }

    /* clear the problem id when transitioning from a problem state to an UP
     * state */
    if (get_current_state() ==  host::state_up) {
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
               get_current_state() ==  host::state_up) {
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
    if (get_state_type() == hard ||
        (get_state_type() == soft && config->log_host_retries() == true))
      log_event();

    /* check for start of flexible (non-fixed) scheduled downtime */
    /* CHANGED 08-05-2010 EG flex downtime can now start on soft states */
    /*if(this->state_type==hard) */
    downtime_manager::instance().check_pending_flex_host_downtime(this);

    /* notify contacts about the recovery or problem if its a "hard" state */
    if (get_current_state_int() == 0)
      notify(reason_recovery, "", "", notifier::notification_option_none);
    else
      notify(reason_normal, "", "", notifier::notification_option_none);

    /* handle the host state change */
    handle_host_event(this);

    /* the host just recovered, so reset the current host attempt */
    if (get_current_state() ==  host::state_up)
      set_current_attempt(1);
  }
  /* else the host state has not changed */
  else {
    /* notify contacts if needed */
    if (get_current_state() != host::state_up)
      notify(reason_normal,
             "",
             "",
             notifier::notification_option_none);
    else
      notify(reason_recovery,
             "",
             "",
             notifier::notification_option_none);

    /* if we're in a soft state and we should log host retries, do so now... */
    if (get_state_type() == soft && config->log_host_retries())
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
  if (this->get_state_type() == soft &&
      this->get_current_state() !=  host::state_up)
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
      if (!check_time_against_period(static_cast<unsigned long>(current_time),
                                    this->check_period_ptr)) {
        preferred_time = current_time;
        if (time_is_valid)
          *time_is_valid = false;
        perform_check = false;
      }
    }

    /* check host dependencies for execution */
    if (!authorized_by_dependencies(hostdependency::execution)) {
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
                         notifier::reason_type type,
                         std::string const& not_author,
                         std::string const& not_data,
                         int options __attribute((unused)),
                         int escalated) {
  std::string raw_command;
  std::string processed_command;
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
  notification_category cat{get_category(type)};
  if (!cntct->should_be_notified(cat, type, *this))
    return ERROR;
//  if (cntct->check_host_notification_viability(this, cat, options) ==
//      ERROR)
//    return ERROR;

  logger(dbg_notifications, most)
      << "** Notifying contact '" << cntct->get_name() << "'";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* send data to event broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_contact_notification_data(
      NEBTYPE_CONTACTNOTIFICATION_START, NEBFLAG_NONE, NEBATTR_NONE,
      host_notification, type, start_time, end_time, (void*)this, cntct,
      not_author.c_str(), not_data.c_str(), escalated, nullptr);
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
        host_notification, type, method_start_time, method_end_time,
        (void*)this, cntct, cmd->get_command_line().c_str(), not_author.c_str(),
        not_data.c_str(), escalated, nullptr);
    if (NEBERROR_CALLBACKCANCEL == neb_result)
      break;
    else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
      continue;

    /* get the raw command line */
    get_raw_command_line_r(mac, cmd.get(), cmd->get_command_line().c_str(),
                           raw_command, macro_options);
    if (raw_command.empty())
      continue;

    logger(dbg_notifications, most)
        << "Raw notification command: " << raw_command;

    /* process any macros contained in the argument */
    process_macros_r(mac, raw_command, processed_command, macro_options);
    if (processed_command.empty())
      continue;

    /* run the notification command... */

    logger(dbg_notifications, most)
        << "Processed notification command: " << processed_command;

    /* log the notification to program log file */
    if (config->log_notifications()) {
      char const* host_state_str("UP");
      if ((unsigned int)_current_state < tab_host_states.size())
        // sizeof(tab_host_state_str) / sizeof(*tab_host_state_str))
        host_state_str =
            tab_host_states[_current_state].second.c_str();

      char const* notification_str("");
      if ((unsigned int)type < tab_notification_str.size())
        notification_str = tab_notification_str[type].c_str();

      std::string info;
      if (type == reason_custom)
        notification_str = "CUSTOM";
      else if (type == reason_acknowledgement)
        info.append(";").append(not_author).append(";").append(not_data);

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
      std::string out;
      my_system_r(mac, processed_command, config->notification_timeout(),
                  &early_timeout, &exectime, out, 0);
    } catch (std::exception const& e) {
      logger(log_runtime_error, basic)
          << "Error: can't execute host notification '" << cntct->get_name()
          << "' : " << e.what();
    }

    /* check to see if the notification timed out */
    if (early_timeout) {
      logger(log_host_notification | log_runtime_warning, basic)
          << "Warning: Contact '" << cntct->get_name()
          << "' host notification command '" << processed_command
          << "' timed out after " << config->notification_timeout()
          << " seconds";
    }

    /* get end time */
    gettimeofday(&method_end_time, nullptr);

    /* send data to event broker */
    broker_contact_notification_method_data(
        NEBTYPE_CONTACTNOTIFICATIONMETHOD_END, NEBFLAG_NONE, NEBATTR_NONE,
        host_notification, type, method_start_time, method_end_time,
        (void*)this, cntct, cmd->get_command_line().c_str(), not_author.c_str(),
        not_data.c_str(), escalated, nullptr);
  }

  /* get end time */
  gettimeofday(&end_time, nullptr);

  /* update the contact's last host notification time */
  cntct->set_last_host_notification(start_time.tv_sec);

  /* send data to event broker */
  broker_contact_notification_data(
      NEBTYPE_CONTACTNOTIFICATION_END, NEBFLAG_NONE, NEBATTR_NONE,
      host_notification, type, start_time, end_time, (void*)this, cntct,
      not_author.c_str(), not_data.c_str(), escalated, nullptr);

  return OK;
}

void host::update_notification_flags() {
  /* update notifications flags */
  if (get_current_state() ==  host::state_down)
    add_notified_on(down);
  else if (get_current_state() ==  host::state_unreachable)
    add_notified_on(unreachable);
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
  add_modified_attributes(attr);

  /* set the flap detection enabled flag */
  set_flap_detection_enabled(false);

  /* send data to event broker */
  broker_adaptive_host_data(NEBTYPE_ADAPTIVEHOST_UPDATE, NEBFLAG_NONE,
                            NEBATTR_NONE, this, CMD_NONE, attr,
                            get_modified_attributes(), nullptr);

  /* handle the details... */
  handle_flap_detection_disabled();
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
  add_modified_attributes(attr);

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
bool host::is_valid_escalation_for_notification(escalation const* e,
                                                int options) const {
  uint32_t notification_number;
  time_t current_time;

  logger(dbg_functions, basic)
      << "host::is_valid_escalation_for_notification()";

  /* get the current time */
  time(&current_time);

  /*
   * if this is a recovery, really we check for who got notified about a
   * previous problem
   */
  if (get_current_state() ==  host::state_up)
    notification_number = get_notification_number() - 1;
  else
    notification_number = get_notification_number();

  /* find the host this escalation entry is associated with */
  if (e->notifier_ptr != this)
    return false;

  /*** EXCEPTION ***/
  /* broadcast options go to everyone, so this escalation is valid */
  if (options & notifier::notification_option_broadcast)
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
      !check_time_against_period(current_time, e->escalation_period_ptr))
    return false;

  /* skip this escalation if the state options don't match */
  if (get_current_state() ==  host::state_up && !e->get_escalate_on(up))
    return false;
  else if (get_current_state() ==  host::state_down &&
           !e->get_escalate_on(down))
    return false;
  else if (get_current_state() ==  host::state_unreachable &&
           !e->get_escalate_on(unreachable))
    return false;

  return true;
}

/* checks to see if a hosts's check results are fresh */
bool host::is_result_fresh(
      time_t current_time,
      int log_this) {
  time_t expiration_time = 0L;
  int freshness_threshold = 0;
  int days = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  int tdays = 0;
  int thours = 0;
  int tminutes = 0;
  int tseconds = 0;

  logger(dbg_checks, most)
    << "Checking freshness of host '" << _name << "'...";

  /* use user-supplied freshness threshold or auto-calculate a freshness threshold to use? */
  if (get_freshness_threshold() == 0) {
    double interval;
    if ((hard == get_state_type())
        || (host::state_up == get_current_state()))
      interval = get_check_interval();
    else
      interval = get_retry_interval();
    freshness_threshold
      = static_cast<int>((interval * config->interval_length())
                         + get_latency()
                         + config->additional_freshness_latency());
  }
  else
    freshness_threshold = get_freshness_threshold();

  logger(dbg_checks, most)
    << "Freshness thresholds: host=" << get_freshness_threshold()
    << ", use=" << freshness_threshold;

  /* calculate expiration time */
  /* CHANGED 11/10/05 EG - program start is only used in expiration time calculation if > last check AND active checks are enabled, so active checks can become stale immediately upon program startup */
  if (!get_has_been_checked())
    expiration_time = (time_t)(event_start + freshness_threshold);
  /* CHANGED 06/19/07 EG - Per Ton's suggestion (and user requests), only use program start time over last check if no specific threshold has been set by user.  Otheriwse use it.  Problems can occur if Engine is restarted more frequently that freshness threshold intervals (hosts never go stale). */
  /* CHANGED 10/07/07 EG - Added max_host_check_spread to expiration time as suggested by Altinity */
  else if (get_checks_enabled()
           && event_start > get_last_check()
           && get_freshness_threshold() == 0)
    expiration_time
      = (time_t)(event_start + freshness_threshold
                 + (config->max_host_check_spread()
                    * config->interval_length()));
  else
    expiration_time
      = (time_t)(get_last_check() + freshness_threshold);

  logger(dbg_checks, most)
    << "HBC: " << get_has_been_checked()
    << ", PS: " << program_start
    << ", ES: " << event_start
    << ", LC: " << get_last_check()
    << ", CT: " << current_time
    << ", ET: " << expiration_time;

  /* the results for the last check of this host are stale */
  if (expiration_time < current_time) {
    get_time_breakdown(
      (current_time - expiration_time),
      &days,
      &hours,
      &minutes,
      &seconds);
    get_time_breakdown(
      freshness_threshold,
      &tdays,
      &thours,
      &tminutes,
      &tseconds);

    /* log a warning */
    if (log_this)
      logger(log_runtime_warning, basic)
        << "Warning: The results of host '" << _name
        << "' are stale by " << days << "d " << hours << "h "
        << minutes << "m " << seconds << "s (threshold="
        << tdays << "d " << thours << "h " << tminutes << "m "
        << tseconds << "s).  I'm forcing an immediate check of"
        " the host.";

    logger(dbg_checks, more)
      << "Check results for host '" << _name
      << "' are stale by " << days << "d " << hours << "h " << minutes
      << "m " << seconds << "s (threshold=" << tdays << "d " << thours
      << "h " << tminutes << "m " << tseconds << "s).  "
      "Forcing an immediate check of the host...";

    return false;
  }
  else
    logger(dbg_checks, more)
      << "Check results for host '" << this->get_name()
      << "' are fresh.";

  return true;
}

/* handles the details for a host when flap detection is disabled (globally or per-host) */
void host::handle_flap_detection_disabled() {
  logger(dbg_functions, basic)
    << "handle_host_flap_detection_disabled()";

  /* if the host was flapping, remove the flapping indicator */
  if (get_is_flapping()) {
    this->set_is_flapping(false);

    /* delete the original comment we added earlier */
    if (this->get_flapping_comment_id() != 0)
      comment::delete_comment(this->get_flapping_comment_id());
    this->set_flapping_comment_id(0);

    /* log a notice - this one is parsed by the history CGI */
    logger(log_info_message, basic)
      << "HOST FLAPPING ALERT: " << this->get_name()
      << ";DISABLED; Flap detection has been disabled";

    /* send data to event broker */
    broker_flapping_data(
      NEBTYPE_FLAPPING_STOP,
      NEBFLAG_NONE,
      NEBATTR_FLAPPING_STOP_DISABLED,
      HOST_FLAPPING,
      this,
      this->get_percent_state_change(),
      0.0,
      0.0,
      nullptr);

    /* send a notification */
    notify(
      reason_flappingdisabled,
      "",
      "",
      notifier::notification_option_none);

    /* Send a recovery notification if needed */
    notify(reason_normal, "", "", notification_option_none);
  }

  /* update host status */
  this->update_status(false);
}

int host::perform_on_demand_check(enum host::host_state* check_return_code,
                                  int check_options,
                                  int use_cached_result,
                                  unsigned long check_timestamp_horizon) {
  logger(dbg_functions, basic) << "perform_on_demand_host_check()";

  perform_on_demand_check_3x(check_return_code, check_options,
                             use_cached_result, check_timestamp_horizon);
  return OK;
}

/* check to see if we can reach the host */
int host::perform_on_demand_check_3x(host::host_state* check_result_code,
  int check_options, int use_cached_result, unsigned long check_timestamp_horizon) {
  int result = OK;

  logger(dbg_functions, basic)
    << "perform_on_demand_host_check_3x()";


  logger(dbg_checks, basic)
    << "** On-demand check for host '" << _name << "'...";

  /* check the status of the host */
  result = this->run_sync_check_3x(
    check_result_code,
    check_options,
    use_cached_result,
    check_timestamp_horizon);
  return result;
}

/* perform a synchronous check of a host *//* on-demand host checks will use this... */
int host::run_sync_check_3x(enum host::host_state* check_result_code,
  int check_options, int use_cached_result, unsigned long check_timestamp_horizon) {
  logger(dbg_functions, basic)
    << "run_sync_host_check_3x: hst=" << this
    << ", check_options=" << check_options
    << ", use_cached_result=" << use_cached_result
    << ", check_timestamp_horizon=" << check_timestamp_horizon;

  try {
    checks::checker::instance().run_sync(
      this,
      check_result_code,
      check_options,
      use_cached_result,
      check_timestamp_horizon);
  }
  catch (checks::viability_failure const& e) {
    // Do not log viability failures.
    (void)e;
    return ERROR;
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: " << e.what();
    return ERROR;
  }
  return OK;
}

/* processes the result of a synchronous or asynchronous host check */
int host::process_check_result_3x(enum host::host_state new_state,
                                 std::string const& old_plugin_output,
                                 int check_options,
                                 int reschedule_check,
                                 int use_cached_result,
                                 unsigned long check_timestamp_horizon) {
  com::centreon::engine::host* master_host = nullptr;
  host* temp_host;
  std::list<host*> check_hostlist;
  host::host_state parent_state =  host::state_up;
  time_t current_time = 0L;
  time_t next_check = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int run_async_check = true;
  bool has_parent;

  logger(dbg_functions, basic) << "process_host_check_result_3x()";

  logger(dbg_checks, more)
    << "HOST: " << _name
    << ", ATTEMPT=" <<get_current_attempt() << "/"
    << get_max_attempts() << ", CHECK TYPE="
    << (get_check_type() == check_active ? "ACTIVE" : "PASSIVE")
    << ", STATE TYPE="
    << (get_state_type() == hard ? "HARD" : "SOFT")
    << ", OLD STATE=" << get_current_state()
    << ", NEW STATE=" << new_state;

  /* get the current time */
  time(&current_time);

  /* default next check time */
  next_check = (unsigned long)(current_time + (get_check_interval() *
    config->interval_length()));

  /* we have to adjust current attempt # for passive checks, as it isn't done
   * elsewhere */
  if (get_check_type() == check_passive &&
    config->passive_host_checks_are_soft())
    adjust_check_attempt(false);

  /* log passive checks - we need to do this here, as some my bypass external
   * commands by getting dropped in checkresults dir */
  if (get_check_type() == check_passive) {
    if (config->log_passive_checks())
      logger(log_passive_check, basic)
        << "PASSIVE HOST CHECK: " << _name << ";" << new_state
        << ";" << get_plugin_output();
  }

  /******* HOST WAS DOWN/UNREACHABLE INITIALLY *******/
  if (_current_state != host::state_up) {
    logger(dbg_checks, more) << "Host was DOWN/UNREACHABLE.";

    /***** HOST IS NOW UP *****/
    /* the host just recovered! */
    if (new_state ==  host::state_up) {
      /* set the current state */
      _current_state = host::state_up;

      /* set the state type */
      /* set state type to HARD for passive checks and active checks that were
       * previously in a HARD STATE */
      if (get_state_type() == hard ||
        (get_check_type() == check_passive &&
          !config->passive_host_checks_are_soft()))
        set_state_type(hard);
      else
        set_state_type(soft);

      logger(dbg_checks, more)
        << "Host experienced a "
        << (get_state_type() == hard ? "HARD" : "SOFT")
        << " recovery (it's now UP).";

      /* reschedule the next check of the host at the normal interval */
      reschedule_check = true;
      next_check = (unsigned long)(current_time + (get_check_interval() *
        config->interval_length()));

      /* propagate checks to immediate parents if they are not already UP */
      /* we do this because a parent host (or grandparent) may have recovered
       * somewhere and we should catch the recovery as soon as possible */
      logger(dbg_checks, more) << "Propagating checks to parent host(s)...";

      for (host_map_unsafe::iterator it{parent_hosts.begin()},
             end{parent_hosts.end()};
           it != end; it++) {
        if (!it->second)
          continue;
        if (it->second->get_current_state() != host::state_up) {
          logger(dbg_checks, more)
            << "Check of parent host '" << it->first << "' queued.";
          check_hostlist.push_back(it->second);
        }
      }

      /* propagate checks to immediate children if they are not already UP */
      /* we do this because children may currently be UNREACHABLE, but may (as a
       * result of this recovery) switch to UP or DOWN states */
      logger(dbg_checks, more) << "Propagating checks to child host(s)...";

      for (host_map_unsafe::iterator it{child_hosts.begin()},
             end{child_hosts.end()};
           it != end; it++) {
        if (!it->second)
          continue;
        if (it->second->get_current_state() != host::state_up) {
          logger(dbg_checks, more)
            << "Check of child host '" << it->first << "' queued.";
          check_hostlist.push_back(it->second);
        }
      }
    }

      /***** HOST IS STILL DOWN/UNREACHABLE *****/
      /* we're still in a problem state... */
    else {
      logger(dbg_checks, more) << "Host is still DOWN/UNREACHABLE.";

      /* passive checks are treated as HARD states by default... */
      if (get_check_type() == check_passive &&
        !config->passive_host_checks_are_soft()) {
        /* set the state type */
        set_state_type(hard);

        /* reset the current attempt */
        set_current_attempt(1);
      }

        /* active checks and passive checks (treated as SOFT states) */
      else {
        /* set the state type */
        /* we've maxed out on the retries */
        if (get_current_attempt() == get_max_attempts())
          set_state_type(hard);
          /* the host was in a hard problem state before, so it still is now */
        else if (get_current_attempt() == 1)
          set_state_type(hard);
          /* the host is in a soft state and the check will be retried */
        else
          set_state_type(soft);
      }

      /* make a determination of the host's state */
      /* translate host state between DOWN/UNREACHABLE (only for passive checks
       * if enabled) */
      _current_state = new_state;
      if (get_check_type() == check_active ||
        config->translate_passive_host_checks())
        _current_state = determine_host_reachability();

      /* reschedule the next check if the host state changed */
      if (_last_state != _current_state ||
        _last_hard_state !=_current_state) {
        reschedule_check = true;

        /* schedule a re-check of the host at the retry interval because we
         * can't determine its final state yet... */
        if (get_state_type() == soft)
          next_check =
            (unsigned long)(current_time + (get_retry_interval() *
              config->interval_length()));

          /* host has maxed out on retries (or was previously in a hard problem
           * state), so reschedule the next check at the normal interval */
        else
          next_check =
            (unsigned long)(current_time + (get_check_interval() *
              config->interval_length()));
      }
    }
  }

    /******* HOST WAS UP INITIALLY *******/
  else {
    logger(dbg_checks, more) << "Host was UP.";

    /***** HOST IS STILL UP *****/
    /* either the host never went down since last check */
    if (new_state ==  host::state_up) {
      logger(dbg_checks, more) << "Host is still UP.";

      /* set the current state */
      _current_state = host::state_up;

      /* set the state type */
      set_state_type(hard);

      /* reschedule the next check at the normal interval */
      if (reschedule_check)
        next_check =
          (unsigned long)(current_time + (get_check_interval() *
            config->interval_length()));
    }
      /***** HOST IS NOW DOWN/UNREACHABLE *****/
    else {
      logger(dbg_checks, more) << "Host is now DOWN/UNREACHABLE.";

      /***** SPECIAL CASE FOR HOSTS WITH MAX_ATTEMPTS==1 *****/
      if (get_max_attempts() == 1) {
        logger(dbg_checks, more) << "Max attempts = 1!.";

        /* set the state type */
        set_state_type(hard);

        /* host has maxed out on retries, so reschedule the next check at the
         * normal interval */
        reschedule_check = true;
        next_check =
          (unsigned long)(current_time + (get_check_interval() *
            config->interval_length()));

        /* we need to run SYNCHRONOUS checks of all parent hosts to accurately
         * determine the state of this host */
        /* this is extremely inefficient (reminiscent of Nagios 2.x logic), but
         * there's no other good way around it */
        /* check all parent hosts to see if we're DOWN or UNREACHABLE */
        /* only do this for ACTIVE checks, as PASSIVE checks contain a
         * pre-determined state */
        if (get_check_type() == check_active) {
          has_parent = false;

          logger(dbg_checks, more)
            << "** WARNING: Max attempts = 1, so we have to run serial "
               "checks of all parent hosts!";

          for (host_map_unsafe::iterator it{parent_hosts.begin()},
                 end{parent_hosts.end()};
               it != end; it++) {
            if (!it->second)
              continue;

            has_parent = true;

            logger(dbg_checks, more)
              << "Running serial check parent host '" << it->first << "'...";

            /* run an immediate check of the parent host */
            it->second->run_sync_check_3x( &parent_state,
                                                      check_options, use_cached_result,
                                                      check_timestamp_horizon);

            /* bail out as soon as we find one parent host that is UP */
            if (parent_state ==  host::state_up) {
              logger(dbg_checks, more)
                << "Parent host is UP, so this one is DOWN.";

              /* set the current state */
              _current_state = host::state_down;
              break;
            }
          }

          if (!has_parent) {
            /* host has no parents, so its up */
            if (parent_hosts.empty()) {
              logger(dbg_checks, more) << "Host has no parents, so it's DOWN.";
              _current_state = host::state_down;
            } else {
              /* no parents were up, so this host is UNREACHABLE */
              logger(dbg_checks, more)
                << "No parents were UP, so this host is UNREACHABLE.";
              _current_state = host::state_unreachable;
            }
          }
        }
          /* set the host state for passive checks */
        else {
          /* set the state */
          _current_state = new_state;

          /* translate host state between DOWN/UNREACHABLE for passive checks
           * (if enabled) */
          /* make a determination of the host's state */
          if (config->translate_passive_host_checks())
            _current_state = determine_host_reachability();
        }

        /* propagate checks to immediate children if they are not UNREACHABLE */
        /* we do this because we may now be blocking the route to child hosts */
        logger(dbg_checks, more)
          << "Propagating check to immediate non-UNREACHABLE child hosts...";

        for (host_map_unsafe::iterator it{child_hosts.begin()},
               end{child_hosts.end()};
             it != end; it++) {
          if (!it->second)
            continue;
          if (it->second->get_current_state() !=  host::state_unreachable) {
            logger(dbg_checks, more)
              << "Check of child host '" << it->first << "' queued.";
            check_hostlist.push_back(it->second);
          }
        }
      }
        /***** MAX ATTEMPTS > 1 *****/
      else {
        /* active and (in some cases) passive check results are treated as SOFT
         * states */
        if (get_check_type() == check_active ||
          config->passive_host_checks_are_soft()) {
          /* set the state type */
          set_state_type(soft);
        }
          /* by default, passive check results are treated as HARD states */
        else {
          /* set the state type */
          set_state_type(hard);

          /* reset the current attempt */
          set_current_attempt(1);
        }

        /* make a (in some cases) preliminary determination of the host's state
         */
        /* translate host state between DOWN/UNREACHABLE (for passive checks
         * only if enabled) */
        _current_state = new_state;
        if (get_check_type() == check_active ||
          config->translate_passive_host_checks())
          _current_state = determine_host_reachability();

        /* reschedule a check of the host */
        reschedule_check = true;

        /* schedule a re-check of the host at the retry interval because we
         * can't determine its final state yet... */
        if (get_check_type() == check_active ||
          config->passive_host_checks_are_soft())
          next_check =
            (unsigned long)(current_time + (get_retry_interval() *
              config->interval_length()));

          /* schedule a re-check of the host at the normal interval */
        else
          next_check =
            (unsigned long)(current_time + (get_check_interval() *
              config->interval_length()));

        /* propagate checks to immediate parents if they are UP */
        /* we do this because a parent host (or grandparent) may have gone down
         * and blocked our route */
        /* checking the parents ASAP will allow us to better determine the final
         * state (DOWN/UNREACHABLE) of this host later */
        logger(dbg_checks, more)
          << "Propagating checks to immediate parent hosts that "
             "are UP...";

        for (host_map_unsafe::iterator it{parent_hosts.begin()},
               end{parent_hosts.end()};
             it != end; it++) {
          if (it->second == nullptr)
            continue;
          if (it->second->get_current_state() ==  host::state_up) {
            check_hostlist.push_back(it->second);
            logger(dbg_checks, more)
              << "Check of host '" << it->first << "' queued.";
          }
        }

        /* propagate checks to immediate children if they are not UNREACHABLE */
        /* we do this because we may now be blocking the route to child hosts */
        logger(dbg_checks, more)
          << "Propagating checks to immediate non-UNREACHABLE "
             "child hosts...";

        for (host_map_unsafe::iterator it{child_hosts.begin()},
               end{child_hosts.end()};
             it != end; it++) {
          if (!it->second)
            continue;
          if (it->second->get_current_state() != host::state_unreachable) {
            logger(dbg_checks, more)
              << "Check of child host '" << it->first << "' queued.";
            check_hostlist.push_back(it->second);
          }
        }

        /* check dependencies on second to last host check */
        if (config->enable_predictive_host_dependency_checks() &&
          get_current_attempt() == (get_max_attempts() - 1)) {
          /* propagate checks to hosts that THIS ONE depends on for
           * notifications AND execution */
          /* we do to help ensure that the dependency checks are accurate before
           * it comes time to notify */
          logger(dbg_checks, more)
            << "Propagating predictive dependency checks to hosts this "
               "one depends on...";

          for (hostdependency_mmap::const_iterator
                 it{hostdependency::hostdependencies.find(_name)},
                 end{hostdependency::hostdependencies.end()};
               it != end && it->first == _name; ++it) {
            hostdependency* temp_dependency(it->second.get());
            if (temp_dependency->dependent_host_ptr == this &&
              temp_dependency->master_host_ptr != nullptr) {
              master_host = (host*) temp_dependency->master_host_ptr;
              logger(dbg_checks, more)
                << "Check of host '" << master_host->get_name()
                << "' queued.";
              check_hostlist.push_back(master_host);
            }
          }
        }
      }
    }
  }

  logger(dbg_checks, more) << "Pre-handle_host_state() Host: "
                           << _name
                           << ", Attempt=" << get_current_attempt() << "/"
                           << get_max_attempts() << ", Type="
                           << (get_state_type() == hard ? "HARD"
                                                                       : "SOFT")
                           << ", Final State=" << _current_state;

  /* handle the host state */
  handle_state();

  logger(dbg_checks, more) << "Post-handle_host_state() Host: "
                           << _name
                           << ", Attempt=" << get_current_attempt() << "/"
                           << get_max_attempts() << ", Type="
                           << (get_state_type() == hard ? "HARD"
                                                                       : "SOFT")
                           << ", Final State=" << _current_state;

  /******************** POST-PROCESSING STUFF *********************/

  /* if the plugin output differs from previous check and no state change, log
   * the current state/output if state stalking is enabled */
  if (_last_state == _current_state && old_plugin_output == get_plugin_output()) {
    if (_current_state ==  host::state_up && get_stalk_on(up))
      log_event();

    else if (_current_state ==  host::state_down && get_stalk_on(down))
      log_event();

    else if (_current_state ==  host::state_unreachable &&
      get_stalk_on(unreachable))
      log_event();
  }

  /* check to see if the associated host is flapping */
  check_for_flapping(true, true, true);

  /* reschedule the next check of the host (usually ONLY for scheduled, active
   * checks, unless overridden above) */
  if (reschedule_check) {
    logger(dbg_checks, more)
      << "Rescheduling next check of host at " << my_ctime(&next_check);

    /* default is to reschedule host check unless a test below fails... */
    set_should_be_scheduled(true);

    /* get the new current time */
    current_time = std::time(nullptr);

    /* make sure we don't get ourselves into too much trouble... */
    if (current_time > next_check)
      set_next_check(current_time);
    else
      set_next_check(next_check);

    // Make sure we rescheduled the next host check at a valid time.
    {
      timezone_locker lock{get_timezone()};
      preferred_time = get_next_check();
      get_next_valid_time(preferred_time, &next_valid_time,
                          check_period_ptr);
      set_next_check(next_valid_time);
    }

    /* hosts with non-recurring intervals do not get rescheduled if we're in a
     * HARD or UP state */
    if (get_check_interval() == 0 &&
      (get_state_type() == hard ||
        _current_state ==  host::state_up))
      set_should_be_scheduled(false);

    /* host with active checks disabled do not get rescheduled */
    if (!get_checks_enabled())
      set_should_be_scheduled(false);

    /* schedule a non-forced check if we can */
    if (get_should_be_scheduled()) {
      schedule_check(get_next_check(), CHECK_OPTION_NONE);
    }
  }

  /* update host status - for both active (scheduled) and passive
   * (non-scheduled) hosts */
  update_status(false);

  /* run async checks of all hosts we added above */
  /* don't run a check if one is already executing or we can get by with a
   * cached state */
  for (std::list<host*>::iterator
         it{check_hostlist.begin()},
         end{check_hostlist.end()};
       it != end;
       ++it) {
    run_async_check = true;
    temp_host = *it;

    logger(dbg_checks, most)
      << "ASYNC CHECK OF HOST: " << temp_host->get_name()
      << ", CURRENTTIME: " << current_time
      << ", LASTHOSTCHECK: " << temp_host->get_last_check()
      << ", CACHEDTIMEHORIZON: " << check_timestamp_horizon
      << ", USECACHEDRESULT: " << use_cached_result
      << ", ISEXECUTING: " << temp_host->get_is_executing();

    if (use_cached_result && (static_cast<unsigned long>(
      current_time - temp_host->get_last_check()) <=
      check_timestamp_horizon))
      run_async_check = false;
    if (temp_host->get_is_executing())
      run_async_check = false;
    if (run_async_check)
      temp_host->run_async_check(CHECK_OPTION_NONE, 0.0, false,
                                 false, nullptr, nullptr);
  }
  return OK;
}

/* determination of the host's state based on route availability*//* used only to determine difference between DOWN and UNREACHABLE states */
enum host::host_state host::determine_host_reachability() {
  enum host::host_state state = host::state_down;
  bool is_host_present = false;

  logger(dbg_functions, basic)
    << "determine_host_reachability()";

  logger(dbg_checks, most)
    << "Determining state of host '" << _name
    << "': current state=" << _current_state;

  /* host is UP - no translation needed */
  if (_current_state ==  host::state_up) {
    state =  host::state_up;
    logger(dbg_checks, most)
      << "Host is UP, no state translation needed.";
  }

    /* host has no parents, so it is DOWN */
  else if (parent_hosts.size() == 0) {
    state =  host::state_down;
    logger(dbg_checks, most)
      << "Host has no parents, so it is DOWN.";
  }

    /* check all parent hosts to see if we're DOWN or UNREACHABLE */
  else {

    for (host_map_unsafe::iterator
           it{parent_hosts.begin()},
           end{parent_hosts.end()};
         it != end;
         it++) {

      if (!it->second)
        continue;

      /* bail out as soon as we find one parent host that is UP */
      if (it->second->get_current_state() ==  host::state_up) {
        is_host_present = true;
        /* set the current state */
        state =  host::state_down;
        logger(dbg_checks, most)
          << "At least one parent (" << it->first
          << ") is up, so host is DOWN.";
        break;
      }
    }
    /* no parents were up, so this host is UNREACHABLE */
    if (!is_host_present) {
      state =  host::state_unreachable;
      logger(dbg_checks, most)
        << "No parents were up, so host is UNREACHABLE.";
    }
  }

  return state;
}

std::list<hostgroup*> const& host::get_parent_groups() const {
  return _hostgroups;
}

std::list<hostgroup*>& host::get_parent_groups() {
  return _hostgroups;
}

/* execute a scheduled host check using either the 2.x or 3.x logic */
int host::perform_scheduled_check(
  int check_options,
  double latency) {
  logger(dbg_functions, basic)
    << "perform_scheduled_host_check()";
  run_scheduled_check(check_options, latency);
  return OK;
}

/**
 *  This function returns a boolean telling if the master hosts of this one
 *  authorize it or forbide it to make its job (execution or notification).
 *
 * @param dependency_type execution / notification
 *
 * @return true if it is authorized.
 */
bool host::authorized_by_dependencies(dependency::types dependency_type) const {
  logger(dbg_functions, basic)
    << "host::authorized_by_dependencies()";

  auto p(hostdependency::hostdependencies.equal_range(_name));
  for (hostdependency_mmap::const_iterator it{p.first}, end{p.second};
       it != end; ++it) {
    hostdependency* dep{it->second.get()};
    /* Only check dependencies of the desired type (notification or execution) */
    if (dep->get_dependency_type() != dependency_type)
      continue;

    /* Find the host we depend on */
    if (!dep->master_host_ptr)
      continue;

    /* Skip this dependency if it has a timepriod and the current time is
     * not valid */
    time_t current_time{std::time(nullptr)};
    if (!dep->get_dependency_period().empty() &&
        !check_time_against_period(current_time, dep->dependency_period_ptr))
      return true;

    /* Get the status to use (use last hard state if it's currently in a soft
     * state) */
    host_state state =
        (dep->master_host_ptr->get_state_type() == notifier::soft &&
         !config->soft_state_dependencies())
            ? dep->master_host_ptr->get_last_hard_state()
            : dep->master_host_ptr->get_current_state();

    /* Is the host we depend on in state that fails the dependency tests? */
    if (dep->get_fail_on(state))
      return false;

    if (state == host::state_up &&
        !dep->master_host_ptr->get_has_been_checked() &&
        dep->get_fail_on_pending())
      return false;

    /* Immediate dependencies ok at this point - check parent dependencies if
     * necessary */
    if (dep->get_inherits_parent()) {
      if (!dep->master_host_ptr->authorized_by_dependencies(dependency_type))
        return false;
    }
  }
  return true;
}

/* check freshness of host results */
void host::check_result_freshness() {
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_host_result_freshness()";
  logger(dbg_checks, most)
    << "Attempting to check the freshness of host check results...";

  /* bail out if we're not supposed to be checking freshness */
  if (!config->check_host_freshness()) {
    logger(dbg_checks, most)
      << "Host freshness checking is disabled.";
    return;
  }

  /* get the current time */
  time(&current_time);

  /* check all hosts... */
  for (host_map::iterator
         it{host::hosts.begin()},
         end{host::hosts.end()};
       it != end;
       ++it) {

    /* skip hosts we shouldn't be checking for freshness */
    if (!it->second->get_check_freshness())
      continue;

    /* skip hosts that have both active and passive checks disabled */
    if (!it->second->get_checks_enabled()
      && !it->second->get_accept_passive_checks())
      continue;

    /* skip hosts that are currently executing (problems here will be caught by orphaned host check) */
    if (it->second->get_is_executing())
      continue;

    /* skip hosts that are already being freshened */
    if (it->second->get_is_being_freshened())
      continue;

    // See if the time is right...
    {
      timezone_locker lock(it->second->get_timezone());
      if (!check_time_against_period(
        current_time,
        it->second->check_period_ptr))
        continue ;
    }

    /* the results for the last check of this host are stale */
    if (!it->second->is_result_fresh(current_time, true)) {

      /* set the freshen flag */
      it->second->set_is_being_freshened(true);

      /* schedule an immediate forced check of the host */
      it->second->schedule_check(
        current_time,
        CHECK_OPTION_FORCE_EXECUTION | CHECK_OPTION_FRESHNESS_CHECK);
    }
  }
}

/**
 *  Adjusts current host check attempt before a new check is performed.
 *
 * @param is_active Boolean telling if the check is active or not.
 *
 */
void host::adjust_check_attempt(bool is_active) {
  logger(dbg_functions, basic)
    << "adjust_host_check_attempt_3x()";

  logger(dbg_checks, most)
    << "Adjusting check attempt number for host '" << _name
    << "': current attempt=" << get_current_attempt() << "/"
    << get_max_attempts() << ", state=" << _current_state
    << ", state type=" << get_state_type();

  /* if host is in a hard state, reset current attempt number */
  if (get_state_type() == notifier::hard)
    set_current_attempt(1);

    /* if host is in a soft UP state, reset current attempt number (active checks only) */
  else if (is_active && get_state_type() == notifier::soft
    && _current_state ==  host::state_up)
    set_current_attempt(1);

    /* increment current attempt number */
  else if (get_current_attempt() < get_max_attempts())
    set_current_attempt(get_current_attempt() + 1);

  logger(dbg_checks, most)
    << "New check attempt number = " << get_current_attempt();
}

/* check for hosts that never returned from a check... */
void host::check_for_orphaned() {
  time_t current_time = 0L;
  time_t expected_time = 0L;

  logger(dbg_functions, basic)
    << "check_for_orphaned_hosts()";

  /* get the current time */
  time(&current_time);

  /* check all hosts... */
  for (host_map::iterator
         it{host::hosts.begin()},
         end{host::hosts.end()};
       it != end;
       ++it) {

    /* skip hosts that don't have a set check interval (on-demand checks are missed by the orphan logic) */
    if (it->second->get_next_check() == (time_t)0L)
      continue;

    /* skip hosts that are not currently executing */
    if (!it->second->get_is_executing())
      continue;

    /* determine the time at which the check results should have come in (allow 10 minutes slack time) */
    expected_time
      = (time_t)(it->second->get_next_check() + it->second->get_latency()
      + config->host_check_timeout()
      + config->check_reaper_interval() + 600);

    /* this host was supposed to have executed a while ago, but for some reason the results haven't come back in... */
    if (expected_time < current_time) {

      /* log a warning */
      logger(log_runtime_warning, basic)
        << "Warning: The check of host '" << it->second->get_name()
        << "' looks like it was orphaned (results never came back).  "
           "I'm scheduling an immediate check of the host...";

      logger(dbg_checks, more)
        << "Host '" << it->second->get_name()
        << "' was orphaned, so we're scheduling an immediate check...";

      /* decrement the number of running host checks */
      if (currently_running_host_checks > 0)
        currently_running_host_checks--;

      /* disable the executing flag */
      it->second->set_is_executing(false);

      /* schedule an immediate check of the host */
      it->second->schedule_check(current_time, CHECK_OPTION_ORPHAN_CHECK);
    }
  }
}

std::string const& host::get_current_state_as_string() const {
  return tab_host_states[get_current_state()].second;
}

bool host::get_notify_on_current_state() const {
  notification_flag type[]{up, down, unreachable};
  bool retval = get_notify_on(type[get_current_state()]);
  return retval;
}

bool host::is_in_downtime() const {
  return get_scheduled_downtime_depth() > 0;
}

/**
 *  This method resolves pointers involved in this host life. If a pointer
 *  cannot be resolved, an exception is thrown.
 *
 * @param w Warnings given by the method.
 * @param e Errors given by the method. An exception is thrown is at less an
 * error is rised.
 */
void host::resolve(int& w, int& e) {
  int warnings{0}, errors{0};

  try {
    notifier::resolve(warnings, errors);
  }
  catch (std::exception const& e) {
    logger(log_verification_error, basic)
      << "Error: Host '" << _name
      << "' has problem in its notifier part: " << e.what();
  }

  for(service_map::iterator
        it_svc{service::services.begin()},
        end_svc{service::services.end()};
      it_svc != end_svc;
      ++it_svc) {
    if (_name == it_svc->first.first)
      services.insert({it_svc->first, nullptr});
  }

  if (services.empty()) {
    logger(log_verification_error, basic)
      << "Warning: Host '" << _name
      << "' has no services associated with it!";
    ++w;
  }
  else {
    for (service_map_unsafe::iterator
           it{services.begin()}, end{services.end()};
           it != end;
           ++it) {
      service_map::const_iterator found{service::services.find(it->first)};
      if (found == service::services.end() || !found->second) {
        logger(log_verification_error, basic)
          << "Error: Host '" << _name
          << "' has a service '" << it->first.second << "' that does not exist!";
        ++errors;
      }
      else {
        it->second = found->second.get();
      }
    }
  }

  /* check all parent parent host */
  for (host_map_unsafe::iterator
         it(parent_hosts.begin()),
         end(parent_hosts.end());
       it != end;
       it++) {

    host_map::const_iterator it_host{host::hosts.find(it->first)};
    if (it_host == host::hosts.end() || !it_host->second) {
      logger(log_verification_error, basic)
        << "Error: '" << it->first << "' is not a "
        "valid parent for host '" << _name << "'!";
      errors++;
    }
    else {
      it->second = it_host->second.get();
      it_host->second->add_child_host(this); //add a reverse (child) link to make searches faster later on
    }
  }

  /* check for illegal characters in host name */
  if (contains_illegal_object_chars(_name.c_str())) {
    logger(log_verification_error, basic)
      << "Error: The name of host '" << _name
      << "' contains one or more illegal characters.";
    errors++;
  }

  // Check for sane recovery options.
  if (get_notifications_enabled()
      && get_notify_on(notifier::up)
      && !get_notify_on(notifier::down)
      && !get_notify_on(notifier::unreachable)) {
    logger(log_verification_error, basic)
      << "Warning: Recovery notification option in host '" << get_display_name()
      << "' definition doesn't make any sense - specify down and/or "
         "unreachable options as well";
    warnings++;
  }

  w += warnings;
  e += errors;

  if (errors)
    throw engine_error() << "Cannot resolve host '" << _name << "'";
}

bool host::get_is_volatile() const {
  return false;
}

timeperiod* host::get_notification_timeperiod() const {
  /* if the service has no notification period, inherit one from the host */
  return get_notification_period_ptr();
}
