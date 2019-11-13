/*
** Copyright 2011-2013,2017 Centreon
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

#ifndef CCE_CONFIGURATION_APPLIER_HOSTDEPENDENCY_HH
#define CCE_CONFIGURATION_APPLIER_HOSTDEPENDENCY_HH

#include <set>
#include <string>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
// Forward declarations.
class hostdependency;
class state;

namespace applier {
class hostdependency {
 public:
  hostdependency();
  hostdependency(hostdependency const& right) = delete;
  ~hostdependency() throw();
  hostdependency& operator=(hostdependency const& right) = delete;
  void add_object(configuration::hostdependency const& obj);
  void expand_objects(configuration::state& s);
  void modify_object(configuration::hostdependency const& obj);
  void remove_object(configuration::hostdependency const& obj);
  void resolve_object(configuration::hostdependency const& obj);

 private:
  void _expand_hosts(std::set<std::string> const& hosts,
                     std::set<std::string> const& hostgroups,
                     configuration::state& s,
                     std::set<std::string>& expanded);
};
}  // namespace applier
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_APPLIER_HOSTDEPENDENCY_HH
