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

#ifndef CCE_CONFIGURATION_APPLIER_HOSTGROUP_HH
#  define CCE_CONFIGURATION_APPLIER_HOSTGROUP_HH

#  include <set>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace               configuration {
  // Forward declarations.
  class                 hostgroup;
  class                 state;

  namespace             applier {
    class               hostgroup {
    public:
                        hostgroup();
                        hostgroup(hostgroup const& right);
                        ~hostgroup() throw ();
      hostgroup&        operator=(hostgroup const& right);
      void              add_object(
                          configuration::hostgroup const& obj,
                          configuration::state const& s);
      void              expand_object(
                          configuration::hostgroup const& obj,
                          configuration::state const& s,
                          std::set<shared_ptr<configuration::hostgroup> >& expanded);
      void              modify_object(
                          configuration::hostgroup const& obj,
                          configuration::state const& s);
      void              remove_object(
                          configuration::hostgroup const& obj,
                          configuration::state const& s);
      void              resolve_object(
                          configuration::hostgroup const& obj,
                          configuration::state const& s);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_HOSTGROUP_HH
