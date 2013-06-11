/*
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

#ifndef CCE_COMPATIBILITY_FIND_H
#  define CCE_COMPATIBILITY_FIND_H

#  ifdef __cplusplus
extern "C" {
#  endif // C++

struct command_struct;
struct contact_struct;
struct contactgroup_struct;
struct host_struct;
struct hostdependency_struct;
struct hostdependency_struct;
struct hostescalation_struct;
struct hostescalation_struct;
struct hostgroup_struct;
struct service_struct;
struct servicedependency_struct;
struct servicedependency_struct;
struct serviceescalation_struct;
struct serviceescalation_struct;
struct servicegroup_struct;
struct timeperiod_struct;

command_struct*           find_command(char const* name);
contact_struct*           find_contact(char const* name);
contactgroup_struct*      find_contactgroup(char const* name);
host_struct*              find_host(char const* name);
hostgroup_struct*         find_hostgroup(char const* name);
service_struct*           find_service(
                            char const* host_name,
                            char const* svc_desc);
servicegroup_struct*      find_servicegroup(char const* name);
timeperiod_struct*        find_timeperiod(char const* name);

hostdependency_struct*    get_first_host_dependency_by_dependent_host(
                            char const* host_name,
                            void** ptr);
hostescalation_struct*    get_first_host_escalation_by_host(
                            char const* host_name,
                            void** ptr);
servicedependency_struct* get_first_service_dependency_by_dependent_service(
                            char const* host_name,
                            char const* svc_description,
                            void** ptr);
serviceescalation_struct* get_first_service_escalation_by_service(
                            char const* host_name,
                            char const* svc_description,
                            void** ptr);
hostdependency_struct*    get_next_host_dependency_by_dependent_host(
                            char const* host_name,
                            void** ptr);
hostescalation_struct*    get_next_host_escalation_by_host(
                            char const* host_name,
                            void** ptr);
servicedependency_struct* get_next_service_dependency_by_dependent_service(
                            char const* host_name,
                            char const* svc_description,
                            void** ptr);
serviceescalation_struct* get_next_service_escalation_by_service(
                            char const* host_name,
                            char const* svc_description,
                            void** ptr);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_COMPATIBILITY_FIND_H


