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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

/**************************************
*                                     *
*           Base properties           *
*                                     *
**************************************/

/**
 *  Get a single address.
 *
 *  @param[in] index  Address index (starting from 0).
 *
 *  @return The requested address.
 */
std::string const& contact::get_address(int index) const {
  return _addresses[index];
}

/**
 *  Get all addresses.
 *
 *  @return Array of addresses.
 */
std::vector<std::string> const& contact::get_addresses() const {
  return _addresses;
}

/**
 *  Set addresses.
 *
 *  @param[in] addresses  New addresses.
 */
void contact::set_addresses(std::vector<std::string> const& addresses) {
  _addresses = addresses;
}

/**
 *  Return the contact alias
 *
 *  @return a reference to the alias
 */
std::string const& contact::get_alias() const {
  return _alias;
}

/**
 *  Set alias
 *
 *  @param[in] alias  New alias.
 */
void contact::set_alias(std::string const& alias) {
  _alias = alias;
}

/**
 *  Check if contact can submit commands.
 *
 *  @return True if contact can submit commands.
 */
bool contact::get_can_submit_commands() const {
  return _can_submit_commands;
}

/**
 *  (Dis)Allow a contact to submit commands.
 *
 *  @param[in] can_submit  True to enable contact to send commands.
 */
void contact::set_can_submit_commands(bool can_submit) {
  _can_submit_commands = can_submit;
}

/**
 *  Return the contact email
 *
 *  @return a reference to the email
 */
std::string const& contact::get_email() const {
  return _email;
}

/**
 *  Set contact email.
 *
 *  @param[in] email  New email.
 */
void contact::set_email(std::string const& email) {
  _email = email;
}

/**
 *  Get the contact's modified attributes.
 *
 *  @return A bitmask, representing modified attributes.
 */
unsigned long contact::get_modified_attributes() const {
  return _modified_attributes;
}

/**
 *  Set the contact's modified attributes.
 *
 *  @param[in] attr  Modified attributes.
 */
void contact::set_modified_attributes(unsigned long attr) {
  _modified_attributes = attr;
}

/**
 *  Return the contact name.
 *
 *  @return A reference to the name.
 */
std::string const& contact::get_name() const {
  return _name;
}

/**
 *  Set the contact name.
 *
 *  @param[in] name  New name.
 */
void contact::set_name(std::string const& name) {
  _name = name;
}

/**
 *  Return the contact pager
 *
 *  @return a reference to the pager
 */
std::string const& contact::get_pager() const {
  return _pager;
}

/**
 *  Set the pager.
 *
 *  @param[in] pager  New pager.
 */
void contact::set_pager(std::string const& pager) {
  _pager = pager;
}

/**
 *  Check if status info should be retained.
 *
 *  @return True if status info should be retained.
 */
bool contact::get_retain_status_information() const {
  return _retain_status_information;
}

/**
 *  Retain (or not) status info.
 *
 *  @param[in] retain  True to retain status info.
 */
void contact::set_retain_status_information(bool retain) {
  _retain_status_information = retain;
}

/**
 *  Check if non-status info should be retained.
 *
 *  @return True if non-status info should be retained.
 */
bool contact::get_retain_nonstatus_information() const {
  return _retain_nonstatus_information;
}

/**
 *  Retain (or not) non-status info.
 *
 *  @param[in] retain  True to retain non-status info.
 */
void contact::set_retain_nonstatus_information(bool retain) {
  _retain_nonstatus_information = retain;
}

/**
 *  Get timezone.
 *
 *  @return Contact timezone.
 */
std::string const& contact::get_timezone() const {
  return _timezone;
}

/**
 *  Set timezone.
 *
 *  @param[in] timezone  New contact timezone.
 */
