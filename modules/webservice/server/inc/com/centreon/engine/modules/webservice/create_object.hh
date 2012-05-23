/*
** Copyright 2011 Merethis
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

#ifndef CCE_MOD_WS_CREATE_OBJECT_HH
#  define CCE_MOD_WS_CREATE_OBJECT_HH

#  include "soapH.h"
#  include "com/centreon/engine/modules/webservice/namespace.hh"

CCE_MOD_WS_BEGIN()

void create_contact(ns1__contactType const& contact);
void create_contact_group(ns1__contactGroupType const& contactgroup);
void create_command(ns1__commandType const& command);
void create_host(ns1__hostType const& host);
void create_host_dependency(ns1__hostDependencyType const& hostdependency);
void create_host_escalation(ns1__hostEscalationType const& hostescalation);
void create_host_group(ns1__hostGroupType const& hostgroup);
void create_service(ns1__serviceType const& service);
void create_service_dependency(ns1__serviceDependencyType const& servicedependency);
void create_service_escalation(ns1__serviceEscalationType const& serviceescalation);
void create_service_group(ns1__serviceGroupType const& servicegroup);
void create_timeperiod(ns1__timeperiodType const& tperiod);

CCE_MOD_WS_END()

#endif // !CCE_MOD_WS_CREATE_OBJECT_HH
