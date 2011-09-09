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

#ifndef CCE_MOD_WS_SERVER_ADD_OBJECT_HH
# define CCE_MOD_WS_SERVER_ADD_OBJECT_HH

# include "soapH.h"

namespace       com {
  namespace     centreon {
    namespace   engine {
      namespace modules {
	void    create_command(ns1__commandType const& command);
	void    create_contactgroup(ns1__contactGroupType const& contactgroup);
	void    create_hostgroup(ns1__hostGroupType const& hostgroup);
	void    create_servicegroup(ns1__serviceGroupType const& servicegroup);
	void    create_host(ns1__hostType const& host);
	void    create_service(ns1__serviceType const& service);
	void    create_contact(ns1__contactType const& contact);
	void    create_hostdependency(ns1__hostDependencyType const& hostdependency);
	void    create_hostescalation(ns1__hostEscalationType const& hostescalation);
	void    create_servicedependency(ns1__serviceDependencyType const& servicedependency);
	void    create_serviceescalation(ns1__serviceEscalationType const& serviceescalation);
        void    create_timeperiod(ns1__timeperiodType const& tperiod);
      }
    }
  }
}

#endif // !CCE_MOD_WS_SERVER_CREATE_OBJECT_HH
