/*
** Copyright 2011      Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CCS_MODULES_LOADER_HH
# define CCS_MODULES_LOADER_HH

# include <QObject>
# include <QString>
# include <QMultiHash>
# include "modules/handle.hh"

namespace                             com {
  namespace                           centreon {
    namespace                         scheduler {
      namespace                       modules {
	/**
	 *  @class loader loader.hh
	 *  @brief Modules loader.
	 *
	 *  Loader manage all modules.
	 */
	class                         loader : public QObject {
	  Q_OBJECT
	public:
	  static loader&              instance();

	  void                        load();
	  void                        unload();

	  void                        add_module(handle const& module);
	  void                        del_module(handle const& module);

	  QString const&              get_directory() const throw();
	  QMultiHash<QString, handle> const&
	                              get_modules() const throw();
	  QMultiHash<QString, handle>&
	                              get_modules() throw();

	  void                        set_directory(QString const& directory);

	public slots:
	  void                        module_name_changed(QString const& filename,
							  QString const& old_name,
							  QString const& new_name);

	private:
	                              loader();
				      loader(loader const& right);
	  virtual                     ~loader() throw();

	  loader&                     operator=(loader const& right);

	  QString                     _directory;
	  QMultiHash<QString, handle> _modules;
	};
      }
    }
  }
}

#endif // !CCS_MODULES_LOADER_HH
