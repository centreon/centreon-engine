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

#include "com/centreon/engine/checkable.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

checkable::checkable(std::string const& display_name,
                     std::string const& check_command,
                     bool checks_enabled,
                     bool accept_passive_checks,
                     uint32_t check_interval,
                     uint32_t retry_interval,
                     int max_attempts,
                     std::string const& check_period,
                     std::string const& event_handler,
                     bool event_handler_enabled,
                     std::string const& notes,
                     std::string const& notes_url,
                     std::string const& action_url,
                     std::string const& icon_image,
                     std::string const& icon_image_alt,
                     bool flap_detection_enabled,
                     double low_flap_threshold,
                     double high_flap_threshold,
                     bool check_freshness,
                     int freshness_threshold,
                     bool obsess_over,
                     std::string const& timezone)
    : check_period_ptr{nullptr},
      _display_name{display_name},
      _check_command{check_command},
      _check_interval{check_interval},
      _retry_interval{retry_interval},
      _max_attempts{max_attempts},
      _check_period{check_period},
      _event_handler{event_handler},
      _event_handler_enabled{event_handler_enabled},
      _action_url{action_url},
      _icon_image{icon_image},
      _icon_image_alt{icon_image_alt},
      _notes{notes},
      _notes_url{notes_url},
      _flap_detection_enabled{flap_detection_enabled},
      _low_flap_threshold{low_flap_threshold},
      _high_flap_threshold{high_flap_threshold},
      _obsess_over{obsess_over},
      _timezone{timezone},
      _checks_enabled{checks_enabled},
      _accept_passive_checks{accept_passive_checks},
      _check_freshness{check_freshness},
      _freshness_threshold{freshness_threshold},
      _check_type{check_active},
      _current_attempt{0},
      _has_been_checked{false},
      _scheduled_downtime_depth{0},
      _execution_time{0.0},
      _is_flapping{false},
      _last_check{0},
      _latency{0.0},
      _next_check{0L},
      _should_be_scheduled{true},
      _state_history_index{0},
      _last_state_change{0},
      _last_hard_state_change{0},
      _state_type{soft},
      _percent_state_change{0.0},
      _event_handler_ptr{nullptr},
      _check_command_ptr{nullptr},
      _is_executing{false} {
  if (check_interval < 0) {
    logger(log_config_error, basic)
        << "Error: Invalid check_interval value for checkable '" << display_name
        << "'";
    throw engine_error() << "Could not register checkable '" << display_name
                         << "'";
  }

  if (max_attempts < 0 || check_interval < 0 || retry_interval <= 0) {
    logger(log_config_error, basic)
        << "Error: Invalid max_attempts, check_interval or retry_interval"
           " value for checkable '" << display_name << "'";
    throw engine_error() << "Could not register checkable '" << display_name
                         << "'";
  }

  if (max_attempts <= 0) {
    logger(log_config_error, basic)
        << "Error: Invalid max_check_attempts value for checkable '"
        << display_name << "'";
    throw engine_error() << "Could not register checkable '" << display_name
                         << "'";
  }

  if (freshness_threshold < 0) {
    logger(log_config_error, basic)
        << "Error: Invalid freshness_threshold value for checkable '"
        << display_name << "'";
    throw engine_error() << "Could not register checkable '" << display_name
                         << "'";
  }
}

std::string const& checkable::get_display_name() const { return _display_name; }

void checkable::set_display_name(std::string const& display_name) {
  _display_name = display_name;
}

std::string const& checkable::get_check_command() const {
  return _check_command;
}

void checkable::set_check_command(std::string const& check_command) {
  _check_command = check_command;
}

uint32_t checkable::get_check_interval() const { return _check_interval; }

void checkable::set_check_interval(uint32_t check_interval) {
  _check_interval = check_interval;
}

double checkable::get_retry_interval() const { return _retry_interval; }

void checkable::set_retry_interval(double retry_interval) {
  _retry_interval = retry_interval;
}

time_t checkable::get_last_state_change() const { return _last_state_change; }

void checkable::set_last_state_change(time_t last_state_change) {
  _last_state_change = last_state_change;
}

time_t checkable::get_last_hard_state_change() const {
  return _last_hard_state_change;
}

void checkable::set_last_hard_state_change(time_t last_hard_state_change) {
  _last_hard_state_change = last_hard_state_change;
}

int checkable::get_max_attempts() const { return _max_attempts; }

void checkable::set_max_attempts(int max_attempts) {
  _max_attempts = max_attempts;
}

std::string const& checkable::get_check_period() const { return _check_period; }

void checkable::set_check_period(std::string const& check_period) {
  _check_period = check_period;
}

std::string const& checkable::get_action_url() const { return _action_url; }

void checkable::set_action_url(std::string const& action_url) {
  _action_url = action_url;
}

std::string const& checkable::get_icon_image() const { return _icon_image; }

void checkable::set_icon_image(std::string const& icon_image) {
  _icon_image = icon_image;
}

std::string const& checkable::get_icon_image_alt() const {
  return _icon_image_alt;
}

void checkable::set_icon_image_alt(std::string const& icon_image_alt) {
  _icon_image_alt = icon_image_alt;
}

std::string const& checkable::get_event_handler() const {
  return _event_handler;
}

void checkable::set_event_handler(std::string const& event_handler) {
  _event_handler = event_handler;
}

std::string const& checkable::get_notes() const { return _notes; }

void checkable::set_notes(std::string const& notes) { _notes = notes; }

std::string const& checkable::get_notes_url() const { return _notes_url; }

