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

#ifndef CCE_MOD_WS_SERVER_FREE_OBJECT_HH
# define CCE_MOD_WS_SERVER_FREE_OBJECT_HH

# include "objects.hh"

namespace                            com {
  namespace                          centreon {
    namespace                        engine {
      namespace                      modules {
	contactsmember const*        free_contactsmember(contactsmember const* member);
	contactgroupsmember const*   free_contactgroupsmember(contactgroupsmember const* member);
	hostsmember const*           free_hostsmember(hostsmember const* member);
	servicesmember const*        free_servicesmember(servicesmember const* member);
	commandsmember const*        free_commandsmember(commandsmember const* member);
	customvariablesmember const* free_customvariablesmember(customvariablesmember const* member);

	void                         free_objectlist(objectlist const* obj);
	void                         free_contactgroup(contactgroup const* group);
	void                         free_hostgroup(hostgroup const* group);
	void                         free_servicegroup(servicegroup const* group);
	void                         free_host(host const* hst);
	void                         free_service(service const* svc);
	void                         free_contact(contact const* cntct);
        void                         free_hostescalation(hostescalation const* hstescalation);
        void                         free_serviceescalation(serviceescalation const* svcescalation);
        void                         free_timeperiod(timeperiod const* tperiod);
      }
    }
  }
}

#endif // !CCE_MOD_WS_SERVER_FREE_OBJECT_HH
