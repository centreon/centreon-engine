/*
** Copyright 2011-2013,2015 Merethis
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

#ifndef CCE_CONFIGURATION_APPLIER_HOSTDEPENDENCY_HH
#  define CCE_CONFIGURATION_APPLIER_HOSTDEPENDENCY_HH

#  include <list>
#  include <set>
#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace             configuration {
  // Forward declarations.
  class               hostdependency;
  class               state;

  namespace           applier {
    class             hostdependency {
    public:
                      hostdependency();
                      hostdependency(hostdependency const& right);
                      ~hostdependency() throw ();
      hostdependency& operator=(hostdependency const& right);
      void            add_object(
                        shared_ptr<configuration::hostdependency> obj);
      void            expand_object(
                        shared_ptr<configuration::hostdependency> obj,
                        configuration::state& s);
      void            modify_object(
                        shared_ptr<configuration::hostdependency> obj);
      void            remove_object(
                        shared_ptr<configuration::hostdependency> obj);
      void            resolve_object(
                        shared_ptr<configuration::hostdependency> obj);

    private:
      void            _expand_hosts(
                        std::list<std::string> const& hosts,
                        std::set<std::string>& expanded);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_HOSTDEPENDENCY_HH
