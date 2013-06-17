/*
** Copyright 1999-2008 Ethan Galstad
** Copyright 2011-2013 Merethis
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

#ifndef CCE_OBJECTS_HH
#  define CCE_OBJECTS_HH

#  include <time.h>
#  include "com/centreon/engine/common.hh"
#  include "com/centreon/engine/objects/command.hh"
#  include "com/centreon/engine/objects/commandsmember.hh"
#  include "com/centreon/engine/objects/contact.hh"
#  include "com/centreon/engine/objects/contactgroup.hh"
#  include "com/centreon/engine/objects/contactgroupsmember.hh"
#  include "com/centreon/engine/objects/contactsmember.hh"
#  include "com/centreon/engine/objects/customvariablesmember.hh"
#  include "com/centreon/engine/objects/daterange.hh"
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/hostdependency.hh"
#  include "com/centreon/engine/objects/hostescalation.hh"
#  include "com/centreon/engine/objects/hostgroup.hh"
#  include "com/centreon/engine/objects/hostsmember.hh"
#  include "com/centreon/engine/objects/objectlist.hh"
#  include "com/centreon/engine/objects/service.hh"
#  include "com/centreon/engine/objects/servicedependency.hh"
#  include "com/centreon/engine/objects/serviceescalation.hh"
#  include "com/centreon/engine/objects/servicegroup.hh"
#  include "com/centreon/engine/objects/servicesmember.hh"
#  include "com/centreon/engine/objects/timeperiod.hh"
#  include "com/centreon/engine/objects/timeperiodexclusion.hh"
#  include "com/centreon/engine/objects/timerange.hh"
#  include "find.hh"

/*
** Current object revision, Increment when changes are made to data
** structures...
*/
#  define CURRENT_OBJECT_STRUCTURE_VERSION 307

/* Skip lists. */
#  define HOST_SKIPLIST                    0
#  define SERVICE_SKIPLIST                 1
#  define COMMAND_SKIPLIST                 2
#  define TIMEPERIOD_SKIPLIST              3
#  define CONTACT_SKIPLIST                 4
#  define CONTACTGROUP_SKIPLIST            5
#  define HOSTGROUP_SKIPLIST               6
#  define SERVICEGROUP_SKIPLIST            7
#  define HOSTDEPENDENCY_SKIPLIST          8
#  define SERVICEDEPENDENCY_SKIPLIST       9
#  define HOSTESCALATION_SKIPLIST          10
#  define SERVICEESCALATION_SKIPLIST       11
#  define NUM_OBJECT_SKIPLISTS             12

/* Other HOST structure. */
struct                   host_other_properties {
  time_t                 initial_notif_time;
};

/* Other SERVICE structure. */
struct                   service_other_properties {
  time_t                 initial_notif_time;
};

/* Hash structures. */
typedef struct host_cursor_struct {
  int          host_hashchain_iterator;
  host*        current_host_pointer;
}              host_cursor;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

/*
** Object creation functions.
*/
/* Adds a child host to a host definition. */
hostsmember*           add_child_link_to_host(
                         host* hst,
                         host* child_ptr);
