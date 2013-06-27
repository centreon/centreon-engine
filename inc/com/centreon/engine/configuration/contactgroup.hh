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

#ifndef CCE_CONFIGURATION_CONTACTGROUP_HH
#  define CCE_CONFIGURATION_CONTACTGROUP_HH

#  include <set>
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/configuration/opt.hh"
#  include "com/centreon/engine/namespace.hh"

typedef std::set<std::string> set_string;

CCE_BEGIN()

namespace                  configuration {
  class                    contactgroup
    : public object {
  public:
                           contactgroup();
                           contactgroup(contactgroup const& right);
                           ~contactgroup() throw ();
    contactgroup&          operator=(contactgroup const& right);
    bool                   operator==(
                             contactgroup const& right) const throw ();
    bool                   operator!=(
                             contactgroup const& right) const throw ();
    bool                   operator<(
                             contactgroup const& right) const throw ();
    void                   check_validity() const;
    std::size_t            id() const throw ();
    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

    std::string const&     alias() const throw ();
    list_string const&     contactgroup_members() const throw ();
    std::string const&     contactgroup_name() const throw ();
    list_string&           members() throw ();
    list_string const&     members() const throw ();

    bool                   is_resolved() const throw ();
    set_string&            resolved_members() const throw ();
    void                   set_resolved(bool resolved) const throw ();

  private:
    bool                   _set_alias(std::string const& value);
    bool                   _set_contactgroup_members(std::string const& value);
    bool                   _set_contactgroup_name(std::string const& value);
    bool                   _set_members(std::string const& value);

    std::string            _alias;
    opt<group>             _contactgroup_members;
    std::string            _contactgroup_name;
    opt<group>             _members;
    mutable bool           _resolved;
    mutable set_string     _resolved_members;
  };

  typedef shared_ptr<contactgroup>   contactgroup_ptr;
  typedef std::set<contactgroup_ptr> set_contactgroup;
}

CCE_END()

#endif // !CCE_CONFIGURATION_CONTACTGROUP_HH
