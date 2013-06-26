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

#ifndef CCE_CONFIGURATION_APPLIER_BASE_HH
#  define CCE_CONFIGURATION_APPLIER_BASE_HH

#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace          configuration {
  // Forward declaration.
  class            state;

  namespace        applier {
    /**
     *  @class base base.hh
     *  @brief Parent class of all applier objects.
     *
     *  Parent class of all applier objects.
     */
    class          base {
    public:
                   base();
                   base(base const& right);
      virtual      ~base() throw ();
      base&        operator=(base const& right);
      virtual void apply(state& new_cfg) = 0;
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_BASE_HH
