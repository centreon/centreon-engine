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

#ifndef CCE_CONFIGURATION_SERVICEGROUP_HH
#  define CCE_CONFIGURATION_SERVICEGROUP_HH

#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    servicegroup
    : public object {
  public:
                           servicegroup();
                           servicegroup(servicegroup const& right);
                           ~servicegroup() throw ();
    servicegroup&          operator=(servicegroup const& right);
    bool                   operator==(
                             servicegroup const& right) const throw ();
    bool                   operator!=(
                             servicegroup const& right) const throw ();
    /*
    std::string const&     action_url() const throw ();
    std::string const&     alias() const throw ();
    std::list<std::string> const&
                           members() const throw ();
    std::string const&     notes() const throw ();
    std::string const&     notes_url() const throw ();
    std::list<std::string> const&
                           servicegroup_members() const throw ();
    std::string const&     servicegroup_name() const throw ();
    */

    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

  private:
    void                   _set_action_url(std::string const& value);
    void                   _set_alias(std::string const& value);
    void                   _set_members(std::string const& value);
    void                   _set_notes(std::string const& value);
    void                   _set_notes_url(std::string const& value);
    void                   _set_servicegroup_members(std::string const& value);
    void                   _set_servicegroup_name(std::string const& value);

    std::string            _action_url;
    std::string            _alias;
    group                  _members;
    std::string            _notes;
    std::string            _notes_url;
    group                  _servicegroup_members;
    std::string            _servicegroup_name;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_SERVICEGROUP_HH
