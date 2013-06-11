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

#ifndef CCE_RETENTION_PARSER_HH
#  define CCE_RETENTION_PARSER_HH

#  include <string>
// #  include "com/centreon/engine/retention/contact.hh"
#  include "com/centreon/engine/retention/host.hh"
// #  include "com/centreon/engine/retention/hostcomment.hh"
// #  include "com/centreon/engine/retention/hostdowntime.hh"
// #  include "com/centreon/engine/retention/service.hh"
// #  include "com/centreon/engine/retention/servicecomment.hh"
// #  include "com/centreon/engine/retention/servicedowntime.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace              retention {
  class                parser {
  public:
                       parser();
                       ~parser() throw ();
    void               parse(std::string const& path);

  private:
                       parser(parser const& right);
    parser&            operator=(parser const& right);
  };
}

CCE_END()

#endif // !CCE_RETENTION_PARSER_HH
