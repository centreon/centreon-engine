/*
** Copyright 2011 Merethis
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

#ifndef CCE_MOD_WS_WEBSERVICE_HH
#  define CCE_MOD_WS_WEBSERVICE_HH

#  include <map>
#  include <string>
#  include "auto_gen.hh"
#  include "com/centreon/engine/modules/webservice/namespace.hh"
#  include "soapH.h"

CCE_MOD_WS_BEGIN()

/**
 *  @class webservice webservice.hh
 *  @brief Base webservice class.
 *
 *  Webservice class provide system to execute commands
 *  over a network with a Simple Object Access Protocol.
 */
class                webservice {
public:
                     webservice(
                       bool ssl_enable = false,
                       std::string const& keyfile = "",
                       std::string const& password = "",
                       std::string const& cacert = "");
                     webservice(webservice const& right);
                     ~webservice();
  webservice&        operator=(webservice const& right);
  bool               execute(
                       std::string const& function,
                       std::map<std::string, std::string>& args);
  std::string const& get_action() const throw ();
  std::string const& get_end_point() const throw ();
  bool               is_ssl_enable() const throw ();
  void               set_action(std::string const& action);
  void               set_end_point(std::string const& end_point);

private:
#ifdef WITH_OPENSSL
  static void        _sigpipe_handle(int x);
#endif // !WITH_OPENSSL

  std::string        _action;
  std::string        _end_point;
  auto_gen&          _gen;
  soap               _soap_ctx;
  bool               _ssl_enable;
};

CCE_MOD_WS_END()

#endif // !CCE_MOD_WS_WEBSERVICE_HH
