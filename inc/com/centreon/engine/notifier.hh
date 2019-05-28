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

#ifndef CCE_NOTIFIER_HH
# define CCE_NOTIFIER_HH

# include <array>
# include <string>
# include "com/centreon/engine/namespace.hh"

//Forward declarations
struct nagios_macros;

CCE_BEGIN()
class                         notifier {
 public:
  static std::array<std::string, 8> const tab_notification_str;
  static std::array<std::string, 2> const tab_state_type;

  static void                 inc_next_notification_id();

                              notifier(int notification_type,
                                       std::string const& display_name,
                                       std::string const& check_command,
                                       int initial_state,
                                       double check_interval,
                                       double retry_interval,
                                       int max_attempts);
  virtual                     ~notifier() {}

  std::string const&          get_display_name() const;
  void                        set_display_name(std::string const& name);
  std::string const&          get_check_command() const;
  void                        set_check_command(
                                std::string const& check_command);
  int                         get_initial_state() const;
  void                        set_initial_state(int initial_state);
  double                      get_check_interval() const;
  void                        set_check_interval(double check_interval);
  double                      get_retry_interval() const;
  void                        set_retry_interval(double retry_interval);
  void                        set_notification_number(int num);
  virtual void                update_status(bool aggregated_dump) = 0;
  int                         get_max_attempts() const;
  void                        set_max_attempts(int max_attempts);
  virtual int                 check_notification_viability(unsigned int type,
                                                           int options) = 0;
  int                         notify(unsigned int type,
                                     std::string const& not_author,
                                     std::string const& not_data,
                                     int options);

  void                        set_current_notification_id(uint64_t id);
  uint64_t                    get_current_notification_id() const;
  virtual int                 create_notification_list(nagios_macros* mac,
                                                       int options,
                                                       bool* escalated) = 0;
  virtual void                grab_macros_r(nagios_macros* mac) = 0;
  virtual int                 notify_contact(nagios_macros* mac,
                                             contact* cntct,
                                             int type,
                                             char const* not_author,
                                             char const* not_data,
                                             int options,
                                             int escalated) = 0;
  time_t                      get_next_notification() const;
  void                        set_next_notification(time_t next_notification);
  time_t                      get_last_notification() const;
  void                        set_last_notification(time_t last_notification);
  virtual void                update_notification_flags() = 0;
  int                         get_current_state() const;
  void                        set_current_state(int current_state);
  virtual time_t              get_next_notification_time(time_t offset) = 0;
  void                        set_initial_notif_time(time_t notif_time);
  time_t                      get_initial_notif_time() const;
  void                        set_acknowledgement_timeout(int timeout);
  void                        set_last_acknowledgement(time_t ack);
  time_t                      get_last_acknowledgement() const;
  void                        set_recovery_notification_delay(uint32_t delay);
  void                        set_recovery_been_sent(bool sent);
  bool                        get_recovery_been_sent() const;
  double                      get_notification_interval(void) const;
  void                        set_notification_interval(
                                               double notification_interval);

 protected:
  int                         _notification_type;
  std::string                 _display_name;
  std::string                 _check_command;
  int                         _initial_state;
  double                      _check_interval;
  double                      _retry_interval;
  int                         _current_notification_number;
  int                         _max_attempts;
  uint64_t                    _current_notification_id;
  time_t                      _next_notification;
  time_t                      _last_notification;
  int                         _current_state;

  time_t                      _initial_notif_time;
  int                         _acknowledgement_timeout;
  time_t                      _last_acknowledgement;
  uint32_t                    _recovery_notification_delay;
  bool                        _recovery_been_sent;
  double                      _notification_interval;

 private:
  static uint64_t             _next_notification_id;
};

CCE_END()

#endif // !CCE_NOTIFIER_HH
