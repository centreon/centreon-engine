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

#  include <list>
#  include <QFile>
#  include <QReadWriteLock>
#  include <string>
#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/logging/object.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace                   logging {
  /**
   *  @class file file.hh
   *  @brief Write logging message into file.
   *
   *  Write logging message into file.
   */
  class                     file : public object {
  public:
                            file(
                              std::string const& file,
                              unsigned long long size_limit = 0);
                            file(file const& right);
                            ~file() throw ();
    file&                   operator=(file const& right);
    std::string             get_file_name() throw ();
    unsigned long long      get_size_limit() const throw ();
    void                    log(
                              char const* message,
                              unsigned long long type,
                              unsigned int verbosity) throw ();
    static void             reopen();
    void                    set_size_limit(
                              unsigned long long size) throw ();

  private:
    com::centreon::shared_ptr<com::centreon::concurrency::mutex>
                            _mutex;
    com::centreon::shared_ptr<QFile>
                            _file;
    static std::list<file*> _files;
    static QReadWriteLock   _rwlock;
    unsigned long long      _size_limit;
  };
}

CCE_END()

#endif // !CCE_LOGGING_FILE_HH
