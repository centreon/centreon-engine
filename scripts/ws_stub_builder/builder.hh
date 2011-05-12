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
# define CCE_SCRIPT_BUILDER_HH

# include <QString>
# include "argument.hh"
# include "function.hh"

namespace                 com {
  namespace               centreon {
    namespace             engine {
      namespace           script {
	/**
	 *  @class builder builer.hh
	 *  @brief Class to builder auto_gen header and source code.
	 *
	 *  This class create header and source code for the client webservice,
	 *  with the soapStub information.
	 */
	class             builder {
	public:
	                  builder(QString const& header_src,
				  QString const& header_dst,
				  QString const& source_dst);
	                  builder(builder const& right);
	                  ~builder() throw();

	  builder&        operator=(builder const& right);

	  void            parse();
	  void            build();

	private:
	  void            _build_header();
	  void            _build_source();

	  QString         _build_ostream_struct(QString const& base,
						argument const& arg);

	  QList<function> _lst_function;
	  QString         _header_src;
	  QString         _header_dst;
	  QString         _source_dst;
	};
      }
    }
  }
}

#endif // !CCE_SCRIPT_BUILDER_HH
