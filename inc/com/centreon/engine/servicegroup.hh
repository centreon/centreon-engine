/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_OBJECTS_SERVICEGROUP_HH
#define CCE_OBJECTS_SERVICEGROUP_HH

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/service.hh"

/* Forward declaration. */
CCE_BEGIN()
class host;
class servicegroup;
CCE_END()

typedef std::unordered_map<std::string,
                           std::shared_ptr<com::centreon::engine::servicegroup>>
    servicegroup_map;

CCE_BEGIN()
class servicegroup {
 public:
  servicegroup(uint64_t id,
               std::string const& group_name,
               std::string const& alias,
               std::string const& notes,
               std::string const& notes_url,
               std::string const& action_url);
  uint64_t get_id() const;
  void set_id(uint64_t id);
  std::string const& get_group_name() const;
  void set_group_name(std::string const& group_name);
  std::string const& get_alias() const;
  void set_alias(std::string const& alias);
  std::string const& get_notes() const;
  void set_notes(std::string const& notes);
  std::string const& get_notes_url() const;
  void set_notes_url(std::string const& notes_url);
  std::string const& get_action_url() const;
  void set_action_url(std::string const& action_url);
  void resolve(int& w, int& e);

  bool operator==(servicegroup const& obj) = delete;
  bool operator!=(servicegroup const& obj) = delete;

  static servicegroup_map servicegroups;
  service_map_unsafe members;

 private:
  uint64_t _id;
  std::string _group_name;
  std::string _alias;
  std::string _notes;
  std::string _notes_url;
  std::string _action_url;
};

bool is_servicegroup_exist(std::string const& name) throw();

CCE_END()

std::ostream& operator<<(std::ostream& os,
                         com::centreon::engine::servicegroup const& obj);

#endif  // !CCE_OBJECTS_SERVICEGROUP_HH
