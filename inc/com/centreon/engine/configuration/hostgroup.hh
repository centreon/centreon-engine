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

#ifndef CCE_CONFIGURATION_HOSTGROUP_HH
#  define CCE_CONFIGURATION_HOSTGROUP_HH

#  include <set>
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/opt.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    hostgroup : public object {
   public:
    typedef std::string    key_type;

                           hostgroup(key_type const& key = "");
                           hostgroup(hostgroup const& right);
                           ~hostgroup() throw () override;
    hostgroup&             operator=(hostgroup const& right);
    bool                   operator==(
                             hostgroup const& right) const throw ();
    bool                   operator!=(
                             hostgroup const& right) const throw ();
    bool                   operator<(
                             hostgroup const& right) const throw ();
    void                   check_validity() const override;
    key_type const&        key() const throw ();
    void                   merge(object const& obj) override;
    bool                   parse(char const* key, char const* value) override;

    std::string const&     action_url() const throw ();
    std::string const&     alias() const throw ();
    unsigned int           hostgroup_id() const throw();
    std::string const&     hostgroup_name() const throw ();
    set_string&            members() throw ();
    set_string const&      members() const throw ();
    std::string const&     notes() const throw ();
    std::string const&     notes_url() const throw ();

   private:
    typedef bool (*setter_func)(hostgroup&, char const*);

    bool                   _set_action_url(std::string const& value);
    bool                   _set_alias(std::string const& value);
    bool                   _set_hostgroup_id(unsigned int value);
    bool                   _set_hostgroup_name(std::string const& value);
    bool                   _set_members(std::string const& value);
    bool                   _set_notes(std::string const& value);
    bool                   _set_notes_url(std::string const& value);

    std::string            _action_url;
    std::string            _alias;
    unsigned int           _hostgroup_id;
    std::string            _hostgroup_name;
    group<set_string>      _members;
    std::string            _notes;
    std::string            _notes_url;
    static std::unordered_map<std::string, setter_func> const _setters;
  };

  typedef std::shared_ptr<hostgroup> hostgroup_ptr;
  typedef std::set<hostgroup>        set_hostgroup;
}

CCE_END()

#endif // !CCE_CONFIGURATION_HOSTGROUP_HH
