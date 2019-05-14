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

#ifndef CCE_RETENTION_APPLIER_HOST_HH
#  define CCE_RETENTION_APPLIER_HOST_HH

#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/retention/host.hh"

// Forward declaration.
CCE_BEGIN()

// Forward declaration.
namespace configuration {
  class   state;
}

namespace   retention {
  namespace applier {
    class   host {
    public:
      void  apply(
              configuration::state const& config,
              list_host const& lst,
              bool scheduling_info_is_ok);

    private:
      void  _update(
              configuration::state const& config,
              retention::host const& state,
              engine::host& obj,
              bool scheduling_info_is_ok);
    };
  }
}

CCE_END()

#endif // !CCE_RETENTION_APPLIER_HOST_HH
