/*
** Copyright 2011-2012 Merethis
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

#  include <map>
#  include <QString>
#  include <QVector>
#  include <string>
#  include "com/centreon/engine/modules/webservice/namespace.hh"
#  include "com/centreon/engine/objects.hh"
#  include "soapH.h"

CCE_MOD_WS_BEGIN()

void create_contact(ns1__contactType const& contact);
void create_contactgroup(ns1__contactgroupType const& contactgroup);
void create_command(ns1__commandType const& command);
void create_host(ns1__hostType const& host);
void create_host_dependency(ns1__hostDependencyType const& hostdependency);
void create_host_escalation(ns1__hostEscalationType const& hostescalation);
void create_hostgroup(ns1__hostgroupType const& hostgroup);
void create_service(ns1__serviceType const& service);
void create_service_dependency(ns1__serviceDependencyType const& servicedependency);
void create_service_escalation(ns1__serviceEscalationType const& serviceescalation);
void create_servicegroup(ns1__servicegroupType const& servicegroup);
void create_timeperiod(ns1__timeperiodType const& tperiod);
QVector<service*>
     _find(std::vector<std::string> const& vec);
std::map<char, bool>
     get_options(
       std::string const* opt,
       std::string const& pattern,
       char const* default_opt);
QVector<QString>
     std2qt(std::vector<std::string> const& vec);

/**
 *  Find objects by name and create a table of it.
 *
 *  @param[in]  objs        The object to find.
 *  @param[in]  find_object The function to find an object.
 *
 *  @return The object's table, stop when the first object are not found.
 */
template<class T>
QVector<T*> _find(
              std::vector<std::string> const& objs,
              void* (find_object)(char const*)) {
  QVector<T*> res;
  res.reserve(objs.size());
  for (std::vector<std::string>::const_iterator
         it(objs.begin()),
         end = objs.end();
       it != end;
       ++it) {
    // Check if the object exists ...
    void* obj((*find_object)(it->c_str()));
    if (!obj)
      return (res);
    res.push_back(static_cast<T*>(obj));
  }
  return (res);
}

CCE_MOD_WS_END()

#endif // !CCE_MOD_WS_CREATE_OBJECT_HH
