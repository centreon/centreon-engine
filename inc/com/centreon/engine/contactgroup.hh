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

#ifndef CCE_OBJECTS_CONTACTGROUP_HH
#  define CCE_OBJECTS_CONTACTGROUP_HH

#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include "com/centreon/engine/namespace.hh"

/* Forward declaration. */
CCE_BEGIN()
class contact;
class contactgroup;

namespace configuration {
  class contactgroup;
}
CCE_END()

typedef std::unordered_map<std::string,
  std::shared_ptr<com::centreon::engine::contactgroup>>
    contactgroup_map;
typedef std::unordered_map<std::string,
    com::centreon::engine::contact*> contact_map_unsafe;
typedef std::unordered_map<std::string,
    com::centreon::engine::contactgroup*> contactgroup_map_unsafe;

CCE_BEGIN()

class contactgroup {
 public:
                      contactgroup();
                      contactgroup(configuration::contactgroup const &obj);
   virtual            ~contactgroup();
   std::string const& get_name() const;
   std::string const& get_alias() const;
   void               set_alias(std::string const &alias);
   void               add_member(contact* cntct);
   void               clear_members();
   contact_map_unsafe const&
                      get_members() const;

   contactgroup& operator=(contactgroup const& other);

   static contactgroup_map
                      contactgroups;

 private:
   std::string        _alias;
   contact_map_unsafe _members;
   std::string        _name;
};

CCE_END()

std::ostream& operator<<(
                std::ostream& os,
                contactgroup_map const& obj);

#endif // !CCE_OBJECTS_CONTACTGROUP_HH
