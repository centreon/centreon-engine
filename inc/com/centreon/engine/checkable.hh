/*
** Copyright 2019 Centreon
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

#ifndef CCE_CHECKABLE_HH
#define CCE_CHECKABLE_HH

#include <ctime>
#include <string>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()
namespace commands {
  class command;
}
class timeperiod;

class checkable {
 public:
  enum check_type {
    check_active,  /* 0: Engine performed the check. */
    check_passive, /* 1: Check result submitted by an external source. */
  };

  enum state_type {
    soft,
    hard
  };

  checkable(std::string const& display_name,
            std::string const& check_command,
            bool checks_enabled,
            bool accept_passive_checks,
            double check_interval,
            double retry_interval,
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
            std::string const& timezone);
  virtual ~checkable() {}

  std::string const& get_display_name() const;
  void set_display_name(std::string const& name);
  std::string const& get_check_command() const;
  void set_check_command(std::string const& check_command);
  double get_check_interval() const;
  void set_check_interval(double check_interval);
  double get_retry_interval() const;
  void set_retry_interval(double retry_interval);
  int get_max_attempts() const;
  void set_max_attempts(int max_attempts);
  std::string const& get_check_period() const;
  void set_check_period(std::string const& check_period);
  std::string const& get_event_handler() const;
  void set_event_handler(std::string const& event_handler);
  bool get_event_handler_enabled() const;
  void set_event_handler_enabled(bool event_handler_enabled);
  std::string const& get_action_url() const;
  void set_action_url(std::string const& action_url);
  std::string const& get_icon_image() const;
  void set_icon_image(std::string const& icon_image);
  std::string const& get_icon_image_alt() const;
  void set_icon_image_alt(std::string const& icon_image_alt);
  std::string const& get_notes() const;
  void set_notes(std::string const& notes);
  std::string const& get_notes_url() const;
  void set_notes_url(std::string const& notes_url);
  std::string const& get_plugin_output() const;
  void set_plugin_output(std::string const& plugin_output);
  std::string const& get_long_plugin_output() const;
  void set_long_plugin_output(std::string const& long_plugin_output);
  std::string const& get_perf_data() const;
  void set_perf_data(std::string const& perf_data);
  bool get_flap_detection_enabled(void) const;
  void set_flap_detection_enabled(bool flap_detection_enabled);
  double get_low_flap_threshold() const;
  void set_low_flap_threshold(double low_flap_threshold);
  double get_high_flap_threshold() const;
  void set_high_flap_threshold(double high_flap_threshold);
  bool get_obsess_over() const;
  void set_obsess_over(bool obsess_over_host);
  std::string const& get_timezone() const;
  void set_timezone(std::string const& timezone);
  bool get_checks_enabled() const;
  void set_checks_enabled(bool checks_enabled);
  bool get_accept_passive_checks() const;
  void set_accept_passive_checks(bool accept_passive_checks);
  bool get_check_freshness() const;
  void set_check_freshness(bool check_freshness);
  std::time_t get_last_state_change() const;
  void set_last_state_change(std::time_t last_state_change);
  std::time_t get_last_hard_state_change() const;
  void set_last_hard_state_change(std::time_t last_hard_state_change);
  uint32_t get_state_history_index() const;
  void set_state_history_index(uint32_t state_history_index);
  int get_check_type() const;
  void set_check_type(int check_type);
  int get_current_attempt() const;
  void set_current_attempt(int current_attempt);
  void add_current_attempt(int num);
  bool get_has_been_checked() const;
  void set_has_been_checked(bool has_been_checked);
  int get_scheduled_downtime_depth() const;
  void set_scheduled_downtime_depth(int scheduled_downtime_depth);
  void dec_scheduled_downtime_depth();
  void inc_scheduled_downtime_depth();
  double get_execution_time() const;
  void set_execution_time(double execution_time);
  int get_freshness_threshold() const;
  void set_freshness_threshold(int freshness_threshold);
  bool get_is_flapping() const;
  void set_is_flapping(bool is_flapping);
  enum state_type get_state_type() const;
  void set_state_type(enum state_type state_type);
  double get_percent_state_change() const;
  void set_percent_state_change(double percent_state_change);
  std::time_t get_last_check() const;
  void set_last_check(time_t last_check);
  double get_latency() const;
  void set_latency(double latency);
  std::time_t get_next_check() const;
  void set_next_check(std::time_t next_check);
  bool get_should_be_scheduled() const;
  void set_should_be_scheduled(bool should_be_scheduled);
  virtual std::string const& get_current_state_as_string() const = 0;
  virtual bool is_in_downtime() const = 0;
  void set_event_handler_ptr(commands::command* cmd);
  commands::command* get_event_handler_ptr() const;
  void set_check_command_ptr(commands::command* cmd);
  commands::command* get_check_command_ptr() const;

  timeperiod *check_period_ptr;

 private:
  std::string _display_name;
  std::string _check_command;
  double _check_interval;
  double _retry_interval;
  int _max_attempts;
  std::string _check_period;
  std::string _event_handler;
  bool _event_handler_enabled;
  std::string _action_url;
  std::string _icon_image;
  std::string _icon_image_alt;
  std::string _notes;
  std::string _notes_url;
  std::string _plugin_output;
  std::string _long_plugin_output;
  std::string _perf_data;
  bool _flap_detection_enabled;
  double _low_flap_threshold;
  double _high_flap_threshold;
  bool _obsess_over;
  std::string _timezone;
  bool _checks_enabled;
  bool _accept_passive_checks;
  bool _check_freshness;
  int _freshness_threshold;
  int _check_type;
  int _current_attempt;
  bool _has_been_checked;
  int _scheduled_downtime_depth;
  double _execution_time;
  bool _is_flapping;
  int _last_check;
  double _latency;
  std::time_t _next_check;
  bool _should_be_scheduled;
  uint32_t _state_history_index;
  std::time_t _last_state_change;
  std::time_t _last_hard_state_change;
  enum state_type _state_type;
  double _percent_state_change;
  commands::command*  _event_handler_ptr;
  commands::command*  _check_command_ptr;
};

CCE_END()

#endif /* !CCE_CHECKABLE */
