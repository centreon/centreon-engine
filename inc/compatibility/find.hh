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

#ifndef CCE_COMPATIBILITY_FIND_H
#  define CCE_COMPATIBILITY_FIND_H
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()
class contact;
class service;
class servicegroup;
CCE_END()

#  ifdef __cplusplus
extern "C" {
#  endif // C++

struct hostdependency_struct;
struct hostescalation_struct;
struct servicedependency_struct;
struct serviceescalation_struct;

com::centreon::engine::service*           find_service(
                            char const* host_name,
                            char const* svc_desc);
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


