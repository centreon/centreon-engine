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

#ifndef CCE_LOGGING_FILE_HH
#  define CCE_LOGGING_FILE_HH

#  include <QFile>
#  include <QList>
#  include <QMutex>
#  include <QReadWriteLock>
#  include <QSharedPointer>
#  include <QString>
#  include "com/centreon/engine/logging/object.hh"

namespace                        com {
  namespace                      centreon {
    namespace                    engine {
      namespace                  logging {
	/**
         *  @class file file.hh
         *  @brief Write logging message into file.
         *
         *  Write logging message into file.
         */
	class                    file : public object {
	public:
	                         file(
                                   QString const& file,
                                   unsigned long long size_limit = 0);
	                         file(file const& right);
	                         ~file() throw();
	  file&                  operator=(file const& right);
	  QString                get_file_name() throw();
	  unsigned long long     get_size_limit() const throw();
	  void                   log(
                                   char const* message,
                                   unsigned long long type,
                                   unsigned int verbosity) throw();
          static void            reopen();
	  void                   set_size_limit(unsigned long long size) throw();

	private:
	  QSharedPointer<QMutex> _mutex;
	  QSharedPointer<QFile>  _file;
          static QList<file*>    _files;
          static QReadWriteLock  _rwlock;
	  unsigned long long     _size_limit;
	};
      }
    }
  }
}

#endif // !CCE_LOGGING_FILE_HH