void contact::set_timezone(std::string const& timezone) {
  _timezone = timezone;
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
       contact const& obj1,
       contact const& obj2) throw () {
  return obj1.get_name() == obj2.get_name()
         && obj1.get_alias() == obj2.get_alias()
         && obj1.get_email() == obj2.get_email()
         && obj1.get_pager() == obj2.get_pager()
         && obj1.get_addresses() == obj2.get_addresses()
         && obj1.get_host_notification_commands() == obj2.get_host_notification_commands()
         && obj1.get_service_notification_commands() == obj2.get_service_notification_commands()
         && obj1.notify_on_service_unknown() == obj2.notify_on_service_unknown()
         && obj1.notify_on_service_warning() == obj2.notify_on_service_warning()
         && obj1.notify_on_service_critical() == obj2.notify_on_service_critical()
         && obj1.notify_on_service_recovery() == obj2.notify_on_service_recovery()
         && obj1.notify_on_service_flapping() == obj2.notify_on_service_flapping()
         && obj1.notify_on_service_downtime() == obj2.notify_on_service_downtime()
         && obj1.notify_on_host_down() == obj2.notify_on_host_down()
         && obj1.notify_on_host_unreachable() == obj2.notify_on_host_unreachable()
         && obj1.notify_on_host_recovery() == obj2.notify_on_host_recovery()
         && obj1.notify_on_host_flapping() == obj2.notify_on_host_flapping()
         && obj1.notify_on_host_downtime() == obj2.notify_on_host_downtime()
         && obj1.get_host_notification_period() == obj2.get_host_notification_period()
         && obj1.get_service_notification_period() == obj2.get_service_notification_period()
         && obj1.get_host_notifications_enabled() == obj2.get_host_notifications_enabled()
         && obj1.get_service_notifications_enabled() == obj2.get_service_notifications_enabled()
         && obj1.get_can_submit_commands() == obj2.get_can_submit_commands()
         && obj1.get_retain_status_information() == obj2.get_retain_status_information()
         && obj1.get_retain_nonstatus_information() == obj2.get_retain_nonstatus_information()
         && obj1.custom_variables == obj2.custom_variables
         && obj1.get_last_host_notification() == obj2.get_last_host_notification()
         && obj1.get_last_service_notification() == obj2.get_last_service_notification()
         && obj1.get_modified_attributes() == obj2.get_modified_attributes()
         && obj1.get_modified_host_attributes() == obj2.get_modified_host_attributes()
         && obj1.get_modified_service_attributes() == obj2.get_modified_service_attributes();
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
       contact const& obj1,
       contact const& obj2) throw () {
  return !operator==(obj1, obj2);
}

