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

#ifndef CCE_LOGGING_BROKER_HH
#  define CCE_LOGGING_BROKER_HH

#  include <QMutex>
#  include <QThread>
#  include "com/centreon/engine/logging/object.hh"

namespace         com {
  namespace       centreon {
    namespace     engine {
      namespace   logging {
	/**
	 *  @class broker broker.hh
	 *  @brief Call broker for all logging message.
	 *
	 *  Call broker for all logging message without debug.
	 */
	class      broker : public object {
	public:
	           broker();
	           ~broker() throw();

	  void     log(char const* message,
		       unsigned long long type,
		       unsigned int verbosity) throw();

	private:
	  QMutex   _mutex;
	  QThread* _thread;
	};
      }
    }
  }
}

#endif // !CCE_LOGGING_BROKER_HH
