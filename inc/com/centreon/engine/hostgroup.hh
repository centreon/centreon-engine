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

#ifndef CCE_OBJECTS_HOSTGROUP_HH
#define CCE_OBJECTS_HOSTGROUP_HH

#include <ostream>
#include <string>
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/host.hh"

/* Forward declaration. */
CCE_BEGIN()
class hostgroup;
CCE_END()

typedef std::unordered_map<std::string,
  std::shared_ptr<com::centreon::engine::hostgroup>> hostgroup_map;


CCE_BEGIN()
class                     hostgroup {
 public:
                          hostgroup(uint64_t id,
                                    std::string const& name,
                                    std::string const& alias,
                                    std::string const& notes,
                                    std::string const& notes_url,
                                    std::string const& action_url);

  uint64_t                 get_id() const;
  void                     set_id(uint64_t id);
  std::string const&       get_group_name() const;
  void                     set_group_name(std::string const& group_name);
  std::string const&       get_alias() const;
  void                     set_alias(std::string const& alias);
  std::string const&       get_notes() const;
  void                     set_notes(std::string const& notes);
  std::string const&       get_notes_url() const;
  void                     set_notes_url(std::string const& notes_url);
  std::string const&       get_action_url() const;
  void                     set_action_url(std::string const& action_url);
  bool                     operator==(hostgroup const& obj) = delete;
  bool                     operator!=(hostgroup const& obj1) = delete;
  void resolve(int& w, int& e);

  host_map_unsafe          members;

  static hostgroup_map     hostgroups;

 private:
  uint64_t                 _id;
  std::string              _group_name;
  std::string              _alias;
  std::string              _notes;
  std::string              _notes_url;
  std::string              _action_url;
};
CCE_END()

std::ostream& operator<<(std::ostream& os, com::centreon::engine::hostgroup const& obj);

#endif // !CCE_OBJECTS_HOSTGROUP_HH