/**
 *  Dump contact content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The contact to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, contact const& obj) {
  std::string cg_name(static_cast<contactgroup const*>(
      obj.contactgroups_ptr->object_ptr)->get_name());
  char const* hst_notif_str(NULL);
  if (obj.host_notification_period_ptr)
    hst_notif_str = chkstr(obj.host_notification_period_ptr->name);
  char const* svc_notif_str(NULL);
  if (obj.service_notification_period_ptr)
    svc_notif_str = chkstr(obj.service_notification_period_ptr->name);

  os << "contact {\n"
    "  name:                            " << obj.get_name() << "\n"
    "  alias:                           " << obj.get_alias() << "\n"
    "  email:                           " << obj.get_email() << "\n"
    "  pager:                           " << obj.get_pager() << "\n"
    "  address:                         ";
  std::vector<std::string> const& address(obj.get_addresses());
  for (unsigned int i(0); i < address.size(); ++i)
    os << address[i]
       << (i + 1 < address.size() && !address[i + 1].empty() ? ", " : "");
  os << (!address[0].empty() ? " \n" : "\"NULL\"\n");
  os << "  host_notification_commands:      ";
  for (std::shared_ptr<commands::command> const& cmd : obj.get_host_notification_commands())
    os << cmd->get_command_line() << " ; ";
  os << "\n  service_notification_commands:   ";
  for (std::shared_ptr<commands::command> const& cmd : obj.get_service_notification_commands())
    os << cmd->get_command_line() << " ; ";
  os << "\n"
    "  notify_on_service_unknown:       " << obj.notify_on_service_unknown() << "\n"
    "  notify_on_service_warning:       " << obj.notify_on_service_warning() << "\n"
    "  notify_on_service_critical:      " << obj.notify_on_service_critical() << "\n"
    "  notify_on_service_recovery:      " << obj.notify_on_service_recovery() << "\n"
    "  notify_on_service_flapping:      " << obj.notify_on_service_flapping() << "\n"
    "  notify_on_service_downtime:      " << obj.notify_on_service_downtime() << "\n"
    "  notify_on_host_down:             " << obj.notify_on_host_down() << "\n"
    "  notify_on_host_unreachable:      " << obj.notify_on_host_unreachable() << "\n"
    "  notify_on_host_recovery:         " << obj.notify_on_host_recovery() << "\n"
    "  notify_on_host_flapping:         " << obj.notify_on_host_flapping() << "\n"
    "  notify_on_host_downtime:         " << obj.notify_on_host_downtime() << "\n"
    "  host_notification_period:        " << obj.get_host_notification_period() << "\n"
    "  service_notification_period:     " << obj.get_service_notification_period() << "\n"
    "  host_notifications_enabled:      " << obj.get_host_notifications_enabled() << "\n"
    "  service_notifications_enabled:   " << obj.get_service_notifications_enabled() << "\n"
    "  can_submit_commands:             " << obj.get_can_submit_commands() << "\n"
    "  retain_status_information:       " << obj.get_retain_status_information() << "\n"
    "  retain_nonstatus_information:    " << obj.get_retain_nonstatus_information() << "\n"
    "  last_host_notification:          " << string::ctime(obj.get_last_host_notification()) << "\n"
    "  last_service_notification:       " << string::ctime(obj.get_last_service_notification()) << "\n"
    "  modified_attributes:             " << obj.get_modified_attributes() << "\n"
    "  modified_host_attributes:        " << obj.get_modified_host_attributes() << "\n"
    "  modified_service_attributes:     " << obj.get_modified_service_attributes() << "\n"
    "  host_notification_period_ptr:    " << chkstr(hst_notif_str) << "\n"
    "  service_notification_period_ptr: " << chkstr(svc_notif_str) << "\n"
    "  contactgroups_ptr:               " << cg_name << "\n"
    "  customvariables:                 ";
  for (std::pair<std::string, customvariable> const& cv : obj.custom_variables)
    os << cv.first << " ; ";
  os << "}\n";
  return os;
}

/**
 *  Add a new contact to the list in memory.
 *
 *  @param[in] name                          Contact name.
 *  @param[in] alias                         Contact alias.
 *  @param[in] email                         Email.
 *  @param[in] pager                         Pager.
 *  @param[in] addresses                     Contact addresses.
 *  @param[in] svc_notification_period       Service notification
 *                                           period.
 *  @param[in] host_notification_period      Host nofication period.
 *  @param[in] notify_service_ok             Contact can be notified
 *                                           when service is ok.
 *  @param[in] notify_service_critical       Contact can be notified
 *                                           when service is critical.
 *  @param[in] notify_service_warning        Contact can be notified
 *                                           when service is warning.
 *  @param[in] notify_service_unknown        Contact can be notified
 *                                           when service is unknown.
 *  @param[in] notify_service_flapping       Contact can be notified
 *                                           when service is flapping.
 *  @param[in] notify_sevice_downtime        Contact can be notified on
 *                                           service downtime.
 *  @param[in] notify_host_up                Contact can be notified
 *                                           when host is up.
 *  @param[in] notify_host_down              Contact can be notified
 *                                           when host is down.
 *  @param[in] notify_host_unreachable       Contact can be notified
 *                                           when host is unreachable.
 *  @param[in] notify_host_flapping          Contact can be notified
 *                                           when host is flapping.
 *  @param[in] notify_host_downtime          Contact can be notified on
 *                                           host downtime.
 *  @param[in] host_notifications_enabled    Are contact host
 *                                           notifications enabled ?
 *  @param[in] service_notifications_enabled Are contact service
 *                                           notifications enabled ?
 *  @param[in] can_submit_commands           Can user submit external
 *                                           commands ?
 *  @param[in] retain_status_information     Shall Engine retain contact
 *                                           status info ?
 *  @param[in] retain_nonstatus_information  Shell Engine retain contact
 *                                           non-status info ?
 *
 *  @return New contact object.
 */
