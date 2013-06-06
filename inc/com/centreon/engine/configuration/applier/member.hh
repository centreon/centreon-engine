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

#ifndef CCE_CONFIGURATION_APPLIER_MEMBER_HH
#  define CCE_CONFIGURATION_APPLIER_MEMBER_HH

#  include <list>
#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"
#  include "com/centreon/unordered_hash.hh"

struct command_struct;
struct commandsmember_struct;
struct contact_struct;
struct contactsmember_struct;

CCE_BEGIN()

namespace             configuration {
  namespace           applier {
    void    add_member(
              umap<std::string, shared_ptr<contact_struct> > const& contacts,
              std::string const& name,
              contactsmember_struct*& members);
    void    add_member(
              umap<std::string, shared_ptr<command_struct> > const& commands,
              std::string const& name,
              commandsmember_struct*& members);

    template<typename T, typename U>
    void    add_members(
              umap<std::string, shared_ptr<T> > const& objects,
              std::list<std::string> const& lst,
              U*& members) {
      for (std::list<std::string>::const_iterator
             it(lst.begin()), end(lst.end());
           it != end;
           ++it)
        add_member(objects, *it, members);
    }

    template<typename T, char* T::*member>
    bool    members_has_change(
              std::list<std::string> const& lst,
              T const* members) {
      for (std::list<std::string>::const_iterator
             it(lst.begin()), end(lst.end());
           it != end;
           ++it)
        if (strcmp(members->*member, it->c_str()))
          return (true);
      return (false);
    }
    void    update_members(
              umap<std::string, shared_ptr<contact_struct> > const& contacts,
              std::list<std::string> const& lst,
              contactsmember_struct*& members);
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_MEMBER_HH
