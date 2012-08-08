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

#ifndef CCE_SCRIPT_BUILDER_HH
#  define CCE_SCRIPT_BUILDER_HH

#  include <string>
#  include "argument.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "function.hh"

CCE_BEGIN()

namespace               script {
  /**
   *  @class builder builer.hh
   *  @brief Class to builder auto_gen header and source code.
   *
   *  This class create header and source code for the client webservice,
   *  with the soapStub information.
   */
  class                 builder {
  public:
                        builder(
                          std::string const& header_src,
                          std::string const& header_dst,
                          std::string const& source_dst);
                        builder(builder const& right);
                        ~builder() throw ();
    builder&            operator=(builder const& right);
    void                build();
    void                parse();

  private:
    static std::string  _basename(std::string const& path);
    void                _build_header();
    std::string         _build_ostream_struct(
                           std::string const& base,
                           argument const& arg);
    void                _build_source();

    std::list<function> _lst_function;
    std::string         _header_src;
    std::string         _header_dst;
    std::string         _source_dst;
  };
}

CCE_END()

#endif // !CCE_SCRIPT_BUILDER_HH