/* Adds a command definition. */
command*               add_command(char const* name, char const* value);
/* Adds a contact definition. */
contact*               add_contact(
                         char const* name,
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
/* Adds a contact to a contact group definition. */
contactsmember*        add_contact_to_contactgroup(
                         contactgroup* grp,
                         char const* contact_name);
/* Adds a contact to a host definition. */
contactsmember*        add_contact_to_host(host* hst, char const* contact_name);
/* Adds a contact to a host escalation definition. */
contactsmember*        add_contact_to_host_escalation(
                         hostescalation* he,
                         char const* contact_name);
/* Adds a contact to an object. */
contactsmember*        add_contact_to_object(
                         contactsmember** object_ptr,
                         char const* contact_name);
/* Adds a contact to a host definition. */
contactsmember*        add_contact_to_service(
                         service* svc,
                         char const* contact_name);
/* Adds a contact to a service escalation definition. */
contactsmember*        add_contact_to_serviceescalation(
                         serviceescalation* se,
                         char const* contact_name);
/* Adds a contactgroup definition. */
contactgroup*          add_contactgroup(
                         char const* name,
                         char const* alias);
/* Adds a contactgroup to a host definition. */
contactgroupsmember*   add_contactgroup_to_host(
                         host* hst,
                         char const* group_name);
/* Adds a contact group to a host escalation definition. */
contactgroupsmember*   add_contactgroup_to_host_escalation(
                         hostescalation* he,
                         char const* group_name);
/* Adds a contact group to a service definition. */
contactgroupsmember*   add_contactgroup_to_service(
                         service* svc,
                         char const* group_name);
/* Adds a contact group to a service escalation definition. */
contactgroupsmember*   add_contactgroup_to_serviceescalation(
                         serviceescalation* se,
                         char const* group_name);
/* Adds a custom variable to a service definition. */
customvariablesmember* add_custom_variable_to_contact(
                         contact* cntct,
                         char const* varname,
                         char const* varvalue);
/* Adds a custom variable to a host definition. */
customvariablesmember* add_custom_variable_to_host(
                         host* hst,
                         char const* varname,
                         char const* varvalue);
/* Adds a custom variable to an object. */
customvariablesmember* add_custom_variable_to_object(
                         customvariablesmember** object_ptr,
                         char const* varname,
                         char const* varvalue);
/* Adds a custom variable to a service definition. */
customvariablesmember* add_custom_variable_to_service(
                         service* svc,
                         char const* varname,
                         char const* varvalue);
/* Adds an exception to a timeperiod. */
daterange*             add_exception_to_timeperiod(
                         timeperiod* period,
                         int type,
                         int syear,
                         int smon,
                         int smday,
                         int swday,
                         int swday_offset,
                         int eyear,
                         int emon,
                         int emday,
                         int ewday,
                         int ewday_offset,
                         int skip_interval);
/* Adds an exclusion to a timeperiod. */
timeperiodexclusion*   add_exclusion_to_timeperiod(
                         timeperiod* period,
                         char const* name);
/* Adds a host definition. */
host*                  add_host(
                         char const* name,
                         char const* display_name,
                         char const* alias,
                         char const* address,
                         char const* check_period,
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
                         char const* notification_period,
                         int notifications_enabled,
                         char const* check_command,
                         int checks_enabled,
                         int accept_passive_checks,
                         char const* event_handler,
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
                         int failure_prediction_enabled,
                         char const* failure_prediction_options,
                         int check_freshness,
                         int freshness_threshold,
                         char const* notes,
                         char const* notes_url,
                         char const* action_url,
                         char const* icon_image,
                         char const* icon_image_alt,
                         char const* vrml_image,
                         char const* statusmap_image,
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
                         int obsess_over_host);
/* Adds a host dependency definition. */
hostdependency*        add_host_dependency(
                         char const* dependent_host_name,
                         char const* host_name,
                         int dependency_type,
                         int inherits_parent,
                         int fail_on_up,
                         int fail_on_down,
                         int fail_on_unreachable,
                         int fail_on_pending,
                         char const* dependency_period);
/* Adds a host notification command to a contact definition. */
commandsmember*        add_host_notification_command_to_contact(
                         contact* cntct,
                         char const* command_name);
/* Adds a host to a hostgroup definition. */
hostsmember*           add_host_to_hostgroup(
                         hostgroup* temp_hostgroup,
                         char const* host_name);
/* Adds a host escalation definition. */
hostescalation*        add_host_escalation(
                         char const* host_name,
                         int first_notification,
                         int last_notification,
                         double notification_interval,
                         char const* escalation_period,
                         int escalate_on_down,
                         int escalate_on_unreachable,
                         int escalate_on_recovery);
/* Adds a hostgroup definition. */
hostgroup*             add_hostgroup(
                         char const* name,
                         char const* alias,
                         char const* notes,
                         char const* notes_url,
                         char const* action_url);
/* Adds a parent host to a host definition. */
hostsmember*           add_parent_host_to_host(
                         host* hst,
                         char const* host_name);
/* Adds a service definition. */
service*               add_service(
                         char const* host_name,
                         char const* description,
                         char const* display_name,
                         char const* check_period,
                         int initial_state,
                         int max_attempts,
                         int parallelize,
                         int accept_passive_checks,
                         double check_interval,
                         double retry_interval,
                         double notification_interval,
                         double first_notification_delay,
                         char const* notification_period,
                         int notify_recovery,
                         int notify_unknown,
                         int notify_warning,
                         int notify_critical,
                         int notify_flapping,
                         int notify_downtime,
                         int notifications_enabled,
                         int is_volatile,
                         char const* event_handler,
                         int event_handler_enabled,
                         char const* check_command,
                         int checks_enabled,
                         int flap_detection_enabled,
                         double low_flap_threshold,
                         double high_flap_threshold,
                         int flap_detection_on_ok,
                         int flap_detection_on_warning,
                         int flap_detection_on_unknown,
                         int flap_detection_on_critical,
                         int stalk_on_ok,
                         int stalk_on_warning,
                         int stalk_on_unknown,
                         int stalk_on_critical,
                         int process_perfdata,
                         int failure_prediction_enabled,
                         char const* failure_prediction_options,
                         int check_freshness,
                         int freshness_threshold,
                         char const* notes,
                         char const* notes_url,
                         char const* action_url,
                         char const* icon_image,
                         char const* icon_image_alt,
                         int retain_status_information,
                         int retain_nonstatus_information,
                         int obsess_over_service);
/* Adds a service dependency definition. */
servicedependency*     add_service_dependency(
                         char const* dependent_host_name,
                         char const* dependent_service_description,
                         char const* host_name,
                         char const* service_description,
                         int dependency_type,
                         int inherits_parent,
                         int fail_on_ok,
                         int fail_on_warning,
                         int fail_on_unknown,
                         int fail_on_critical,
                         int fail_on_pending,
                         char const* dependency_period);
/* Link service to host. */
servicesmember*        add_service_link_to_host(
                         host* hst,
                         service* service_ptr);
/* Adds a service notification command to a contact definition. */
commandsmember*        add_service_notification_command_to_contact(
                         contact* cntct,
                         char const* command_name);
/* Adds a service to a servicegroup definition. */
servicesmember*        add_service_to_servicegroup(
                         servicegroup* temp_servicegroup,
                         char const* host_name,
                         char const* svc_description);
/* Adds a service escalation definition. */
serviceescalation*     add_service_escalation(
                         char const* host_name,
                         char const* description,
                         int first_notification,
                         int last_notification,
                         double notification_interval,
                         char const* escalation_period,
                         int escalate_on_warning,
                         int escalate_on_unknown,
                         int escalate_on_critical,
                         int escalate_on_recovery);
/* Adds a servicegroup definition. */
servicegroup*          add_servicegroup(
                         char const* name,
                         char const* alias,
                         char const* notes,
                         char const* notes_url,
                         char const* action_url);
/* Adds a timeperiod definition. */
timeperiod*            add_timeperiod(
                         char const* name,
                         char const* alias);
/* Adds a timerange to a daterange. */
timerange*             add_timerange_to_daterange(
                         daterange* drange,
                         unsigned long start_time,
                         unsigned long end_time);
/* Adds a timerange to a timeperiod definition. */
timerange*             add_timerange_to_timeperiod(
                         timeperiod* period,
                         int day,
                         unsigned long start_time,
                         unsigned long end_time);

/*
** Object modification functions.
*/
int modify_command(char const* name, char const* value);

/*
** Object cleanup functions.
*/
/* Frees all allocated memory for the object definitions. */
int free_object_data();
int remove_command_by_id(char const* command_name);
int remove_contact_by_id(char const* contact_name);
int remove_contactgroup_by_id(char const* contactgroup_name);
int remove_host_by_id(char const* host_name);
int remove_host_dependency_by_id(
      char const* host_name,
      char const* dependency_name);
int remove_host_escalation_by_id(char const* host_name);
int remove_hostgroup_by_id(char const* hostgroup_name);
int remove_service_by_id(
      char const* host_name,
      char const* service_description);
int remove_service_dependency_by_id(
      char const* host_name,
      char const* service_description,
      char const* dependency_name,
      char const* dependency_service_description);
int remove_service_escalation_by_id(
      char const* host_name,
      char const* service_description);
int remove_servicegroup_by_id(char const* servicegroup_name);
int remove_timeperiod_by_id(char const* name);

/*
** Count functions.
*/
int get_host_count();
int get_service_count();
/* Counts the number of immediate child hosts for a particular host. */
int number_of_immediate_child_hosts(host* hst);
/* Counts the number of immediate parents hosts for a particular host. */
int number_of_immediate_parent_hosts(host* hst);
/* Counts the number of total child hosts for a particular host. */
int number_of_total_child_hosts(host* hst);
/* Counts the number of total parents hosts for a particular host. */
int number_of_total_parent_hosts(host* hst);

/*
** Object query functions.
*/
/* Tests whether or not a contact is a contact member for a specific host. */
int is_contact_for_host(host* hst, contact* cntct);
/* Tests whether or not a contact is a contact member for a specific service. */
int is_contact_for_service(service* svc, contact* cntct);
/* Tests whether or not a contact is a member of a specific contact group. */
int is_contact_member_of_contactgroup(
      contactgroup* group,
      contact* cntct);
/* Checks whether or not a contact is an escalated contact for a specific host. */
int is_escalated_contact_for_host(host* hst, contact* cntct);
/* Checks whether or not a contact is an escalated contact for a specific service. */
int is_escalated_contact_for_service(service* svc, contact* cntct);
/* Checks if a host is an immediate child of another host. */
int is_host_immediate_child_of_host(
      host* parent_host,
      host* child_host);
/* Checks if a host is an immediate child of another host. */
int is_host_immediate_parent_of_host(
      host* child_host,
      host* parent_host);
/* Tests whether or not a host is a member of a specific hostgroup. */
int is_host_member_of_hostgroup(hostgroup* group, host* hst);
/* Tests whether or not a service is a member of a specific servicegroup. */
int is_host_member_of_servicegroup(servicegroup* group, host* hst);
/* Tests whether or not a service is a member of a specific servicegroup. */
int is_service_member_of_servicegroup(
      servicegroup* group,
      service* svc);

/*
** Circular dependency check functions.
*/
/* Checks if a circular dependency exists for a given host. */
int check_for_circular_hostdependency_path(
      hostdependency* root_dep,
      hostdependency* dep,
      int dependency_type);
/* Checks if a circular dependency exists for a given service. */
int check_for_circular_servicedependency_path(
      servicedependency* root_dep,
      servicedependency* dep,
      int dependency_type);

/*
** Object lists functions.
*/
int add_object_to_objectlist(objectlist** list, void* object_ptr);
int free_objectlist(objectlist** temp_list);
int remove_object_to_objectlist(objectlist** list, void* object_ptr);

/*
** Config functions.
*/
/* Reads all external configuration data of specific types. */
int read_object_config_data(
      char const* main_config_file,
      int options,
      int cache,
      int precache);

#  ifdef __cplusplus
}
#  endif /* C++ */

#endif /* !CCE_OBJECTS_HH */
