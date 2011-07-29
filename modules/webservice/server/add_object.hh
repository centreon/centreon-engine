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
	void    add_command(ns1__commandType const& command);
	void    add_contactgroup(ns1__contactGroupType const& contactgroup);
	void    add_hostgroup(ns1__hostGroupType const& hostgroup);
	void    add_servicegroup(ns1__serviceGroupType const& servicegroup);
	void    add_host(ns1__hostType const& host);
	void    add_service(ns1__serviceType const& service);
	void    add_contact(ns1__contactType const& contact);
      }
    }
  }
}

#endif // !CCE_MOD_WS_SERVER_ADD_OBJECT_HH
