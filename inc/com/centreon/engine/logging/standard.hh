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

#ifndef CCE_LOGGING_STANDARD_HH
#  define CCE_LOGGING_STANDARD_HH

#  include <QMutex>
#  include <stdio.h>
#  include "com/centreon/engine/logging/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace     logging {
  /**
   *  @class standard standard.hh
   *  @brief Call console for all logging message.
   *
   *  Call console for all logging message. You can select stdout or
   *  stderr.
   */
  class       standard : public object {
  public:
              standard(bool is_stdout = true);
              standard(standard const& right);
              ~standard() throw ();
    standard& operator=(standard const& right);
    void      log(
                char const* message,
                unsigned long long type,
                unsigned int verbosity) throw ();

  private:
    void      _internal_copy(standard const& right);

    FILE*     _file;
    QMutex    _mutex;
  };
}

CCE_END()

#endif // !CCE_LOGGING_STANDARD_HH
