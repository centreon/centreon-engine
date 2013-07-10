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

#ifndef CCE_CONFIGURATION_APPLIER_HOSTESCALATION_HH
#  define CCE_CONFIGURATION_APPLIER_HOSTESCALATION_HH

#  include <list>
#  include <set>
#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace             configuration {
  // Forward declarations.
  class               hostescalation;
  class               state;

  namespace           applier {
    class             hostescalation {
    public:
                      hostescalation();
                      hostescalation(hostescalation const& right);
                      ~hostescalation() throw ();
      hostescalation& operator=(hostescalation const& right);
      void            add_object(
                        shared_ptr<configuration::hostescalation> obj);
      void            expand_object(
                        shared_ptr<configuration::hostescalation> obj,
                        configuration::state& s);
      void            modify_object(
                        shared_ptr<configuration::hostescalation> obj);
      void            remove_object(
                        shared_ptr<configuration::hostescalation> obj);
      void            resolve_object(
                        shared_ptr<configuration::hostescalation> obj);

    private:
      void            _expand_hosts(
                        std::list<std::string> const& h,
                        std::list<std::string> const& hg,
                        configuration::state const& s,
                        std::set<std::string>& expanded);
      void            _inherits_special_vars(
                        shared_ptr<configuration::hostescalation> obj,
                        configuration::state& s);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_HOSTESCALATION_HH
