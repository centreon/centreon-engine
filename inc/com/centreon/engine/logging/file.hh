/*
** Copyright 2011-2013 Merethis
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

#  include <string>
#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/logging/object.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/io/file_stream.hh"

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
    std::string const&      get_file_name() const throw ();
    unsigned long long      get_size_limit() const;
    void                    log(
                              char const* message,
                              unsigned long long type,
                              unsigned int verbosity) throw ();
    static void             reopen();
    void                    set_size_limit(
                              unsigned long long size);

  private:
    void                    _close();
    void                    _internal_copy(file const& right);
    void                    _open();

    io::file_stream         _file;
    std::string             _filename;
    mutable concurrency::mutex
                            _mutex;
    unsigned long long      _size_limit;
    unsigned long long      _written;
  };
}

CCE_END()

#endif // !CCE_LOGGING_FILE_HH
