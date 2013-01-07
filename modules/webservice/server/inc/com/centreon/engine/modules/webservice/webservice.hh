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

#ifndef CCE_MOD_WS_WERBSERVICE_HH
#  define CCE_MOD_WS_WERBSERVICE_HH

#  include "soapH.h"
#  include "com/centreon/concurrency/runnable.hh"
#  include "com/centreon/concurrency/thread.hh"
#  include "com/centreon/engine/modules/webservice/configuration.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                   modules {
  namespace                 webservice {
    /**
     *  @class webservice webservice.hh
     *  @brief Base webservice class.
     *
     *  Webservice class provide system to execute commands
     *  over a network with a Simple Object Access Protocol.
     */
    class                   webservice : public com::centreon::concurrency::thread {
    public:
                            webservice(configuration const* config);
                            ~webservice() throw();

    protected:
      void                  _run();

    private:
      class                 query : public com::centreon::concurrency::runnable {
      public:
                            query(soap* s = NULL);
                            ~query() throw ();
        void                run();

      private:
        soap*               _soap;
      };

                            webservice(webservice const& right);
      webservice&           operator=(webservice const& right);
      void                  _init();
      void                  _internal_copy(webservice const& right);
      static void           _sigpipe_handle(int x);

      configuration const*  _config;
      volatile bool         _is_end;
      soap                  _soap_ctx;
    };
  }
}

CCE_END()

#endif // !CCE_MOD_WS_WERBSERVICE_HH
