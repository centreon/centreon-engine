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

#ifndef CCE_CONFIGURATION_APPLIER_LOGGING_HH
#  define CCE_CONFIGURATION_APPLIER_LOGGING_HH

#  include <QSharedPointer>
#  include <QString>
#  include "com/centreon/engine/configuration/applier/base.hh"
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/logging/file.hh"
#  include "com/centreon/engine/logging/object.hh"
#  include "com/centreon/engine/logging/standard.hh"
#  include "com/centreon/engine/logging/syslog.hh"

namespace                      com {
  namespace                    centreon {
    namespace                  engine {
      namespace                configuration {
	namespace              applier {
	/**
	 *  @class logging logging.hh
	 *  @brief Simple configuration applier for logging class.
	 *
	 *  Simple configuration applier for logging class.
	 */
	  class                logging : public base {
	  public:
	    void               apply(state const& config);
            static logging&    instance();
            static void        load();
            static void        unload();

	  private:
	                       logging();
	                       logging(state const& config);
	                       logging(logging& right);
	                       ~logging() throw();
	    logging&           operator=(logging& right);
	    void               _add_stdout();
	    void               _add_stderr();
	    void               _add_syslog();
	    void               _add_log_file(state const& config);
	    void               _add_debug(state const& config);
	    void               _del_syslog();
	    void               _del_log_file();
	    void               _del_debug();
	    void               _del_stdout();
	    void               _del_stderr();

	    QString            _debug_file;
	    unsigned long      _debug_id;
	    unsigned long      _debug_level;
	    unsigned long      _debug_limit;
	    unsigned int       _debug_verbosity;
            static logging*    _instance;
	    QString            _log_file;
	    unsigned long      _log_id;
            unsigned long      _log_limit;
	    unsigned long      _stderr_id;
	    unsigned long      _stdout_id;
	    unsigned long      _syslog_id;
	  };
	}
      }
    }
  }
}

#endif // !CCE_CONFIGURATION_APPLIER_LOGGING_HH
