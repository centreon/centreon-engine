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

#ifndef CCE_CONFIGURATION_APPLIER_CONTACTGROUP_HH
#  define CCE_CONFIGURATION_APPLIER_CONTACTGROUP_HH

#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace           configuration {
  // Forward declarations.
  class             contactgroup;
  class             state;

  namespace         applier {
    class           contactgroup {
    public:
                    contactgroup();
                    contactgroup(contactgroup const& right);
                    ~contactgroup() throw ();
      contactgroup& operator=(contactgroup const& right);
      void          add_object(
                      shared_ptr<configuration::contactgroup> obj);
      void          expand_object(
                      shared_ptr<configuration::contactgroup> obj,
                      configuration::state& s);
      void          modify_object(
                      shared_ptr<configuration::contactgroup> obj);
      void          remove_object(
                      shared_ptr<configuration::contactgroup> obj);
      void          resolve_object(
                      shared_ptr<configuration::contactgroup> obj);

    private:
      void          _resolve_members(
                       shared_ptr<configuration::contactgroup> obj,
                       configuration::state& s);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_CONTACTGROUP_HH
