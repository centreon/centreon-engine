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

#ifndef CCE_CONFIGURATION_HOSTGROUP_HH
#  define CCE_CONFIGURATION_HOSTGROUP_HH

#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    hostgroup
    : public object {
  public:
                           hostgroup();
                           hostgroup(hostgroup const& right);
                           ~hostgroup() throw ();
    hostgroup&             operator=(hostgroup const& right);
    bool                   operator==(
                             hostgroup const& right) const throw ();
    bool                   operator!=(
                             hostgroup const& right) const throw ();
    std::size_t            id() const throw ();
    bool                   is_valid() const throw ();
    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

  private:
    bool                   _set_action_url(std::string const& value);
    bool                   _set_alias(std::string const& value);
    bool                   _set_hostgroup_members(std::string const& value);
    bool                   _set_hostgroup_name(std::string const& value);
    bool                   _set_members(std::string const& value);
    bool                   _set_notes(std::string const& value);
    bool                   _set_notes_url(std::string const& value);

    std::string            _action_url;
    std::string            _alias;
    group                  _hostgroup_members;
    std::string            _hostgroup_name;
    group                  _members;
    std::string            _notes;
    std::string            _notes_url;
  };

  typedef umap<std::size_t, shared_ptr<hostgroup> > map_hostgroup;
}

CCE_END()

#endif // !CCE_CONFIGURATION_HOSTGROUP_HH


