/*
 * Copyright 2017 - 2019 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#ifndef CCE_CONTACT_HH
#define CCE_CONTACT_HH

#include <time.h>
#include <list>
#include <memory>
#include <ostream>
#include <vector>
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/customvariable.hh"
#include "com/centreon/engine/notifier.hh"

/* Max number of custom addresses a contact can have. */
#define MAX_CONTACT_ADDRESSES 6

/* Forward declaration. */
typedef std::unordered_map<std::string,
                           std::shared_ptr<com::centreon::engine::contact>>
    contact_map;
typedef std::unordered_map<std::string, com::centreon::engine::contact*>
    contact_map_unsafe;

CCE_BEGIN()
class host;
class service;
class timeperiod;

namespace commands {
class command;
}

/**
 *  @class contact contact.hh "com/centreon/engine/objects/contact.hh
 *  @brief Object representing a contact user
 *
 */
class contact {
 public:
  typedef bool (contact::*to_notify)(notifier::reason_type,
                                     notifier const&) const;
  contact();
  ~contact();
  contact(contact const& other) = delete;
  contact& operator=(contact const& other) = delete;
  bool operator==(contact const& other) = delete;
  bool operator!=(contact const& other) = delete;
  void update_status_info(bool aggregated_dump);

  // Base properties.
  std::string const& get_address(int index) const;
  std::vector<std::string> const& get_addresses() const;
  void set_addresses(std::vector<std::string> const& addresses);
  std::string const& get_alias() const;
  void set_alias(std::string const& alias);
  bool get_can_submit_commands() const;
  void set_can_submit_commands(bool can_submit);
  std::string const& get_email() const;
  void set_email(std::string const& email);
  uint32_t get_modified_attributes() const;
  void set_modified_attributes(uint32_t attr);
  void add_modified_attributes(uint32_t attr);
  std::string const& get_name() const;
  void set_name(std::string const& name);
  std::string const& get_pager() const;
  void set_pager(std::string const& pager);
  bool get_retain_status_information() const;
  void set_retain_status_information(bool retain);
  bool get_retain_nonstatus_information() const;
  void set_retain_nonstatus_information(bool retain);
  std::string const& get_timezone() const;
  void set_timezone(std::string const& timezone);
  bool notify_on(notifier::notifier_type type,
                 notifier::notification_flag notif) const;
  uint32_t notify_on(notifier::notifier_type type) const;
  void set_notify_on(notifier::notifier_type type, uint32_t notif);
  void add_notify_on(notifier::notifier_type type,
                     notifier::notification_flag notif);
  void remove_notify_on(notifier::notifier_type type,
                        notifier::notification_flag notif);
  uint32_t notify_on(notifier::notifier_type type,
                     notifier::notifier_type notif) const;
  std::string const& get_host_notification_period() const;
  void set_host_notification_period(std::string const& period);
  std::string const& get_service_notification_period() const;
  void set_service_notification_period(std::string const& period);
  bool get_host_notifications_enabled() const;
  void set_host_notifications_enabled(bool enabled);
  bool get_service_notifications_enabled() const;
  void set_service_notifications_enabled(bool enabled);

  // Host notification properties.
  time_t get_last_host_notification() const;
  void set_last_host_notification(time_t t);
  unsigned long get_modified_host_attributes() const;
  void set_modified_host_attributes(unsigned long attr);

  // Service notification properties.
  time_t get_last_service_notification() const;
  void set_last_service_notification(time_t t);
  unsigned long get_modified_service_attributes() const;
  void set_modified_service_attributes(unsigned long attr);
  std::list<std::shared_ptr<commands::command>> const&
  get_host_notification_commands() const;
  std::list<std::shared_ptr<commands::command>>&
  get_host_notification_commands();
  std::list<std::shared_ptr<commands::command>> const&
  get_service_notification_commands() const;
  std::list<std::shared_ptr<commands::command>>&
  get_service_notification_commands();
  contactgroup_map_unsafe const& get_parent_groups() const;
  contactgroup_map_unsafe& get_parent_groups();
  bool should_be_notified(notifier::notification_category cat,
                          notifier::reason_type type,
                          notifier const& notif) const;
  void resolve(int& w, int& e);
  map_customvar const& get_custom_variables() const;
  map_customvar& get_custom_variables();
  timeperiod* get_host_notification_period_ptr() const;
  void set_host_notification_period_ptr(timeperiod* period);
  timeperiod* get_service_notification_period_ptr() const;
  void set_service_notification_period_ptr(timeperiod* period);

  static contact_map contacts;

 private:
  static std::array<to_notify, 6> const _to_notify;

  bool _to_notify_normal(notifier::reason_type type,
                         notifier const& notif) const;
  bool _to_notify_recovery(notifier::reason_type type,
                           notifier const& notif) const;
  bool _to_notify_acknowledgement(notifier::reason_type type,
                                  notifier const& notif) const;
  bool _to_notify_flapping(notifier::reason_type type,
                           notifier const& notif) const;
  bool _to_notify_downtime(notifier::reason_type type,
                           notifier const& notif) const;
  bool _to_notify_custom(notifier::reason_type type,
                         notifier const& notif) const;

  std::vector<std::string> _addresses;
  std::string _alias;
  bool _can_submit_commands;
  std::string _email;
  std::string _name;
  time_t _last_host_notification;
  time_t _last_service_notification;
  unsigned long _modified_attributes;
  unsigned long _modified_host_attributes;
  unsigned long _modified_service_attributes;
  std::string _pager;
  bool _retain_status_information;
  bool _retain_nonstatus_information;
  std::array<uint32_t, 2> _notify_on;
  std::string _host_notification_period;
  std::string _service_notification_period;
  bool _host_notifications_enabled;
  bool _service_notifications_enabled;
  std::string _timezone;
  std::list<std::shared_ptr<commands::command>> _host_notification_commands;
  std::list<std::shared_ptr<commands::command>> _service_notification_commands;
  contactgroup_map_unsafe _contactgroups;
  map_customvar _custom_variables;
  timeperiod* _host_notification_period_ptr;
  timeperiod* _service_notification_period_ptr;
};

CCE_END()

std::shared_ptr<com::centreon::engine::contact> add_contact(
    std::string const& name,
    std::string const& alias,
    std::string const& email,
    std::string const& pager,
    std::array<std::string, MAX_CONTACT_ADDRESSES> const& addresses,
    std::string const& svc_notification_period,
    std::string const& host_notification_period,
    int notify_service_ok,
    int notify_service_critical,
    int notify_service_warning,
    int notify_service_unknown,
    int notify_service_flapping,
    int notify_service_downtime,
    int notify_host_up,
    int notify_host_down,
    int notify_host_unreachable,
    int notify_host_flapping,
    int notify_host_downtime,
    int host_notifications_enabled,
    int service_notifications_enabled,
    int can_submit_commands,
    int retain_status_information,
    int retain_nonstatus_information);

// bool          operator==(
//                com::centreon::engine::contact const& obj1,
//                com::centreon::engine::contact const& obj2) throw ();
// bool          operator!=(
//                com::centreon::engine::contact const& obj1,
//                com::centreon::engine::contact const& obj2) throw ();
std::ostream& operator<<(std::ostream& os,
                         com::centreon::engine::contact const& obj);
std::ostream& operator<<(std::ostream& os, contact_map_unsafe const& obj);

#endif  // !CCE_CONTACT_HH
