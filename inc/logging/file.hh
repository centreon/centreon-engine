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

#ifndef CCE_LOGGING_FILE_HH
# define CCE_LOGGING_FILE_HH

# include <QFile>
# include <QString>
# include <QList>

namespace                     com {
  namespace                   centreon {
    namespace                 engine {
      namespace               logging {
	/**
         *  @class file file.hh
         *  @brief Write logging message into file.
         *
         *  Write logging message into file.
         */
	class                 file {
	public:
	                      file(char const* file,
				   char const* archive_path = "./",
				   long long size_limit = 0);
	                      ~file() throw();

	  static void         rotate_all();
	  void                rotate();

	  void                log(char const* message,
				  unsigned long long type,
				  unsigned int verbosity) throw();

	  void                set_archive_path(char const* path) throw();
	  QString const&      get_archive_path() const throw();

	  void                set_size_limit(long long size) throw();
	  long long           get_size_limit() const throw();

	private:
	                      file(file const& right);
	  file&               operator=(file const& right);

	  QFile               _file;
	  QString             _archive_path;
	  long long           _size_limit;

	  static QList<file*> _files;
	};
      }
    }
  }
}

#endif // !CCE_LOGGING_FILE_HH
