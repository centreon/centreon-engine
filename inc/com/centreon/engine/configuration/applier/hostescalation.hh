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

#ifndef CCE_CONFIGURATION_APPLIER_HOSTESCALATION_HH
#  define CCE_CONFIGURATION_APPLIER_HOSTESCALATION_HH

#  include <set>
#  include <string>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace             configuration {
  // Forward declarations.
  class               hostescalation;
  class               state;

  namespace           applier {
    class             hostescalation {
     public:
      hostescalation();
      hostescalation(hostescalation const& right) = delete;
      ~hostescalation() throw();
      hostescalation& operator=(hostescalation const& right) = delete;
      void            add_object(
                        configuration::hostescalation const& obj);
      void            expand_objects(configuration::state& s);
      void            modify_object(
                        configuration::hostescalation const& obj);
      void            remove_object(
                        configuration::hostescalation const& obj);
      void            resolve_object(
                        configuration::hostescalation const& obj);

     private:
      void            _expand_hosts(
                        std::set<std::string> const& h,
                        std::set<std::string> const& hg,
                        configuration::state const& s,
                        std::set<std::string>& expanded);
      void            _inherits_special_vars(
                        configuration::hostescalation& obj,
                        configuration::state& s);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_HOSTESCALATION_HH
