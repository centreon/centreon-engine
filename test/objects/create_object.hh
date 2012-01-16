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

#ifndef TEST_OBJECTS_CREATE_OBJECT_HH
# define TEST_OBJECTS_CREATE_OBJECT_HH

# include "objects.hh"

namespace test {
  namespace objects {
    objectlist*            create_objectlist(unsigned int id);
    timerange*             create_timerange(unsigned int id);
    daterange*             create_daterange(unsigned int id);
    timeperiodexclusion*   create_timeperiodexclusion(unsigned int id);
    timeperiod*            create_timeperiod(unsigned int id);
    servicegroup*          create_servicegroup(unsigned int id);
    serviceescalation*     create_serviceescalation(unsigned int id);
    servicedependency*     create_servicedependency(unsigned int id);
    service*               create_service(unsigned int id);
    hostgroup*             create_hostgroup(unsigned int id);
    hostescalation*        create_hostescalation(unsigned int id);
    hostdependency*        create_hostdependency(unsigned int id);
    host*                  create_host(unsigned int id);
    contactgroup*          create_contactgroup(unsigned int id);
    contact*               create_contact(unsigned int id);
    command*               create_command(unsigned int id);
    commandsmember*        create_commandsmember(unsigned int id,
                                                 commandsmember** head);
    servicesmember*        create_servicesmember(unsigned int id,
                                                 servicesmember** head);
    hostsmember*           create_hostsmember(unsigned int id,
                                              hostsmember** head);
    customvariablesmember* create_customvariablesmember(unsigned int id,
                                                        customvariablesmember** head);
    contactsmember*        create_contactsmember(unsigned int id,
                                                 contactsmember** head);
    contactgroupsmember*   create_contactgroupsmember(unsigned int id,
                                                      contactgroupsmember** head);
  }
}

#endif // !TEST_OBJECTS_CREATE_OBJECT_HH
