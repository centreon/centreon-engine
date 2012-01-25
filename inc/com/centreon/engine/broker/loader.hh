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

#ifndef CCE_MODULES_LOADER_HH
# define CCE_MODULES_LOADER_HH

# include <QObject>
# include <string>
# include <QMultiHash>
# include <QSharedPointer>
# include "broker/handle.hh"

namespace                        com {
  namespace                      centreon {
    namespace                    engine {
      namespace                  broker {
        /**
         *  @class loader loader.hh
         *  @brief Modules loader.
         *
         *  Loader manage all modules.
         */
        class                    loader : public QObject {
          Q_OBJECT
         public:
          static loader&         instance();
	  static void            cleanup();

          unsigned int           load();
          void                   unload();

          QSharedPointer<handle> add_module(std::string const& filename = "",
                                            std::string const& args = "");
          void                   del_module(QSharedPointer<handle> const& module);

          std::string const&         get_directory() const throw();
          QList<QSharedPointer<handle> >
                                 get_modules() const throw();

          void                   set_directory(std::string const& directory);

         public slots:
          void                   module_name_changed(std::string const& old_name,
                                                     std::string const& new_name);

         private:
                                 loader();
                                 loader(loader const& right);
          virtual                ~loader() throw();

          loader&                operator=(loader const& right);

          std::string                _directory;
          QMultiHash<std::string, QSharedPointer<handle> > _modules;
          static loader*         _instance;
        };
      }
    }
  }
}

#endif // !CCE_MODULES_LOADER_HH