void checkable::set_notes_url(std::string const& notes_url) {
  _notes_url = notes_url;
}

std::string const& checkable::get_plugin_output() const {
  return _plugin_output;
}

void checkable::set_plugin_output(std::string const& plugin_output) {
  _plugin_output = plugin_output;
}

std::string const& checkable::get_long_plugin_output() const {
  return _long_plugin_output;
}

void checkable::set_long_plugin_output(std::string const& long_plugin_output) {
  _long_plugin_output = long_plugin_output;
}

std::string const& checkable::get_perf_data() const { return _perf_data; }

void checkable::set_perf_data(std::string const& perf_data) {
  _perf_data = perf_data;
}

bool checkable::get_flap_detection_enabled(void) const {
  return _flap_detection_enabled;
}

void checkable::set_flap_detection_enabled(bool flap_detection_enabled) {
  _flap_detection_enabled = flap_detection_enabled;
}

double checkable::get_low_flap_threshold() const { return _low_flap_threshold; }

void checkable::set_low_flap_threshold(double low_flap_threshold) {
  _low_flap_threshold = low_flap_threshold;
}

double checkable::get_high_flap_threshold() const {
  return _high_flap_threshold;
}

void checkable::set_high_flap_threshold(double high_flap_threshold) {
  _high_flap_threshold = high_flap_threshold;
}

std::string const& checkable::get_timezone() const { return _timezone; }

void checkable::set_timezone(std::string const& timezone) {
  _timezone = timezone;
}

uint32_t checkable::get_state_history_index() const {
  return _state_history_index;
}

void checkable::set_state_history_index(uint32_t state_history_index) {
  _state_history_index = state_history_index;
}

bool checkable::get_checks_enabled() const { return _checks_enabled; }

void checkable::set_checks_enabled(bool checks_enabled) {
  _checks_enabled = checks_enabled;
}

bool checkable::get_check_freshness() const { return _check_freshness; }

void checkable::set_check_freshness(bool check_freshness) {
  _check_freshness = check_freshness;
}

int checkable::get_check_type() const { return _check_type; }

void checkable::set_check_type(int check_type) { _check_type = check_type; }

void checkable::set_current_attempt(int attempt) { _current_attempt = attempt; }

int checkable::get_current_attempt() const { return _current_attempt; }

void checkable::add_current_attempt(int num) { _current_attempt += num; }

bool checkable::get_has_been_checked() const { return _has_been_checked; }

void checkable::set_has_been_checked(bool has_been_checked) {
  _has_been_checked = has_been_checked;
}

bool checkable::get_event_handler_enabled() const {
  return _event_handler_enabled;
}

void checkable::set_event_handler_enabled(bool event_handler_enabled) {
  _event_handler_enabled = event_handler_enabled;
}

bool checkable::get_accept_passive_checks() const {
  return _accept_passive_checks;
}

void checkable::set_accept_passive_checks(bool accept_passive_checks) {
  _accept_passive_checks = accept_passive_checks;
}

int checkable::get_scheduled_downtime_depth() const {
  return _scheduled_downtime_depth;
}

void checkable::set_scheduled_downtime_depth(int scheduled_downtime_depth) {
  _scheduled_downtime_depth = scheduled_downtime_depth;
}

void checkable::inc_scheduled_downtime_depth() { ++_scheduled_downtime_depth; }

void checkable::dec_scheduled_downtime_depth() { --_scheduled_downtime_depth; }

double checkable::get_execution_time() const { return _execution_time; }

void checkable::set_execution_time(double execution_time) {
  _execution_time = execution_time;
}

int checkable::get_freshness_threshold() const { return _freshness_threshold; }

void checkable::set_freshness_threshold(int freshness_threshold) {
  _freshness_threshold = freshness_threshold;
}

bool checkable::get_is_flapping() const { return _is_flapping; }

void checkable::set_is_flapping(bool is_flapping) {
  _is_flapping = is_flapping;
}

std::time_t checkable::get_last_check() const { return _last_check; }

void checkable::set_last_check(time_t last_check) { _last_check = last_check; }

double checkable::get_latency() const { return _latency; }

void checkable::set_latency(double latency) { _latency = latency; }

std::time_t checkable::get_next_check() const { return _next_check; }

void checkable::set_next_check(std::time_t next_check) {
  _next_check = next_check;
}

enum checkable::state_type checkable::get_state_type() const {
  return _state_type;
}

void checkable::set_state_type(enum checkable::state_type state_type) {
  _state_type = state_type;
}

double checkable::get_percent_state_change() const {
  return _percent_state_change;
}

void checkable::set_percent_state_change(double percent_state_change) {
  _percent_state_change = percent_state_change;
}

bool checkable::get_obsess_over() const { return _obsess_over; }

void checkable::set_obsess_over(bool obsess_over) {
  _obsess_over = obsess_over;
}

bool checkable::get_should_be_scheduled() const { return _should_be_scheduled; }

void checkable::set_should_be_scheduled(bool should_be_scheduled) {
  _should_be_scheduled = should_be_scheduled;
}

commands::command* checkable::get_event_handler_ptr() const {
  return _event_handler_ptr;
}

void checkable::set_event_handler_ptr(commands::command* cmd) {
  _event_handler_ptr = cmd;
}

commands::command* checkable::get_check_command_ptr() const {
  return _check_command_ptr;
}

void checkable::set_check_command_ptr(commands::command* cmd) {
  _check_command_ptr = cmd;
}

bool checkable::get_is_executing() const {
  return _is_executing;
}

void checkable::set_is_executing(bool is_executing) {
  _is_executing = is_executing;
}
