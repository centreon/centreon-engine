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

#  include <QMutex>
#  include "com/centreon/engine/logging/object.hh"

namespace                com {
  namespace              centreon {
    namespace            engine {
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

	  void           set_facility(int facility) throw ();
	  void           log(
                           char const* message,
                           unsigned long long type,
                           unsigned int verbosity) throw ();

	private:
	                 syslog(syslog const& right);
	  syslog&        operator=(syslog const& right);

	  QMutex         _mutex;
	  int            _facility;
	};
      }
    }
  }
}

#endif // !CCE_LOGGING_SYSLOG_HH
