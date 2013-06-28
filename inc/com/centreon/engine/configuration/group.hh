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

#ifndef CCE_CONFIGURATION_GROUP_HH
#  define CCE_CONFIGURATION_GROUP_HH

#  include <list>
#  include <string>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace  configuration {
  class    group {
  public:
           group(bool inherit = false);
           group(group const& right);
           ~group() throw ();
    group& operator=(group const& right);
    group& operator=(std::string const& right);
    bool   operator==(group const& right) const throw ();
    bool   operator!=(group const& right) const throw ();
    bool   operator<(group const& right) const throw ();
    void   clear();
    bool   empty() const throw ();
    std::list<std::string> const&
           get() const throw ();
    std::list<std::string>&
           get() throw ();
    bool   is_add_inherit() const throw ();
    void   is_add_inherit(bool enable) throw ();
    void   set(group const& grp);
    void   set(std::string const& value);

  private:
    std::list<std::string>
           _group;
    bool   _is_add_inherit;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_GROUP_HH

