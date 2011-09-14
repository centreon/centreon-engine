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
# include <QString>
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

          QSharedPointer<handle> add_module(QString const& filename = "",
                                            QString const& args = "");
          void                   del_module(QSharedPointer<handle> const& module);

          QString const&         get_directory() const throw();
          QList<QSharedPointer<handle> >
                                 get_modules() const throw();

          void                   set_directory(QString const& directory);

         public slots:
          void                   module_name_changed(QString const& old_name,
                                                     QString const& new_name);

         private:
                                 loader();
                                 loader(loader const& right);
          virtual                ~loader() throw();

          loader&                operator=(loader const& right);

          QString                _directory;
          QMultiHash<QString, QSharedPointer<handle> > _modules;
        };
      }
    }
  }
}

#endif // !CCE_MODULES_LOADER_HH
