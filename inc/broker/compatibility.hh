/*
** Copyright 2011      Merethis
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

#ifndef CCE_BROKER_COMPATIBILITY_HH
# define CCE_BROKER_COMPATIBILITY_HH

# include <QObject>
# include "nebmodules.hh"
# include "broker/handle.hh"

namespace                       com {
  namespace                     centreon {
    namespace                   engine {
      namespace                 broker {
	/**
	 *  @class compatibility compatibility.hh
	 *  @brief Simple compatibility class.
	 *
	 *  Use to keep compatibility with old
	 *  module system API.
	 */
	class                   compatibility : public QObject {
	  Q_OBJECT
	  public:
	  static compatibility& instance();

	public slots:
	  void                  create_module(broker::handle* module);
	  void                  destroy_module(broker::handle* module);
	  void                  name_module(broker::handle* module);
	  void                  author_module(broker::handle* module);
	  void                  copyright_module(broker::handle* module);
	  void                  version_module(broker::handle* module);
	  void                  license_module(broker::handle* module);
	  void                  description_module(broker::handle* module);
	  void                  loaded_module(broker::handle* module);
	  void                  unloaded_module(broker::handle* module);

	private:
	  compatibility();
	  compatibility(compatibility const& right);
	  virtual               ~compatibility() throw();

	  compatibility&        operator=(compatibility const& right);
	};
      }
    }
  }
}

#endif // !CCE_BROKER_COMPATIBILITY_HH
