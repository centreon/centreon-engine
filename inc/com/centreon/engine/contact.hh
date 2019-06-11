/*
** Copyright 2017-2019 Centreon
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

#ifndef CCE_CONTACT_HH
#  define CCE_CONTACT_HH

#include <list>
#include <memory>
#include <time.h>
#include <vector>
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/customvariable.hh"
#include "com/centreon/engine/notifier.hh"

/* Max number of custom addresses a contact can have. */
#  define MAX_CONTACT_ADDRESSES 6

/* Forward declaration. */
typedef std::unordered_map<std::string,
  std::shared_ptr<com::centreon::engine::contact>> contact_map;

typedef struct notify_list_struct notification;

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
class                           contact {
 public:
                                contact();
                                ~contact();
  void                          update_status_info(bool aggregated_dump);

  // Base properties.
  std::string const&            get_address(int index) const;
  std::vector<std::string> const&
                                get_addresses() const;
  void                          set_addresses(
                                  std::vector<std::string> const& addresses);
  std::string const&            get_alias() const;
  void                          set_alias(std::string const& alias);
  bool                          get_can_submit_commands() const;
  void                          set_can_submit_commands(bool can_submit);
  std::string const&            get_email() const;
  void                          set_email(std::string const& email);
  uint32_t                      get_modified_attributes() const;
  void                          set_modified_attributes(uint32_t attr);
  void                          add_modified_attributes(uint32_t attr);
  std::string const&            get_name() const;
  void                          set_name(std::string const& name);
  std::string const&            get_pager() const;
  void                          set_pager(std::string const& pager);
  bool                          get_retain_status_information() const;
  void                          set_retain_status_information(bool retain);
  bool                          get_retain_nonstatus_information() const;
  void                          set_retain_nonstatus_information(bool retain);
  std::string const&            get_timezone() const;
  void                          set_timezone(std::string const& timezone);
  bool                          notify_on_service(notifier::notification_type type) const;
  void                          set_notify_on_service(uint32_t notif);
  void                          add_notify_on_service(notifier::notification_type type);
  void                          remove_notify_on_service(notifier::notification_type type);
  bool                          notify_on_host(notifier::notification_type type) const;
  void                          set_notify_on_host(uint32_t notif);
  void                          add_notify_on_host(notifier::notification_type type);
  void                          remove_notify_on_host(notifier::notification_type type);
  std::string const&            get_host_notification_period() const;
  void                          set_host_notification_period(std::string const& period);
  std::string const&            get_service_notification_period() const;
  void                          set_service_notification_period(std::string const& period);
  bool                          get_host_notifications_enabled() const;
  void                          set_host_notifications_enabled(bool enabled);
  bool                          get_service_notifications_enabled() const;
  void                          set_service_notifications_enabled(bool enabled);
  notification*                 find_notification();

  // Host notification properties.
  time_t                        get_last_host_notification() const;
  void                          set_last_host_notification(time_t t);
  unsigned long                 get_modified_host_attributes() const;
  void                          set_modified_host_attributes(
                                  unsigned long attr);

  // Service notification properties.
  time_t                        get_last_service_notification() const;
  void                          set_last_service_notification(time_t t);
  unsigned long                 get_modified_service_attributes() const;
  void                          set_modified_service_attributes(
                                  unsigned long attr);
  std::list<std::shared_ptr<commands::command>> const&
                                get_host_notification_commands() const;
  std::list<std::shared_ptr<commands::command>>&
                                get_host_notification_commands();
  std::list<std::shared_ptr<commands::command>> const&
                                get_service_notification_commands() const;
  std::list<std::shared_ptr<commands::command>>&
                                get_service_notification_commands();
  std::list<std::shared_ptr<contactgroup>> const&
                                get_parent_groups() const;
  std::list<std::shared_ptr<contactgroup>>&
                                get_parent_groups();
  int                           check_service_notification_viability(
                                  service* svc,
                                  unsigned int type,
                                  int options);
  int                           check_host_notification_viability(
                                  host* hst,
                                  unsigned int type,
                                  int options);

  static contact_map            contacts;

 private:
                                contact(contact const& other);
  contact&                      operator=(contact const& other);

  std::vector<std::string>      _addresses;
  std::string                   _alias;
  bool                          _can_submit_commands;
  std::string                   _email;
  std::string                   _name;
  time_t                        _last_host_notification;
  time_t                        _last_service_notification;
  unsigned long                 _modified_attributes;
  unsigned long                 _modified_host_attributes;
  unsigned long                 _modified_service_attributes;
  std::string                   _pager;
  bool                          _retain_status_information;
  bool                          _retain_nonstatus_information;
  uint32_t                      _notify_on_service;
  uint32_t                      _notify_on_host;
  std::string                   _host_notification_period;
  std::string                   _service_notification_period;
  bool                          _host_notifications_enabled;
  bool                          _service_notifications_enabled;
  std::string                   _timezone;
  std::list<std::shared_ptr<commands::command>>
                                _host_notification_commands;
  std::list<std::shared_ptr<commands::command>>
                                _service_notification_commands;
  std::list<std::shared_ptr<contactgroup>>
                                _contactgroups;

 public:
  std::unordered_map<std::string, customvariable>
                                custom_variables;

  timeperiod*                   host_notification_period_ptr;
  timeperiod*                   service_notification_period_ptr;
};

CCE_END()

typedef std::unordered_map<std::string,
                           std::shared_ptr<com::centreon::engine::contact>>
                      contact_map;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

com::centreon::engine::contact* add_contact(
           std::string const& name,
           char const* alias,
           char const* email,
           char const* pager,
           char const* const* addresses,
           char const* svc_notification_period,
           char const* host_notification_period,
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

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                com::centreon::engine::contact const& obj1,
                com::centreon::engine::contact const& obj2) throw ();
bool          operator!=(
                com::centreon::engine::contact const& obj1,
                com::centreon::engine::contact const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, com::centreon::engine::contact const& obj);
std::ostream& operator<<(
                std::ostream& os,
                contact_map const& obj);

#  endif /* C++ */

#endif // !CCE_CONTACT_HH
