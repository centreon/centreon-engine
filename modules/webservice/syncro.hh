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

#ifndef CCE_MOD_WS_SYNCRO_HH
# define CCE_MOD_WS_SYNCRO_HH

# include <QWaitCondition>
# include <QMutex>

namespace                com {
  namespace              centreon {
    namespace            engine {
      namespace          modules {
	/**
	 *  @class syncro syncro.hh
	 *  @brief Synchronize thread and callback.
	 *
	 *  Syncro class provide system to synchronize thread
	 *  and callback processing.
	 */
	class            syncro {
	public:
	  static syncro& instance();

	  void           wakeup_worker();
	  void           waiting_callback();
	  void           worker_finish();

	private:
	                 syncro();
	                 syncro(syncro const& right);
	                 ~syncro();

	  syncro&        operator=(syncro const& right);

	  QWaitCondition _condition;
	  QMutex         _mutex;
	  unsigned int   _thread_count;
	  bool           _can_run;
	};
      }
    }
  }
}

#endif // !CCE_MOD_WS_SYNCRO_HH