contact* add_contact(
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
           int retain_nonstatus_information) {
  // Make sure we have the data we need.
  if (name.empty()) {
    logger(log_config_error, basic)
      << "Error: Contact name is empty";
    return NULL;
  }

  // Check if the contact already exist.
  std::string id(name);
  if (configuration::applier::state::instance().contacts().count(id)) {
    logger(log_config_error, basic)
      << "Error: Contact '" << name << "' has already been defined";
    return NULL;
  }

  // Allocate memory for a new contact.
  std::shared_ptr<contact> obj(new contact);

  try {
    // Duplicate vars.
    obj->set_name(name);
    obj->set_alias(!alias ? name : alias);
    if (email)
      obj->set_email(email);
    if (host_notification_period)
      obj->set_host_notification_period(host_notification_period);
    if (pager)
      obj->set_pager(pager);
    if (svc_notification_period)
      obj->set_service_notification_period(svc_notification_period);
    if (addresses) {
      std::vector<std::string> addr;
      for (unsigned int x(0); x < MAX_CONTACT_ADDRESSES; ++x)
        if (addresses[x])
          addr[x] = string::dup(addresses[x]);
      obj->set_addresses(addr);
    }

    // Set remaining contact properties.
    obj->set_can_submit_commands(can_submit_commands > 0);
    obj->set_host_notifications_enabled(host_notifications_enabled > 0);
    obj->set_modified_attributes(MODATTR_NONE);
    obj->set_modified_host_attributes(MODATTR_NONE);
    obj->set_modified_service_attributes(MODATTR_NONE);
    obj->set_notify_on_host_down(notify_host_down > 0);
    obj->set_notify_on_host_downtime(notify_host_downtime > 0);
    obj->set_notify_on_host_flapping(notify_host_flapping > 0);
    obj->set_notify_on_host_recovery(notify_host_up > 0);
    obj->set_notify_on_host_unreachable(notify_host_unreachable > 0);
    obj->set_notify_on_service_critical(notify_service_critical > 0);
    obj->set_notify_on_service_downtime(notify_service_downtime > 0);
    obj->set_notify_on_service_flapping(notify_service_flapping > 0);
    obj->set_notify_on_service_recovery(notify_service_ok > 0);
    obj->set_notify_on_service_unknown(notify_service_unknown > 0);
    obj->set_notify_on_service_warning(notify_service_warning > 0);
    obj->set_retain_nonstatus_information(retain_nonstatus_information > 0);
    obj->set_retain_status_information(retain_status_information > 0);
    obj->set_service_notifications_enabled(service_notifications_enabled > 0);

    // Add new items to the configuration state.
    state::instance().contacts()[id] = obj;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_contact_data(
      NEBTYPE_CONTACT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);
  }
  catch (...) {
    obj.reset();
  }

  return obj.get();
}

contact::contact()
 : _addresses(MAX_CONTACT_ADDRESSES) {}

contact::~contact() {
  deleter::listmember(this->contactgroups_ptr, &deleter::objectlist);

  // host_notification_period_ptr not free.
  // service_notification_period_ptr not free.

}

bool contact::notify_on_service_unknown() const {
  return _notify_on_service_unknown;
}

void contact::set_notify_on_service_unknown(bool notify) {
  _notify_on_service_unknown = notify;
}

bool contact::notify_on_service_warning() const {
  return _notify_on_service_warning;
}

void contact::set_notify_on_service_warning(bool notify) {
  _notify_on_service_warning = notify;
}

bool contact::notify_on_service_critical() const {
  return _notify_on_service_critical;
}

void contact::set_notify_on_service_critical(bool notify) {
  _notify_on_service_critical = notify;
}

bool contact::notify_on_service_recovery() const {
  return _notify_on_service_recovery;
}

void contact::set_notify_on_service_recovery(bool notify) {
  _notify_on_service_recovery = notify;
}

bool contact::notify_on_service_flapping() const {
  return _notify_on_service_flapping;
}

void contact::set_notify_on_service_flapping(bool notify) {
  _notify_on_service_flapping = notify;
}

bool contact::notify_on_service_downtime() const {
  return _notify_on_service_downtime;
}

void contact::set_notify_on_service_downtime(bool notify) {
  _notify_on_service_downtime = notify;
}

bool contact::notify_on_host_down() const {
  return _notify_on_host_down;
}

void contact::set_notify_on_host_down(bool notify) {
  _notify_on_host_down = notify;
}

bool contact::notify_on_host_unreachable() const {
  return _notify_on_host_unreachable;
}

