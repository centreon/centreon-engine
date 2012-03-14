/*
** Copyright 2011-2012 Merethis
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

#ifndef CCE_BROKER_HANDLE_HH
# define CCE_BROKER_HANDLE_HH

#  include <QLibrary>
#  include <QObject>
#  include <QSharedPointer>
#  include <QString>

namespace                          com {
  namespace                        centreon {
    namespace                      engine {
      namespace                    broker {
        /**
         *  @class handle handle.hh
         *  @brief Handle contains module informations.
         *
         *  Handle is a module object, contains information
         *  about module, start and stop module.
         */
        class                      handle : public QObject {
          Q_OBJECT

        public:
                                   handle(
                                     QString const& filename = "",
                                     QString const& args = "");
                                   handle(handle const& right);
          virtual                  ~handle() throw ();
          handle&                  operator=(handle const& rigth);
          bool                     operator==(
                                     handle const& right) const throw ();
          bool                     operator!=(
                                     handle const& right) const throw ();
          void                     close();
          QLibrary*                get_handle() const throw ();
          QString const&           get_author() const throw ();
          QString const&           get_copyright() const throw ();
          QString const&           get_description() const throw ();
          QString const&           get_filename() const throw ();
          QString const&           get_license() const throw ();
          QString const&           get_name() const throw ();
          QString const&           get_version() const throw ();
          QString const&           get_args() const throw ();
          bool                     is_loaded();
          void                     open();
          void                     open(
                                     QString const& filename,
                                     QString const& args);
          void                     set_author(QString const& author);
          void                     set_copyright(
                                     QString const& copyright);
          void                     set_description(
                                     QString const& description);
          void                     set_license(QString const& license);
          void                     set_name(QString const& name);
          void                     set_version(QString const& version);

        signals:
          void                     event_author(broker::handle* module);
          void                     event_copyright(
                                     broker::handle* module);
          void                     event_create(broker::handle* module);
          void                     event_description(
                                     broker::handle* module);
          void                     event_destroy(
                                     broker::handle* module);
          void                     event_license(
                                     broker::handle* module);
          void                     event_loaded(broker::handle* module);
          void                     event_name(broker::handle* module);
          void                     event_unloaded(
                                     broker::handle* module);
          void                     event_version(
                                     broker::handle* module);
          void                     name_changed(
                                     QString const& old_name,
                                     QString const& new_name);

        private:
          void                     _internal_copy(handle const& right);

          QString                  _args;
          QString                  _author;
          QString                  _copyright;
          QString                  _description;
          QString                  _filename;
          QSharedPointer<QLibrary> _handle;
          QString                  _license;
          QString                  _name;
          QString                  _version;
        };
      }
    }
  }
}

#endif // !CCE_BROKER_HANDLE_HH
