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

#ifndef CCE_CONFIGURATION_APPLIER_CONTACTGROUP_HH
#define CCE_CONFIGURATION_APPLIER_CONTACTGROUP_HH

#include <map>
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
// Forward declarations.
class state;

namespace applier {
class contactgroup {
 public:
  contactgroup();
  contactgroup(contactgroup const& right);
  ~contactgroup() throw();
  contactgroup& operator=(contactgroup const& right);
  void add_object(configuration::contactgroup const& obj);
  void expand_objects(configuration::state& s);
  void modify_object(configuration::contactgroup const& obj);
  void remove_object(configuration::contactgroup const& obj);
  void resolve_object(configuration::contactgroup const& obj);

 private:
  typedef std::map<configuration::contactgroup::key_type,
                   configuration::contactgroup>
      resolved_set;

  void _resolve_members(configuration::state& s,
                        configuration::contactgroup const& obj);

  resolved_set _resolved;
};
}  // namespace applier
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_APPLIER_CONTACTGROUP_HH