void contact::set_notify_on_host_unreachable(bool notify) {
  _notify_on_host_unreachable = notify;
}

bool contact::notify_on_host_recovery() const {
  return _notify_on_host_recovery;
}

void contact::set_notify_on_host_recovery(bool notify) {
  _notify_on_host_recovery = notify;
}

bool contact::notify_on_host_flapping() const {
  return _notify_on_host_flapping;
}

void contact::set_notify_on_host_flapping(bool notify) {
  _notify_on_host_flapping = notify;
}

bool contact::notify_on_host_downtime() const {
  return _notify_on_host_downtime;
}

void contact::set_notify_on_host_downtime(bool notify) {
  _notify_on_host_downtime = notify;
}

/**
 *  Get the host notification period.
 *
 *  @return A pointer to the host notification period.
 */
std::string const& contact::get_host_notification_period() const {
  return _host_notification_period;
}

/**
 *  Set the host notification period.
 *
 *  @param[in] tp  Pointer to the new host notification period.
 */
void contact::set_host_notification_period(std::string const& period) {
  _host_notification_period = period;
}

/**
 *  Get the last time a host notification was sent for this contact.
 *
 *  @return A timestamp.
 */
time_t contact::get_last_host_notification() const {
  return _last_host_notification;
}

/**
 *  Set the last time a host notification was sent.
 *
 *  @param[in] t  Timestamp.
 */
void contact::set_last_host_notification(time_t t) {
  _last_host_notification = t;
}

/**
 *  Get the modified host attributes.
 *
 *  @return A bitmask.
 */
unsigned long contact::get_modified_host_attributes() const {
  return _modified_host_attributes;
}

/**
 *  Set the modified host attributes.
 *
 *  @param[in] attr  Modified host attributes.
 */
void contact::set_modified_host_attributes(unsigned long attr) {
  _modified_host_attributes = attr;
}

/**
 *  Get the service notification period.
 *
 *  @return Pointer to the notification period.
 */
std::string const& contact::get_service_notification_period() const {
  return _service_notification_period;
}

/**
 *  Set service notification period.
 *
 *  @param[in] tp  Pointer to the new service notification period.
 */
void contact::set_service_notification_period(std::string const& period) {
  _service_notification_period = period;
}

/**
 *  Get the last time a service notification was sent.
 *
 *  @return Timestamp.
 */
time_t contact::get_last_service_notification() const {
  return _last_service_notification;
}

/**
 *  Set the last time a service notification was sent.
 *
 *  @param[in] t  Timestamp.
 */
void contact::set_last_service_notification(time_t t) {
  _last_service_notification = t;
}

/**
 *  Get modified service attributes.
 *
 *  @return A bitmask.
 */
unsigned long contact::get_modified_service_attributes() const {
  return _modified_service_attributes;
}

/**
 *  Set the service modified attributes.
 *
 *  @param[in] attr  Service modified attributes.
 */
void contact::set_modified_service_attributes(unsigned long attr) {
  _modified_service_attributes = attr;
}

bool contact::get_host_notifications_enabled() const {
  return _host_notifications_enabled;
}

void contact::set_host_notifications_enabled(bool enabled) {
  _host_notifications_enabled = enabled;
}

bool contact::get_service_notifications_enabled() const {
  return _service_notifications_enabled;
}

void contact::set_service_notifications_enabled(bool enabled) {
  _service_notifications_enabled = enabled;
}

/**
 *  Updates contact status info.
 *
 *  @param aggregated_dump
 *
 */
void contact::update_status_info(bool aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (!aggregated_dump)
    broker_contact_status(
      NEBTYPE_CONTACTSTATUS_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      this,
      NULL);
}

std::list<std::shared_ptr<commands::command>> const&
contact::get_host_notification_commands() const {
  return _host_notification_commands;
}

std::list<std::shared_ptr<commands::command>>&
contact::get_host_notification_commands() {
  return _host_notification_commands;
}

std::list<std::shared_ptr<commands::command>> const&
contact::get_service_notification_commands() const {
  return _service_notification_commands;
}

std::list<std::shared_ptr<commands::command>>&
contact::get_service_notification_commands() {
  return _service_notification_commands;
}

