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

#ifndef CCE_LOGGING_SYSLOG_HH
#  define CCE_LOGGING_SYSLOG_HH

#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/logging/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace          logging {
  /**
   *  @class syslog syslog.hh
   *  @brief Call syslog for all logging message.
   *
   *  Call syscall for all logging message.
   */
  class            syslog : public object {
  public:
                   syslog();
                   ~syslog() throw();
    static syslog& instance();
    void           log(
                     char const* message,
                     unsigned long long type,
                     unsigned int verbosity) throw ();
    void           set_facility(int facility) throw ();

  private:
	           syslog(syslog const& right);
    syslog&        operator=(syslog const& right);
    void           _internal_copy(syslog const& right);

    com::centreon::concurrency::mutex
                   _mutex;
    int            _facility;
  };
}

CCE_END()

#endif // !CCE_LOGGING_SYSLOG_HH
