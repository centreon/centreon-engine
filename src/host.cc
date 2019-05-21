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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/objects/servicesmember.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

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
 *  @param[in] failure_prediction_enabled    Deprecated.
 *  @param[in] failure_prediction_options    Deprecated.
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
 */
host::host(unsigned int host_id,
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
           int obsess_over_host) {
  // Make sure we have the data we need.
  if (name.empty() || address.empty()) {
    logger(log_config_error, basic)
    << "Error: Host name or address is nullptr";
    throw (engine_error() << "Could not register host '"
                          << name << "'");
  }
  if (host_id == 0) {
    logger(log_config_error, basic)
      << "Error: Host must contain a host id because it comes from a database";
    throw (engine_error() << "Could not register host '"
                          << name << "'");
  }
  if (max_attempts <= 0) {
    logger(log_config_error, basic)
      << "Error: Invalid max_check_attempts value for host '"
      << name << "'";
    throw (engine_error() << "Could not register host '"
                          << name << "'");
  }
  if (check_interval < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid check_interval value for host '"
      << name << "'";
    throw (engine_error() << "Could not register host '"
                          << name << "'");
  }
  if (notification_interval < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid notification_interval value for host '"
      << name << "'";
    throw (engine_error() << "Could not register host '"
                          << name << "'");
  }
  if (first_notification_delay < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid first_notification_delay value for host '"
      << name << "'";
    throw (engine_error() << "Could not register host '"
                          << name << "'");
  }
  if (freshness_threshold < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid freshness_threshold value for host '"
      << name << "'";
    throw (engine_error() << "Could not register host '"
                          << name << "'");
  }

  // Check if the host already exists.
  unsigned int id(host_id);
  if (is_host_exist(id)) {
    logger(log_config_error, basic)
      << "Error: Host '" << name << "' has already been defined";
    throw (engine_error() << "Could not register host '"
                          << name << "'");
  }

  _should_be_scheduled = true;
  _acknowledgement_type = ACKNOWLEDGEMENT_NONE;
  _check_options = CHECK_OPTION_NONE;
  _check_type = HOST_CHECK_ACTIVE;
  _modified_attributes = MODATTR_NONE;
  _state_type = HARD_STATE;

  // Duplicate string vars.
  _name = name;
  _address = address;
  _alias = !alias.empty() ? alias : name;
  _display_name = !display_name.empty() ? display_name : name;
  _action_url = action_url;
  _check_period = check_period;
  _event_handler = event_handler;
  _host_check_command = check_command;
  _icon_image = icon_image;
  _icon_image_alt = icon_image_alt;
  _notes = notes;
  _notes_url = notes_url;
  _notification_period = notification_period;
  _statusmap_image = statusmap_image;
  _vrml_image = vrml_image;

  _accept_passive_host_checks = (accept_passive_checks > 0);
  _check_freshness = (check_freshness > 0);
  _check_interval = check_interval;
  _checks_enabled = (checks_enabled > 0);
  _current_attempt = (initial_state == HOST_UP) ? 1 : max_attempts;
  _current_state = initial_state;
  _event_handler_enabled = (event_handler_enabled > 0);
  _first_notification_delay = first_notification_delay;
  _flap_detection_enabled = (flap_detection_enabled > 0);
  _flap_detection_on_down = (flap_detection_on_down > 0);
  _flap_detection_on_unreachable = (flap_detection_on_unreachable > 0);
  _flap_detection_on_up = (flap_detection_on_up > 0);
  _freshness_threshold = freshness_threshold;
  _have_2d_coords = (have_2d_coords > 0);
  _have_3d_coords = (have_3d_coords > 0);
  _high_flap_threshold = high_flap_threshold;
  _initial_state = initial_state;
  _last_hard_state = initial_state;
  _last_state = initial_state;
  _low_flap_threshold = low_flap_threshold;
  _max_attempts = max_attempts;
  _notification_interval = notification_interval;
  _notifications_enabled = (notifications_enabled > 0);
  _notify_on_down = (notify_down > 0);
  _notify_on_downtime = (notify_downtime > 0);
  _notify_on_flapping = (notify_flapping > 0);
  _notify_on_recovery = (notify_up > 0);
  _notify_on_unreachable = (notify_unreachable > 0);
  _obsess_over_host = (obsess_over_host > 0);
  _process_performance_data = (process_perfdata > 0);
  _retain_nonstatus_information = (retain_nonstatus_information > 0);
  _retain_status_information = (retain_status_information > 0);
  _retry_interval = retry_interval;
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
      throw (engine_error() << "add child link called with nullptr ptr");

  child_hosts.insert({child->get_name(), std::make_shared<com::centreon::engine::host>(*child)});

  // Notify event broker.
  timeval tv(get_broker_timestamp(nullptr));
  broker_relation_data(
    NEBTYPE_PARENT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    nullptr,
    child,
    nullptr,
    &tv);
}

