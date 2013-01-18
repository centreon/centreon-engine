/*
** Copyright 2012-2013 Merethis
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

#ifndef CCE_MOD_WS_CONFIGURATION_SAVE_STATE_HH
#  define CCE_MOD_WS_CONFIGURATION_SAVE_STATE_HH

#  include <sstream>
#  include <string>
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/modules/webservice/namespace.hh"

CCE_MOD_WS_BEGIN()

namespace                configuration {
  namespace              save {
    class                state {
    public:
                         state();
                         state(state const& right);
                         ~state() throw ();
      state&             operator=(state const& right);
      state&             operator<<(engine::configuration::state const& obj);
      void               backup(std::string const& filename) const;
      void               clear();
      std::string        to_string() const;

    private:
      state&             _internal_copy(state const& right);

      std::ostringstream _stream;
    };
  }
}

CCE_MOD_WS_END()

#endif // !CCE_MOD_WS_CONFIGURATION_SAVE_STATE_HH
