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

#ifndef CCE_OBJECTS_HOSTGROUP_HH
#  define CCE_OBJECTS_HOSTGROUP_HH

#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/objects.hh"

#  ifdef __cplusplus
#    include <vector>
extern "C" {
#  endif // C++

bool link_hostgroup(
       hostgroup* obj,
       host** members,
       hostgroup** groups);
void release_hostgroup(hostgroup const* obj);

#  ifdef __cplusplus
}

CCE_BEGIN()

namespace objects {
  void    link(
            hostgroup* obj,
            std::vector<host*> const& members,
            std::vector<hostgroup*> const& groups);
  void    release(hostgroup const* obj);
}

CCE_END()

#  endif // C++

#endif // !CCE_OBJECTS_HOSTGROUP_HH