void host::add_parent_host(std::string const& host_name) {
  // Make sure we have the data we need.
  if (host_name.empty()) {
    logger(log_config_error, basic) << "add child link called with bad host_name";
    throw (engine_error() << "add child link called with bad host_name");
  }

  // A host cannot be a parent/child of itself.
  if (_name == host_name) {
    logger(log_config_error, basic)
      << "Error: Host '" << _name
      << "' cannot be a child/parent of itself";
    throw (engine_error() << "host is child/parent itself");
  }

  parent_hosts.insert({host_name, nullptr});
}

std::string const& host::get_name() const {
  return _name;
}

void host::set_name(std::string const& name) {
  _name = name;
}

std::string const& host::get_display_name() const {
  return _display_name;
}

void host::set_display_name(std::string const& display_name) {
  _display_name = display_name;
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

std::string const& host::get_host_check_command() const {
  return _host_check_command;
}

void host::set_host_check_command(std::string const& host_check_command) {
  _host_check_command = host_check_command;
}

int host::get_initial_state() const {
  return _initial_state;
}

void host::set_initial_state(int initial_state) {
  _initial_state = initial_state;
}

double host::get_check_interval() const {
  return _check_interval;
}

void host::set_check_interval(double check_interval) {
  _check_interval = check_interval;
}

double host::get_retry_interval() const {
  return _retry_interval;
}

void host::set_retry_interval(double retry_interval) {
  _retry_interval = retry_interval;
}

int host::get_max_attempts() const {
  return _max_attempts;
}

void host::set_max_attempts(int max_attempts) {
  _max_attempts = max_attempts;
}

std::string const& host::get_event_handler() const {
  return _event_handler;
}

void host::set_event_handler(std::string const& event_handler) {
  _event_handler = event_handler;
}

double host::get_notification_interval(void) const {
  return _notification_interval;
}

void host::set_notification_interval(double notification_interval) {
  _notification_interval = notification_interval;
}

double host::get_first_notification_delay(void) const {
  return _first_notification_delay;
}

void host::set_first_notification_delay(double first_notification_delay) {
  _first_notification_delay = first_notification_delay;
}

int host::get_notify_on_down() const {
  return _notify_on_down;
}

void host::set_notify_on_down(int notify_on_down) {
  _notify_on_down = notify_on_down;
}

int host::get_notify_on_unreachable() const {
  return _notify_on_unreachable;
}

void host::set_notify_on_unreachable(int notify_on_unreachable) {
  _notify_on_unreachable = notify_on_unreachable;
}

int host::get_notify_on_recovery() const {
  return _notify_on_recovery;
}

void host::set_notify_on_recovery(int notify_on_recovery) {
  notify_on_recovery = _notify_on_recovery;
}

int host::get_notify_on_flapping() const {
  return _notify_on_flapping;
}

void host::set_notify_on_flapping(int notify_on_flapping) {
  _notify_on_flapping = notify_on_flapping;
}

int host::get_notify_on_downtime() const {
  return _notify_on_downtime;
}

void host::set_notify_on_downtime(int notify_on_downtime) {
  _notify_on_downtime = notify_on_downtime;
}

std::string const& host::get_notification_period() const {
  return _notification_period;
}

void  host::set_notification_period(std::string const& notification_period)
{
  _notification_period = notification_period;
}

std::string const& host::get_check_period() const {
  return _check_period;
}

void host::set_check_period(std::string const& check_period) {
  _check_period = check_period;
}

bool host::get_flap_detection_enabled(void) const {
  return _flap_detection_enabled;
}

void host::set_flap_detection_enabled(bool flap_detection_enabled) {
  _flap_detection_enabled = flap_detection_enabled;
}

double host::get_low_flap_threshold() const {
  return _low_flap_threshold;
}

void host::set_low_flap_threshold(double low_flap_threshold) {
  _low_flap_threshold = low_flap_threshold;
}

double host::get_high_flap_threshold() const {
  return _high_flap_threshold;
}

void host::set_high_flap_threshold(double high_flap_threshold) {
  _high_flap_threshold = high_flap_threshold;
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

int host::get_check_freshness() const {
  return _check_freshness;
}

void host::set_check_freshness(int check_freshness) {
  _check_freshness = check_freshness;
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

bool host::get_checks_enabled() const {
  return _checks_enabled;
}

void host::set_checks_enabled(bool checks_enabled) {
  _checks_enabled = checks_enabled;
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

std::string const& host::get_failure_prediction_options() const {
  return _failure_prediction_options;
}

void host::set_failure_prediction_options(std::string const& failure) {
  _failure_prediction_options = failure;
}

int host::get_obsess_over_host() const {
  return _obsess_over_host;
}

void host::set_obsess_over_host(int obsess_over_host) {
  _obsess_over_host = obsess_over_host;
}

std::string const& host::get_notes() const {
  return _notes;
}

void host::set_notes(std::string const& notes) {
  _notes = notes;
}

std::string const& host::get_notes_url() const {
  return _notes_url;
}

void host::set_notes_url(std::string const& notes_url) {
  _notes_url = notes_url;
}

std::string const& host::get_action_url() const {
  return _action_url;
}

void host::set_action_url(std::string const& action_url) {
  _action_url = action_url;
}

std::string const& host::get_icon_image() const {
  return _icon_image;
}

void host::set_icon_image(std::string const& icon_image) {
  _icon_image = icon_image;
}

std::string const& host::get_icon_image_alt() const {
  return _icon_image_alt;
}

void host::set_icon_image_alt(std::string const& icon_image_alt) {
  _icon_image_alt = icon_image_alt;
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

int host::get_problem_has_been_acknowledged() const {
  return _problem_has_been_acknowledged;
}

void host::set_problem_has_been_acknowledged(int problem_has_been_acknowledged) {
  _problem_has_been_acknowledged = problem_has_been_acknowledged;
}

int host::get_acknowledgement_type() const {
  return _acknowledgement_type;
}

void host::set_acknowledgement_type(int acknowledgement_type) {
  _acknowledgement_type = acknowledgement_type;
}

int host::get_check_type() const {
  return _check_type;
}

void host::set_check_type(int check_type) {
  _check_type = check_type;
}

int host::get_current_state() const {
  return _current_state;
}

void host::set_current_state(int current_state) {
  _current_state = current_state;
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

std::string const& host::get_plugin_output() const {
  return _plugin_output;
}

void host::set_plugin_output(std::string const& plugin_output) {
  _plugin_output = plugin_output;
}

std::string const& host::get_long_plugin_output() const {
  return _long_plugin_output;
}

void host::set_long_plugin_output(std::string const& long_plugin_output) {
  _long_plugin_output = long_plugin_output;
}

std::string const& host::get_perf_data() const {
  return _perf_data;
}

void host::set_perf_data(std::string const& perf_data) {
  _perf_data = perf_data;
}

int host::get_state_type() const {
  return _state_type;
}

void host::set_state_type(int state_type) {
  _state_type = state_type;
}

int host::get_current_attempt() const {
  return _current_attempt;
}

void host::set_current_attempt(int current_attempt) {
  _current_attempt = current_attempt;
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

bool host::get_notifications_enabled() const {
  return _notifications_enabled;
}

void host::set_notifications_enabled(bool notifications_enabled) {
  _notifications_enabled = notifications_enabled;
}

time_t host::get_last_host_notification() const {
  return _last_host_notification;
}

void host::set_last_host_notification(time_t last_host_notification) {
  _last_host_notification = last_host_notification;
}

time_t host::get_next_host_notification() const {
  return _next_host_notification;
}

void host::set_next_host_notification(time_t next_host_notification) {
  _next_host_notification = next_host_notification;
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

bool host::get_notified_on_down() const {
  return _notified_on_down;
}

void host::set_notified_on_down(bool notified_on_down) {
  _notified_on_down = notified_on_down;
}

bool host::get_notified_on_unreachable() const {
  return _notified_on_unreachable;
}

void host::set_notified_on_unreachable(bool notified_on_unreachable) {
  _notified_on_unreachable = notified_on_unreachable;
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

unsigned long host::get_current_notification_id() const {
  return _current_notification_id;
}

void host::set_current_notification_id(unsigned long current_notification_id) {
  _current_notification_id = current_notification_id;
}

int host::get_check_flapping_recovery_notification() const {
  return _check_flapping_recovery_notification;
}

void host::set_check_flapping_recovery_notification(int check_flapping_recovery_notification) {
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

void host::set_total_service_check_interval(unsigned long total_service_check_interval) {
  _total_service_check_interval = total_service_check_interval;
}

unsigned long host::get_modified_attributes() const {
  return _modified_attributes;
}

void host::set_modified_attributes(unsigned long modified_attributes) {
  _modified_attributes = modified_attributes;
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
bool operator==(
       host const& obj1,
       host const& obj2) throw () {
  return (obj1.get_name() == obj2.get_name())
          && obj1.get_display_name() == obj2.get_display_name()
          && obj1.get_alias() == obj2.get_alias()
          && obj1.get_address() == obj2.get_address()
           && ((obj1.parent_hosts.size() == obj2.parent_hosts.size()) &&
               std::equal(obj1.parent_hosts.begin(),
                          obj1.parent_hosts.end(),
                          obj2.parent_hosts.begin()))
          // Children do not need to be tested, they are
          // created as parent back links.
          // Services do not need to be tested, they are
          // created as services back links.
          && obj1.get_host_check_command() == obj2.get_host_check_command()
          && obj1.get_initial_state() == obj2.get_initial_state()
          && obj1.get_check_interval() == obj2.get_check_interval()
          && obj1.get_retry_interval() == obj2.get_retry_interval()
          && obj1.get_max_attempts() == obj2.get_max_attempts()
          && obj1.get_event_handler() == obj2.get_event_handler()
          && ((obj1.contact_groups.size() == obj2.contact_groups.size()) &&
               std::equal(obj1.contact_groups.begin(),
                          obj1.contact_groups.end(),
                          obj2.contact_groups.begin()))
          && ((obj1.contacts.size() == obj2.contacts.size()) &&
               std::equal(obj1.contacts.begin(),
                          obj1.contacts.end(),
                          obj2.contacts.begin()))
          && obj1.get_notification_interval() == obj2.get_notification_interval()
          && obj1.get_first_notification_delay() == obj2.get_first_notification_delay()
          && obj1.get_notify_on_down() == obj2.get_notify_on_down()
          && obj1.get_notify_on_unreachable() == obj2.get_notify_on_unreachable()
          && obj1.get_notify_on_recovery() == obj2.get_notify_on_recovery()
          && obj1.get_notify_on_flapping() == obj2.get_notify_on_flapping()
          && obj1.get_notify_on_downtime() == obj2.get_notify_on_downtime()
          && obj1.get_notification_period() == obj2.get_notification_period()
          && obj1.get_check_period() == obj2.get_check_period()
          && obj1.get_flap_detection_enabled() == obj2.get_flap_detection_enabled()
          && obj1.get_low_flap_threshold() == obj2.get_low_flap_threshold()
          && obj1.get_high_flap_threshold() == obj2.get_high_flap_threshold()
          && obj1.get_flap_detection_on_up() == obj2.get_flap_detection_on_up()
          && obj1.get_flap_detection_on_down() == obj2.get_flap_detection_on_down()
          && obj1.get_flap_detection_on_unreachable() == obj2.get_flap_detection_on_unreachable()
          && obj1.get_stalk_on_up() == obj2.get_stalk_on_up()
          && obj1.get_stalk_on_down() == obj2.get_stalk_on_down()
          && obj1.get_stalk_on_unreachable() == obj2.get_stalk_on_unreachable()
          && obj1.get_check_freshness() == obj2.get_check_freshness()
          && obj1.get_freshness_threshold() == obj2.get_freshness_threshold()
          && obj1.get_process_performance_data() == obj2.get_process_performance_data()
          && obj1.get_checks_enabled() == obj2.get_checks_enabled()
          && obj1.get_accept_passive_host_checks() == obj2.get_accept_passive_host_checks()
          && obj1.get_event_handler_enabled() == obj2.get_event_handler_enabled()
          && obj1.get_retain_status_information() == obj2.get_retain_status_information()
          && obj1.get_retain_nonstatus_information() == obj2.get_retain_nonstatus_information()
          && obj1.get_obsess_over_host() == obj2.get_obsess_over_host()
          && obj1.get_notes() == obj2.get_notes()
          && obj1.get_notes_url() == obj2.get_notes_url()
          && obj1.get_action_url() == obj2.get_action_url()
          && obj1.get_icon_image() == obj2.get_icon_image()
          && obj1.get_icon_image_alt() == obj2.get_icon_image_alt()
          && obj1.get_vrml_image() == obj2.get_vrml_image()
          && obj1.get_statusmap_image() == obj2.get_statusmap_image()
          && obj1.get_have_2d_coords() == obj2.get_have_2d_coords()
          && obj1.get_x_2d() == obj2.get_x_2d()
          && obj1.get_y_2d() == obj2.get_y_2d()
          && obj1.get_have_3d_coords() == obj2.get_have_3d_coords()
          && obj1.get_x_3d() == obj2.get_x_3d()
          && obj1.get_y_3d() == obj2.get_y_3d()
          && obj1.get_z_3d() == obj2.get_z_3d()
          && obj1.get_should_be_drawn() == obj2.get_should_be_drawn()
          && obj1.custom_variables == obj2.custom_variables
          && obj1.get_problem_has_been_acknowledged() == obj2.get_problem_has_been_acknowledged()
          && obj1.get_acknowledgement_type() == obj2.get_acknowledgement_type()
          && obj1.get_check_type() == obj2.get_check_type()
          && obj1.get_current_state() == obj2.get_current_state()
          && obj1.get_last_state() == obj2.get_last_state()
          && obj1.get_last_hard_state() == obj2.get_last_hard_state()
          && obj1.get_plugin_output() == obj2.get_plugin_output()
          && obj1.get_long_plugin_output() == obj2.get_long_plugin_output()
          && obj1.get_perf_data() == obj2.get_perf_data()
          && obj1.get_state_type() == obj2.get_state_type()
          && obj1.get_current_attempt() == obj2.get_current_attempt()
          && obj1.get_current_event_id() == obj2.get_current_event_id()
          && obj1.get_last_event_id() == obj2.get_last_event_id()
          && obj1.get_current_problem_id() == obj2.get_current_problem_id()
          && obj1.get_last_problem_id() == obj2.get_last_problem_id()
          && obj1.get_latency() == obj2.get_latency()
          && obj1.get_execution_time() == obj2.get_execution_time()
          && obj1.get_is_executing() == obj2.get_is_executing()
          && obj1.get_check_options() == obj2.get_check_options()
          && obj1.get_notifications_enabled() == obj2.get_notifications_enabled()
          && obj1.get_last_host_notification() == obj2.get_last_host_notification()
          && obj1.get_next_host_notification() == obj2.get_next_host_notification()
          && obj1.get_next_check() == obj2.get_next_check()
          && obj1.get_should_be_scheduled() == obj2.get_should_be_scheduled()
          && obj1.get_last_check() == obj2.get_last_check()
          && obj1.get_last_state_change() == obj2.get_last_state_change()
          && obj1.get_last_hard_state_change() == obj2.get_last_hard_state_change()
          && obj1.get_last_time_up() == obj2.get_last_time_up()
          && obj1.get_last_time_down() == obj2.get_last_time_down()
          && obj1.get_last_time_unreachable() == obj2.get_last_time_unreachable()
          && obj1.get_has_been_checked() == obj2.get_has_been_checked()
          && obj1.get_is_being_freshened() == obj2.get_is_being_freshened()
          && obj1.get_notified_on_down() == obj2.get_notified_on_down()
          && obj1.get_notified_on_unreachable() == obj2.get_notified_on_unreachable()
          && obj1.get_current_notification_number() == obj2.get_current_notification_number()
          && obj1.get_no_more_notifications() == obj2.get_no_more_notifications()
          && obj1.get_current_notification_id() == obj2.get_current_notification_id()
          && obj1.get_check_flapping_recovery_notification() == obj2.get_check_flapping_recovery_notification()
          && obj1.get_scheduled_downtime_depth() == obj2.get_scheduled_downtime_depth()
          && obj1.get_pending_flex_downtime() == obj2.get_pending_flex_downtime()
          && is_equal(obj1.state_history, obj2.state_history, MAX_STATE_HISTORY_ENTRIES)
          && obj1.get_state_history_index() == obj2.get_state_history_index()
          && obj1.get_last_state_history_update() == obj2.get_last_state_history_update()
          && obj1.get_is_flapping() == obj2.get_is_flapping()
          && obj1.get_flapping_comment_id() == obj2.get_flapping_comment_id()
          && obj1.get_percent_state_change() == obj2.get_percent_state_change()
          && obj1.get_total_services() == obj2.get_total_services()
          && obj1.get_total_service_check_interval() == obj2.get_total_service_check_interval()
          && obj1.get_modified_attributes() == obj2.get_modified_attributes()
          && obj1.get_circular_path_checked() == obj2.get_circular_path_checked()
          && obj1.get_contains_circular_path() == obj2.get_contains_circular_path();
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
  return !operator==(obj1, obj2);
}

std::ostream& operator<<(std::ostream& os, host_map const& obj)
{
  for (host_map::const_iterator
         it(obj.begin()), end(obj.end());
       it != end;
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

  char const* evt_str(nullptr);
  if (obj.event_handler_ptr)
    evt_str = obj.event_handler_ptr->get_name().c_str();
  char const* cmd_str(nullptr);
  if (obj.check_command_ptr)
    cmd_str = obj.check_command_ptr->get_name().c_str();
  char const* chk_period_str(nullptr);
  if (obj.check_period_ptr)
    chk_period_str = chkstr(obj.check_period_ptr->name);
  char const* notif_period_str(nullptr);
  if (obj.notification_period_ptr)
    notif_period_str = chkstr(obj.notification_period_ptr->name);

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
    "  name:                                 " << obj.get_name() << "\n"
    "  display_name:                         " << obj.get_display_name() << "\n"
    "  alias:                                " << obj.get_alias() << "\n"
    "  address:                              " << obj.get_address() << "\n"
    "  parent_hosts:                         " << p_oss << "\n"
    "  child_hosts:                          " << child_oss << "\n"
    "  services:                             " << chkobj(obj.services) << "\n"
    "  host_check_command:                   " << obj.get_host_check_command() << "\n"
    "  initial_state:                        " << obj.get_initial_state() << "\n"
    "  check_interval:                       " << obj.get_check_interval() << "\n"
    "  retry_interval:                       " << obj.get_retry_interval() << "\n"
    "  max_attempts:                         " << obj.get_max_attempts() << "\n"
    "  event_handler:                        " << obj.get_event_handler() << "\n"
    "  contact_groups:                       " << cg_oss << "\n"
    "  contacts:                             " << c_oss << "\n"
    "  notification_interval:                " << obj.get_notification_interval ()<< "\n"
    "  first_notification_delay:             " << obj.get_first_notification_delay() << "\n"
    "  notify_on_down:                       " << obj.get_notify_on_down() << "\n"
    "  notify_on_unreachable:                " << obj.get_notify_on_unreachable() << "\n"
    "  notify_on_recovery:                   " << obj.get_notify_on_recovery() << "\n"
    "  notify_on_flapping:                   " << obj.get_notify_on_flapping() << "\n"
    "  notify_on_downtime:                   " << obj.get_notify_on_downtime() << "\n"
    "  notification_period:                  " << obj.get_notification_period() << "\n"
    "  check_period:                         " << obj.get_check_period() << "\n"
    "  flap_detection_enabled:               " << obj.get_flap_detection_enabled() << "\n"
    "  low_flap_threshold:                   " << obj.get_low_flap_threshold() << "\n"
    "  high_flap_threshold:                  " << obj.get_high_flap_threshold() << "\n"
    "  flap_detection_on_up:                 " << obj.get_flap_detection_on_up() << "\n"
    "  flap_detection_on_down:               " << obj.get_flap_detection_on_down() << "\n"
    "  flap_detection_on_unreachable:        " << obj.get_flap_detection_on_unreachable() << "\n"
    "  stalk_on_up:                          " << obj.get_stalk_on_up() << "\n"
    "  stalk_on_down:                        " << obj.get_stalk_on_down() << "\n"
    "  stalk_on_unreachable:                 " << obj.get_stalk_on_unreachable() << "\n"
    "  check_freshness:                      " << obj.get_check_freshness() << "\n"
    "  freshness_threshold:                  " << obj.get_freshness_threshold() << "\n"
    "  process_performance_data:             " << obj.get_process_performance_data() << "\n"
    "  checks_enabled:                       " << obj.get_checks_enabled() << "\n"
    "  accept_passive_host_checks:           " << obj.get_accept_passive_host_checks() << "\n"
    "  event_handler_enabled:                " << obj.get_event_handler_enabled() << "\n"
    "  retain_status_information:            " << obj.get_retain_status_information() << "\n"
    "  retain_nonstatus_information:         " << obj.get_retain_nonstatus_information() << "\n"
    "  obsess_over_host:                     " << obj.get_obsess_over_host() << "\n"
    "  notes:                                " << obj.get_notes() << "\n"
    "  notes_url:                            " << obj.get_notes_url() << "\n"
    "  action_url:                           " << obj.get_action_url() << "\n"
    "  icon_image:                           " << obj.get_icon_image() << "\n"
    "  icon_image_alt:                       " << obj.get_icon_image_alt() << "\n"
    "  vrml_image:                           " << obj.get_vrml_image() << "\n"
    "  statusmap_image:                      " << obj.get_statusmap_image() << "\n"
    "  have_2d_coords:                       " << obj.get_have_2d_coords() << "\n"
    "  x_2d:                                 " << obj.get_x_2d() << "\n"
    "  y_2d:                                 " << obj.get_y_2d() << "\n"
    "  have_3d_coords:                       " << obj.get_have_3d_coords() << "\n"
    "  x_3d:                                 " << obj.get_x_3d() << "\n"
    "  y_3d:                                 " << obj.get_y_3d() << "\n"
    "  z_3d:                                 " << obj.get_z_3d() << "\n"
    "  should_be_drawn:                      " << obj.get_should_be_drawn() << "\n"
    "  problem_has_been_acknowledged:        " << obj.get_problem_has_been_acknowledged() << "\n"
    "  acknowledgement_type:                 " << obj.get_acknowledgement_type() << "\n"
    "  check_type:                           " << obj.get_check_type() << "\n"
    "  current_state:                        " << obj.get_current_state() << "\n"
    "  last_state:                           " << obj.get_last_state() << "\n"
    "  last_hard_state:                      " << obj.get_last_hard_state() << "\n"
    "  plugin_output:                        " << obj.get_plugin_output() << "\n"
    "  long_plugin_output:                   " << obj.get_long_plugin_output() << "\n"
    "  perf_data:                            " << obj.get_perf_data() << "\n"
    "  state_type:                           " << obj.get_state_type() << "\n"
    "  current_attempt:                      " << obj.get_current_attempt() << "\n"
    "  current_event_id:                     " << obj.get_current_event_id() << "\n"
    "  last_event_id:                        " << obj.get_last_event_id() << "\n"
    "  current_problem_id:                   " << obj.get_current_problem_id() << "\n"
    "  last_problem_id:                      " << obj.get_last_problem_id() << "\n"
    "  latency:                              " << obj.get_latency() << "\n"
    "  execution_time:                       " << obj.get_execution_time() << "\n"
    "  is_executing:                         " << obj.get_is_executing() << "\n"
    "  check_options:                        " << obj.get_check_options() << "\n"
    "  notifications_enabled:                " << obj.get_notifications_enabled() << "\n"
    "  last_host_notification:               " << string::ctime(obj.get_last_host_notification()) << "\n"
    "  next_host_notification:               " << string::ctime(obj.get_next_host_notification()) << "\n"
    "  next_check:                           " << string::ctime(obj.get_next_check()) << "\n"
    "  should_be_scheduled:                  " << obj.get_should_be_scheduled() << "\n"
    "  last_check:                           " << string::ctime(obj.get_last_check()) << "\n"
    "  last_state_change:                    " << string::ctime(obj.get_last_state_change()) << "\n"
    "  last_hard_state_change:               " << string::ctime(obj.get_last_hard_state_change()) << "\n"
    "  last_time_up:                         " << string::ctime(obj.get_last_time_up()) << "\n"
    "  last_time_down:                       " << string::ctime(obj.get_last_time_down()) << "\n"
    "  last_time_unreachable:                " << string::ctime(obj.get_last_time_unreachable()) << "\n"
    "  has_been_checked:                     " << obj.get_has_been_checked() << "\n"
    "  is_being_freshened:                   " << obj.get_is_being_freshened() << "\n"
    "  notified_on_down:                     " << obj.get_notified_on_down() << "\n"
    "  notified_on_unreachable:              " << obj.get_notified_on_unreachable() << "\n"
    "  current_notification_number:          " << obj.get_current_notification_number() << "\n"
    "  no_more_notifications:                " << obj.get_no_more_notifications() << "\n"
    "  current_notification_id:              " << obj.get_current_notification_id() << "\n"
    "  check_flapping_recovery_notification: " << obj.get_check_flapping_recovery_notification() << "\n"
    "  scheduled_downtime_depth:             " << obj.get_scheduled_downtime_depth() << "\n"
    "  pending_flex_downtime:                " << obj.get_pending_flex_downtime() << "\n";

  os << "  state_history:                        ";
  for (unsigned int i(0), end(sizeof(obj.state_history) / sizeof(obj.state_history[0]));
       i < end;
       ++i)
    os << obj.state_history[i] << (i + 1 < end ? ", " : "\n");

  os <<
    "  state_history_index:                  " << obj.get_state_history_index() << "\n"
    "  last_state_history_update:            " << string::ctime(obj.get_last_state_history_update()) << "\n"
    "  is_flapping:                          " << obj.get_is_flapping() << "\n"
    "  flapping_comment_id:                  " << obj.get_flapping_comment_id() << "\n"
    "  percent_state_change:                 " << obj.get_percent_state_change() << "\n"
    "  total_services:                       " << obj.get_total_services() << "\n"
    "  total_service_check_interval:         " << obj.get_total_service_check_interval() << "\n"
    "  modified_attributes:                  " << obj.get_modified_attributes() << "\n"
    "  circular_path_checked:                " << obj.get_circular_path_checked() << "\n"
    "  contains_circular_path:               " << obj.get_contains_circular_path() << "\n"
    "  event_handler_ptr:                    " << chkstr(evt_str) << "\n"
    "  check_command_ptr:                    " << chkstr(cmd_str) << "\n"
    "  check_period_ptr:                     " << chkstr(chk_period_str) << "\n"
    "  notification_period_ptr:              " << chkstr(notif_period_str) << "\n"
    "  hostgroups_ptr:                       " << hg->get_group_name() << "\n";

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
  for (contact_map::iterator
         it(hst->contacts.begin()),
         end(hst->contacts.end());
       it != end;
       ++it)
    if (it->second.get() == cntct)
      return true;

  for (contactgroup_map::iterator
         it(hst->contact_groups.begin()),
         end(hst->contact_groups.end());
       it != end;
       ++it)
    if (it->second->get_members().find(cntct->get_name()) ==
        it->second->get_members().end())
      return true;

  return false;
}

/**
 *  Tests whether or not a contact is an escalated contact for a
 *  particular host.
 *
 *  @param[in] hst   Target host.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_escalated_contact_for_host(com::centreon::engine::host* hst,
                                  contact* cntct) {
  if (!hst || !cntct)
    return false;

  std::string id(hst->get_name());
  umultimap<std::string, std::shared_ptr<hostescalation> > const&
    escalations(state::instance().hostescalations());

  for (umultimap<std::string, std::shared_ptr<hostescalation> >::const_iterator
         it(escalations.find(id)), end(escalations.end());
         it != end && it->first == id;
       ++it) {
    hostescalation* hstescalation(&*it->second);
    // Search all contacts of this host escalation.
  for (contact_map::iterator
         it(hstescalation->contacts.begin()),
         end(hstescalation->contacts.end());
       it != end;
       ++it)
    if (it->second.get() == cntct)
      return true;

  // Search all contactgroups of this host escalation.
  for (contactgroup_map::iterator
         it(hstescalation->contact_groups.begin()),
         end(hstescalation->contact_groups.begin());
       it != end;
       ++it)
    if (it->second->get_members().find(cntct->get_name()) ==
      it->second->get_members().end())
      return true;
  }

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
int is_host_immediate_child_of_host(
      com::centreon::engine::host* parent_host,
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
    for (host_map::iterator
           it(child_host->parent_hosts.begin()),
           end(child_host->parent_hosts.end());
         it != end;
         it++)
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
int is_host_immediate_parent_of_host(
      com::centreon::engine::host* child_host,
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
  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it)
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
  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it)
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
  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it)
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
  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it)
    if (is_host_immediate_parent_of_host(hst, it->second.get()))
      parents += number_of_total_parent_hosts(it->second.get()) + 1;
  return parents;
}

/**
 *  Check if acknowledgement on host expired.
 *
 *  @param[in] h  Target host.
 */
void engine::check_for_expired_acknowledgement(com::centreon::engine::host* h) {
  if (h->get_problem_has_been_acknowledged()) {
    int acknowledgement_timeout(
          host_other_props[h->get_name()].acknowledgement_timeout);
    if (acknowledgement_timeout > 0) {
      time_t last_ack(host_other_props[h->get_name()].last_acknowledgement);
      time_t now(time(nullptr));
      if (last_ack + acknowledgement_timeout >= now) {
        logger(log_info_message, basic)
          << "Acknowledgement of host '" << h->get_name() << "' just expired";
        h->set_problem_has_been_acknowledged(false);
        h->set_acknowledgement_type(ACKNOWLEDGEMENT_NONE);
        update_host_status(h, false);
      }
    }
  }
  return ;
}

/**
 *  Get host by id.
 *
 *  @param[in] host_id The host id.
 *
 *  @return The struct host or throw exception if the
 *          host is not found.
 */
host& engine::find_host(unsigned long host_id) {
  umap<unsigned long,
       std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it{state::instance().hosts().find(host_id)};
  if (it == state::instance().hosts().end())
    throw engine_error() << "Host '" << host_id << "' was not found";
  return *it->second;
}

/**
 *  Get host timezone.
 *
 *  @param[in] name  Host name.
 *
 *  @return Host timezone.
 */
char const* engine::get_host_timezone(std::string const& name) {
  std::string const& timezone(host_other_props[name].timezone);
  return timezone.empty() ? nullptr : timezone.c_str();
}

/**
 *  Get if host exist.
 *
 *  @param[in] name The host name.
 *
 *  @return True if the host is found, otherwise false.
 */
bool engine::is_host_exist(unsigned long host_id) throw () {
  umap<unsigned long,
       std::shared_ptr<com::centreon::engine::host>>::const_iterator
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
unsigned long engine::get_host_id(std::string const& name) {
  std::map<std::string, host_other_properties>::const_iterator
    found = host_other_props.find(name);
  return found != host_other_props.end() ? found->second.host_id : 0;
}

/**
 *  Schedule acknowledgement expiration.
 *
 *  @param[in] h  Target host.
 */
void engine::schedule_acknowledgement_expiration(com::centreon::engine::host* h) {
  int ack_timeout(host_other_props[h->get_name()].acknowledgement_timeout);
  time_t last_ack(host_other_props[h->get_name()].last_acknowledgement);
  if ((ack_timeout > 0) && (last_ack != (time_t)0)) {
    schedule_new_event(
      EVENT_EXPIRE_HOST_ACK,
      false,
      last_ack + ack_timeout,
      false,
      0,
      nullptr,
      true,
      h,
      nullptr,
      0);
  }
  return ;
}
