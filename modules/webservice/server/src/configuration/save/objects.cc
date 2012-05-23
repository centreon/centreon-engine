/*
** Copyright 2012 Merethis
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

#include <fstream>
#include <iomanip>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/webservice/configuration/save/objects.hh"

using namespace com::centreon::engine::modules::webservice::configuration::save;

/**
 *  Default constructor.
 */
objects::objects() {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right  The object to copy.
 */
objects::objects(objects const& right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
objects::~objects() throw () {

}

/**
 *  Assignment operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
objects& objects::operator=(objects const& right) {
  return (_internal_copy(right));
}

/**
 *  Translate a command to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(command const& obj) {
  _stream << "define command{\n";
  _add_string("name", obj.name);
  _add_string("command_line", obj.command_line);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a contact to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(contact const& obj) {
  std::string host_notification_options;
  if (obj.notify_on_host_down) host_notification_options += ",d";
  if (obj.notify_on_host_unreachable) host_notification_options += ",u";
  if (obj.notify_on_host_recovery) host_notification_options += ",r";
  if (obj.notify_on_host_flapping) host_notification_options += ",f";
  if (obj.notify_on_host_downtime) host_notification_options += ",s";
  if (host_notification_options.empty())
    host_notification_options = "n";
  else
    host_notification_options.erase(0, 1);

  std::string service_notification_options;
  if (obj.notify_on_service_warning) service_notification_options += ",w";
  if (obj.notify_on_service_unknown) service_notification_options += ",u";
  if (obj.notify_on_service_critical) service_notification_options += ",c";
  if (obj.notify_on_service_recovery) service_notification_options += ",r";
  if (obj.notify_on_service_flapping) service_notification_options += ",f";
  if (obj.notify_on_service_downtime) service_notification_options += ",s";
  if (service_notification_options.empty())
    service_notification_options = "n";
  else
    service_notification_options.erase(0, 1);

  _stream << "define contact{\n";
  _add_string("contact_name", obj.name);
  _add_string("alias", obj.alias);
  _add_line("host_notifications_enabled", obj.host_notifications_enabled);
  _add_line("service_notifications_enabled", obj.service_notifications_enabled);
  _add_string("host_notification_period", obj.host_notification_period);
  _add_string("service_notification_period", obj.service_notification_period);
  _add_string("host_notification_options", host_notification_options);
  _add_string("service_notification_options", service_notification_options);
  _add_commands("host_notification_commands", obj.host_notification_commands);
  _add_commands("service_notification_commands", obj.service_notification_commands);
  _add_string("email", obj.email);
  _add_string("pager", (char const*)obj.pager);
  for (unsigned int i(0); i < MAX_CONTACT_ADDRESSES; ++i)
    if (obj.address[i])
      _stream << "  address" << (i + 1) << " " << obj.address[i] << "\n";
  _add_line("can_submit_commands", obj.can_submit_commands);
  _add_line("retain_status_information", obj.retain_status_information);
  _add_line("retain_nonstatus_information", obj.retain_nonstatus_information);
  _add_customvariables(obj.custom_variables);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a contactgroup to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(contactgroup const& obj) {
  _stream << "define contactgroup{\n";
  _add_string("contactgroup_name", obj.group_name);
  _add_string("alias", obj.alias);
  _add_contacts("members", obj.members);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate an host to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(host const& obj) {
  std::string initial_state("o");
  if (obj.initial_state == HOST_DOWN) initial_state = "d";
  else if (obj.initial_state == HOST_UNREACHABLE) initial_state = "u";

  std::string flap_detection_options;
  if (obj.flap_detection_on_up) flap_detection_options += ",o";
  if (obj.flap_detection_on_down) flap_detection_options += ",d";
  if (obj.flap_detection_on_unreachable) flap_detection_options += ",u";
  if (!flap_detection_options.empty()) flap_detection_options.erase(0, 1);

  std::string notification_options;
  if (obj.notify_on_down) notification_options += ",d";
  if (obj.notify_on_unreachable) notification_options += ",u";
  if (obj.notify_on_recovery) notification_options += ",r";
  if (obj.notify_on_flapping) notification_options += ",f";
  if (obj.notify_on_downtime) notification_options += ",s";
  if (!notification_options.empty()) notification_options.erase(0, 1);

  std::string stalking_options;
  if (obj.stalk_on_up) stalking_options += ",o";
  if (obj.stalk_on_down) stalking_options += ",d";
  if (obj.stalk_on_unreachable) stalking_options += ",u";
  if (!stalking_options.empty()) stalking_options.erase(0, 1);

  _stream << "define host{\n";
  _add_string("host_name", obj.name);
  _add_string("alias", obj.alias);
  _add_string("display_name", obj.display_name);
  _add_string("address", obj.address);
  _add_hosts("parents", obj.parent_hosts);
  _add_string("check_command", obj.host_check_command);
  _add_string("initial_state", initial_state);
  _add_line("max_check_attempts", obj.max_attempts);
  _add_line("check_interval", obj.check_interval);
  _add_line("retry_interval", obj.retry_interval);
  _add_line("active_checks_enabled", obj.checks_enabled);
  _add_line("passive_checks_enabled", obj.accept_passive_host_checks);
  _add_string("check_period", obj.check_period);
  _add_line("obsess_over_host", obj.obsess_over_host);
  _add_line("check_freshness", obj.check_freshness);
  _add_line("freshness_threshold", obj.freshness_threshold);
  _add_string("event_handler", obj.event_handler);
  _add_line("event_handler_enabled", obj.event_handler_enabled);
  _add_line("low_flap_threshold", obj.low_flap_threshold);
  _add_line("high_flap_threshold", obj.high_flap_threshold);
  _add_line("flap_detection_enabled", obj.flap_detection_enabled);
  _add_string("flap_detection_options", flap_detection_options);
  _add_line("process_perf_data", obj.process_performance_data);
  _add_line("retain_status_information", obj.retain_status_information);
  _add_line("retain_nonstatus_information", obj.retain_nonstatus_information);
  _add_contacts("contacts", obj.contacts);
  _add_contactgroups("contact_groups", obj.contact_groups);
  _add_line("notification_interval", obj.notification_interval);
  _add_line("first_notification_delay", obj.first_notification_delay);
  _add_string("notification_period", obj.notification_period);
  _add_string("notification_options", notification_options);
  _add_line("notifications_enabled", obj.notifications_enabled);
  _add_string("stalking_options", stalking_options);
  _add_string("notes", obj.notes);
  _add_string("notes_url", obj.notes_url);
  _add_string("action_url", obj.action_url);
  _add_string("icon_image", obj.icon_image);
  _add_string("icon_image_alt", obj.icon_image_alt);
  _add_string("vrml_image", obj.vrml_image);
  _add_string("statusmap_image", obj.statusmap_image);
  if (obj.have_2d_coords)
    _stream << "  2d_coords " << obj.x_2d << "," << obj.y_2d << "\n";
  if (obj.have_3d_coords)
    _stream << "  3d_coords " << obj.x_3d << "," << obj.y_3d << "," << obj.z_3d << "\n";
  _add_customvariables(obj.custom_variables);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a hostdependency to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(hostdependency const& obj) {
  std::string failure_criteria;
  if (obj.fail_on_up) failure_criteria += ",o";
  if (obj.fail_on_down) failure_criteria += ",d";
  if (obj.fail_on_unreachable) failure_criteria += ",u";
  if (obj.fail_on_pending) failure_criteria += ",p";
  if (failure_criteria.empty())
    failure_criteria = "n";
  else
    failure_criteria.erase(0, 1);

  _stream << "define hostdependency{\n";
  _add_string("dependent_host_name", obj.dependent_host_name);
  _add_string("host_name", obj.host_name);
  _add_line("inherits_parent", obj.inherits_parent);
  if (obj.dependency_type == EXECUTION_DEPENDENCY)
    _add_string("execution_failure_criteria", failure_criteria);
  else
    _add_string("notification_failure_criteria", failure_criteria);
  _add_string("dependency_period", obj.dependency_period);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a hostescalation to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(hostescalation const& obj) {
  std::string escalation_options;
  if (obj.escalate_on_down) escalation_options += ",d";
  if (obj.escalate_on_unreachable) escalation_options += ",u";
  if (obj.escalate_on_recovery) escalation_options += ",r";
  if (!escalation_options.empty())
    escalation_options.erase(0, 1);

  _stream << "define hostescalation{\n";
  _add_string("host_name", obj.host_name);
  _add_contacts("contacts", obj.contacts);
  _add_contactgroups("contact_groups", obj.contact_groups);
  _add_line("first_notification", obj.first_notification);
  _add_line("last_notification", obj.last_notification);
  _add_line("notification_interval", obj.notification_interval);
  _add_string("escalation_period", obj.escalation_period);
  _add_string("escalation_options", escalation_options);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a hostgroup to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(hostgroup const& obj) {
  _stream << "define hostgroup{\n";
  _add_string("hostgroup_name", obj.group_name);
  _add_string("alias", obj.alias);
  _add_hosts("members", obj.members);
  _add_string("notes", obj.notes);
  _add_string("notes_url", obj.notes_url);
  _add_string("action_url", obj.action_url);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a service to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(service const& obj) {
  std::string initial_state("o");
  if (obj.initial_state == STATE_WARNING) initial_state = "w";
  else if (obj.initial_state == STATE_UNKNOWN) initial_state = "u";
  else if (obj.initial_state == STATE_CRITICAL) initial_state = "c";

  std::string flap_detection_options;
  if (obj.flap_detection_on_ok) flap_detection_options += ",o";
  if (obj.flap_detection_on_warning) flap_detection_options += ",w";
  if (obj.flap_detection_on_critical) flap_detection_options += ",c";
  if (obj.flap_detection_on_unknown) flap_detection_options += ",u";
  if (!flap_detection_options.empty()) flap_detection_options.erase(0, 1);

  std::string notification_options;
  if (obj.notify_on_warning) notification_options += ",w";
  if (obj.notify_on_unknown) notification_options += ",u";
  if (obj.notify_on_critical) notification_options += ",c";
  if (obj.notify_on_recovery) notification_options += ",r";
  if (obj.notify_on_flapping) notification_options += ",f";
  if (obj.notify_on_downtime) notification_options += ",s";
  if (!notification_options.empty()) notification_options.erase(0, 1);

  std::string stalking_options;
  if (obj.stalk_on_ok) stalking_options += ",o";
  if (obj.stalk_on_warning) stalking_options += ",w";
  if (obj.stalk_on_unknown) stalking_options += ",u";
  if (obj.stalk_on_critical) stalking_options += ",c";
  if (!stalking_options.empty()) stalking_options.erase(0, 1);

  _stream << "define service{\n";
  _add_string("host_name", obj.host_name);
  _add_string("service_description", obj.description);
  _add_string("display_name", obj.display_name);
  _add_line("is_volatile", obj.is_volatile);
  _add_string("check_command", obj.service_check_command);
  _add_string("initial_state", initial_state);
  _add_line("max_check_attempts", obj.max_attempts);
  _add_line("check_interval", obj.check_interval);
  _add_line("retry_interval", obj.retry_interval);
  _add_line("active_checks_enabled", obj.checks_enabled);
  _add_line("passive_checks_enabled", obj.accept_passive_service_checks);
  _add_string("check_period", obj.check_period);
  _add_line("obsess_over_service", obj.obsess_over_service);
  _add_line("check_freshness", obj.check_freshness);
  _add_line("freshness_threshold", obj.freshness_threshold);
  _add_string("event_handler", obj.event_handler);
  _add_line("event_handler_enabled", obj.event_handler_enabled);
  _add_line("low_flap_threshold", obj.low_flap_threshold);
  _add_line("high_flap_threshold", obj.high_flap_threshold);
  _add_line("flap_detection_enabled", obj.flap_detection_enabled);
  _add_string("flap_detection_options", flap_detection_options);
  _add_line("process_perf_data", obj.process_performance_data);
  _add_line("retain_status_information", obj.retain_status_information);
  _add_line("retain_nonstatus_information", obj.retain_nonstatus_information);
  _add_line("notification_interval", obj.notification_interval);
  _add_line("first_notification_delay", obj.first_notification_delay);
  _add_string("notification_period", obj.notification_period);
  _add_string("notification_options", notification_options);
  _add_line("notifications_enabled", obj.notifications_enabled);
  _add_contacts("contacts", obj.contacts);
  _add_contactgroups("contact_groups", obj.contact_groups);
  _add_string("stalking_options", stalking_options);
  _add_string("notes", obj.notes);
  _add_string("notes_url", obj.notes_url);
  _add_string("action_url", obj.action_url);
  _add_string("icon_image", obj.icon_image);
  _add_string("icon_image_alt", obj.icon_image_alt);
  _add_customvariables(obj.custom_variables);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a servicedependency to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(servicedependency const& obj) {
  std::string failure_criteria;
  if (obj.fail_on_ok) failure_criteria += ",o";
  if (obj.fail_on_warning) failure_criteria += ",w";
  if (obj.fail_on_unknown) failure_criteria += ",u";
  if (obj.fail_on_critical) failure_criteria += ",c";
  if (obj.fail_on_pending) failure_criteria += ",p";
  if (failure_criteria.empty())
    failure_criteria = "n";
  else
    failure_criteria.erase(0, 1);

  _stream << "define servicedependency{\n";
  _add_string("dependent_host_name", obj.dependent_host_name);
  _add_string("dependent_service_description", obj.dependent_service_description);
  _add_string("host_name", obj.host_name);
  _add_string("service_description", obj.service_description);
  _add_line("inherits_parent", obj.inherits_parent);
  if (obj.dependency_type == EXECUTION_DEPENDENCY)
    _add_string("execution_failure_criteria", failure_criteria);
  else
    _add_string("notification_failure_criteria", failure_criteria);
  _add_string("dependency_period", obj.dependency_period);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a serviceescalation to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(serviceescalation const& obj) {
  std::string escalation_options;
  if (obj.escalate_on_warning) escalation_options += ",w";
  if (obj.escalate_on_unknown) escalation_options += ",u";
  if (obj.escalate_on_critical) escalation_options += ",c";
  if (obj.escalate_on_recovery) escalation_options += ",r";
  if (!escalation_options.empty())
    escalation_options.erase(0, 1);

  _stream << "define serviceescalation{\n";
  _add_string("host_name", obj.host_name);
  _add_string("description", obj.description);
  _add_contacts("contacts", obj.contacts);
  _add_contactgroups("contact_groups", obj.contact_groups);
  _add_line("first_notification", obj.first_notification);
  _add_line("last_notification", obj.last_notification);
  _add_line("notification_interval", obj.notification_interval);
  _add_string("escalation_period", obj.escalation_period);
  _add_string("escalation_options", escalation_options);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a servicegroup to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(servicegroup const& obj) {
  _stream << "define servicegroup{\n";
  _add_string("servicegroup_name", obj.group_name);
  _add_string("alias", obj.alias);
  _add_services("members", obj.members);
  _add_string("notes", obj.notes);
  _add_string("notes_url", obj.notes_url);
  _add_string("action_url", obj.action_url);
  _stream << "}\n";
  return (*this);
}

/**
 *  Translate a timeperiod to the engine configuration format.
 *
 *  @param[in] obj  The object to save.
 */
objects& objects::operator<<(timeperiod const& obj) {
  _stream << "define timeperiod{\n";
  _add_string("timeperiod_name", obj.name);
  _add_string("alias", obj.alias);
  for (unsigned int i(0); i < 7; ++i)
    _add_timerange(_get_weekday(i), obj.days[i]);
  for (unsigned int i(0); i < DATERANGE_TYPES; ++i)
    for (daterange const* it(obj.exceptions[i]); it; it = it->next)
      _add_daterange(it);
  _add_timeperiodexclusion("exclude", obj.exclusions);
  _stream << "}\n";
  return (*this);
}

/**
 *  Save into the file the current configuration.
 *
 *  @param[in] filename  The file path to save buffer.
 */
void objects::backup(std::string const& filename) const {
  std::ofstream output(filename.c_str());
  if (!output.is_open())
    throw (engine_error() << "save configuration: open file '"
           << filename << "' failed");
  output << _stream.str();
}

/**
 *  Clear buffer.
 */
void objects::clear() {
  _stream.str("");
}

/**
 *  Get the save string.
 *
 *  @return The save string.
 */
std::string objects::to_string() const {
  return (_stream.str());
}


/**
 *  Extract commands member.
 *
 *  @param[in] key  The key name.
 *  @param[in] obj  The object to write.
 */
void objects::_add_commands(char const* key, commandsmember const* obj) {
  if (!obj)
    return;
  _stream << "  " << key << " ";
  for (commandsmember const* cm(obj); cm; cm = cm->next)
    _stream << cm->cmd << (cm->next ? "," : "\n");
}

/**
 *  Extract contacts member.
 *
 *  @param[in] key  The key name.
 *  @param[in] obj  The object to write.
 */
void objects::_add_contacts(char const* key, contactsmember const* obj) {
  if (!obj)
    return;
  _stream << "  " << key << " ";
  for (contactsmember const* cm(obj); cm; cm = cm->next)
    _stream << cm->contact_name << (cm->next ? "," : "\n");
}

/**
 *  Extract contact groups member.
 *
 *  @param[in] key  The key name.
 *  @param[in] obj  The object to write.
 */
void objects::_add_contactgroups(
             char const* key,
             contactgroupsmember const* obj) {
  if (!obj)
    return;
  _stream << "  " << key << " ";
  for (contactgroupsmember const* cgm(obj); cgm; cgm = cgm->next)
    _stream << cgm->group_name << (cgm->next ? "," : "\n");
}

/**
 *  Extract custom variables.
 *
 *  @param[in] obj  The object to write.
 */
void objects::_add_customvariables(customvariablesmember const* obj) {
  for (customvariablesmember const* cvm(obj); cvm; cvm = cvm->next)
    _add_string(cvm->variable_name, cvm->variable_value);
}

/**
 *  Extract daterange.
 *
 *  @param[in] obj  The object to write.
 */
void objects::_add_daterange(daterange const* obj) {
  typedef std::string (*func)(daterange const*);
  static func tab[] = {
    &_build_daterange_calendar_date,
    &_build_daterange_month_date,
    &_build_daterange_month_day,
    &_build_daterange_month_week_day,
    &_build_daterange_week_day,
    NULL
  };

  if (!obj)
    return;
  if (obj->type < 0 || obj->type >= DATERANGE_TYPES)
    throw (engine_error() << "save configuration: invalid daterange "
           "type: type='" << obj->type << "'");
  std::string key((*(tab[obj->type]))(obj));
  _add_timerange(key.c_str(), obj->times);
}

/**
 *  Extract hosts member.
 *
 *  @param[in] key  The key name.
 *  @param[in] obj  The object to write.
 */
void objects::_add_hosts(char const* key, hostsmember const* obj) {
  if (!obj)
    return;
  _stream << "  " << key << " ";
  for (hostsmember const* hm(obj); hm; hm = hm->next)
    _stream << hm->host_name << (hm->next ? "," : "\n");
}

/**
 *  Write a new line into the configuration file.
 *
 *  @param[in] key    The key name.
 *  @param[in] value  The value to write.
 */
template<typename T>
void objects::_add_line(char const* key, T value) {
  _stream << "  " << key << " " << value << "\n";
}

/**
 *  Extract services member.
 *
 *  @param[in] key  The key name.
 *  @param[in] obj  The object to write.
 */
void objects::_add_services(char const* key, servicesmember const* obj) {
  if (!obj)
    return;
  _stream << "  " << key << " ";
  for (servicesmember const* sm(obj); sm; sm = sm->next)
    _stream << sm->host_name << "," << sm->service_description
            << (sm->next ? "," : "\n");
}

/**
 *  write a new line into the configuration file.
 *
 *  @param[in] key    The key name.
 *  @param[in] value  The value to write.
 */
void objects::_add_string(char const* key, char const* value) {
  if (value && value[0])
    _stream << "  " << key << " " << value << "\n";
}

/**
 *  Write a new line into the configuration file.
 *
 *  @param[in] key    The key name.
 *  @param[in] value  The value to write.
 */
void objects::_add_string(char const* key, std::string const& value) {
  if (!value.empty())
    _stream << "  " << key << " " << value << "\n";
}

/**
 *  Extract timeperiod exclusion.
 *
 *  @param[in] key  The key name.
 *  @param[in] obj  The object to write.
 */
void objects::_add_timeperiodexclusion(
             char const* key,
             timeperiodexclusion const* obj) {
  if (!obj)
    return;
  _stream << "  " << key << " ";
  for (timeperiodexclusion const* te(obj); te; te = te->next)
    _stream << te->timeperiod_name << (te->next ? "," : "\n");
}

/**
 *  Extract timerange.
 *
 *  @param[in] key  The key name.
 *  @param[in] obj  The object to write.
 */
void objects::_add_timerange(char const* key, timerange const* obj) {
  if (!obj)
    return;
  _stream << "  " << key << " ";
  for (timerange const* range(obj); range; range = range->next) {
    unsigned int start_hours(range->range_start / 3600);
    unsigned int start_minutes((range->range_start % 3600) / 60);
    unsigned int end_hours(range->range_end / 3600);
    unsigned int end_minutes((range->range_end % 3600) / 60);
    _stream << std::setfill('0') << std::setw(2) << start_hours << ":"
            << std::setfill('0') << std::setw(2) << start_minutes << "-"
            << std::setfill('0') << std::setw(2) << end_hours << ":"
            << std::setfill('0') << std::setw(2) << end_minutes
            << (range->next ? "," : "\n");
  }
}

/**
 *  Build string with the daterange value into the calendar date format.
 *
 *  @param[in] obj  The daterange to stringify.
 *
 *  @return The string representation.
 */
std::string objects::_build_daterange_calendar_date(daterange const* obj) {
  if (!obj || obj->type != DATERANGE_CALENDAR_DATE)
    return ("");
  std::ostringstream oss;
  oss << obj->syear << "-" << obj->smon + 1 << "-" << obj->smday << " - "
      << obj->eyear << "-" << obj->emon + 1 << "-" << obj->emday << " / "
      << obj->skip_interval;
  return (oss.str());
}

/**
 *  Build string with the daterange value into the month date format.
 *
 *  @param[in] obj  The daterange to stringify.
 *
 *  @return The string representation.
 */
std::string objects::_build_daterange_month_date(daterange const* obj) {
  if (!obj || obj->type != DATERANGE_MONTH_DATE)
    return ("");
  std::ostringstream oss;
  oss << _get_month(obj->smon) << " " << obj->smday << " - "
      << _get_month(obj->emon) << " " << obj->emday << " / "
      << obj->skip_interval;
  return (oss.str());
}

/**
 *  Build string with the daterange value into the month day format.
 *
 *  @param[in] obj  The daterange to stringify.
 *
 *  @return The string representation.
 */
std::string objects::_build_daterange_month_day(daterange const* obj) {
  if (!obj || obj->type != DATERANGE_MONTH_DAY)
    return ("");
  std::ostringstream oss;
  oss << "day " << obj->smday << " - day " << obj->emday << " / "
      << obj->skip_interval;
  return (oss.str());
}

/**
 *  Build string with the daterange value into the month week day
 *  format.
 *
 *  @param[in] obj  The daterange to stringify.
 *
 *  @return The string representation.
 */
std::string objects::_build_daterange_month_week_day(
                    daterange const* obj) {
  if (!obj || obj->type != DATERANGE_MONTH_WEEK_DAY)
    return ("");
  std::ostringstream oss;
  oss << _get_weekday(obj->swday) << " " << obj->swday_offset << " "
      << _get_month(obj->smon) << " - " << _get_weekday(obj->ewday)
      << " " << obj->ewday_offset << " " << _get_month(obj->emon)
      << " / " << obj->skip_interval;
  return (oss.str());
}

/**
 *  Build string with the daterange value into the week day format.
 *
 *  @param[in] obj  The daterange to stringify.
 *
 *  @return The string representation.
 */
std::string objects::_build_daterange_week_day(daterange const* obj) {
  if (!obj || obj->type != DATERANGE_WEEK_DAY)
    return ("");
  std::ostringstream oss;
  oss << _get_weekday(obj->swday) << " " << obj->smday << " - "
      << _get_weekday(obj->ewday) << " " << obj->emday << " / "
      << obj->skip_interval;
  return (oss.str());
}

/**
 *  Get the month name.
 *
 *  @param[in] index  The month position.
 *
 *  @return The month name.
 */
char const* objects::_get_month(unsigned int index) {
  static char const* month[] = {
    "january",
    "february",
    "march",
    "april",
    "may",
    "june",
    "july",
    "august",
    "september",
    "october",
    "november",
    "december"
  };
  if (index >= sizeof(month) / sizeof(*month))
    throw (engine_error() << "save configuration: invalid month "
           "index: index='" << index << "'");
  return (month[index]);
}

/**
 *  Get the weekday name.
 *
 *  @param[in] index  The weekday position.
 *
 *  @return The weekday name.
 */
char const* objects::_get_weekday(unsigned int index) {
  static char const* days[] = {
    "sunday",
    "monday",
    "tuesday",
    "wednesday",
    "thursday",
    "friday",
    "saturday"
  };
  if (index >= sizeof(days) / sizeof(*days))
    throw (engine_error() << "save configuration: invalid weekday "
           "index: index='" << index << "'");
  return (days[index]);
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
objects& objects::_internal_copy(objects const& right) {
  if (&right != this) {
    _stream.str(right._stream.str());
  }
  return (*this);
}
