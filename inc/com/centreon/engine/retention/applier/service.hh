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

#ifndef CCE_RETENTION_APPLIER_SERVICE_HH
#  define CCE_RETENTION_APPLIER_SERVICE_HH

#  include <list>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/retention/service.hh"

// forward declaration.
struct service_struct;

CCE_BEGIN()

namespace   retention {
  namespace applier {
    class   service {
    public:
            service();
            ~service() throw ();
      void  apply(std::list<retention::service> const& lst);

    private:
      void  _update(retention::service const& state, service_struct& obj);
    };
  }
}

CCE_END()

#endif // !CCE_RETENTION_APPLIER_SERVICE_HH
