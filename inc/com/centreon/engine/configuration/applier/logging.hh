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

#ifndef CCE_CONFIGURATION_APPLIER_LOGGING_HH
# define CCE_CONFIGURATION_APPLIER_LOGGING_HH

# include <QSharedPointer>
# include <string>

# include "logging/object.hh"
# include "logging/file.hh"
# include "logging/syslog.hh"
# include "logging/standard.hh"
# include "configuration/state.hh"
# include "configuration/applier/base.hh"

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
	                       logging();
	                       logging(state const& config);
	                       logging(logging& right);
	                       ~logging() throw();

	    logging&           operator=(logging& right);

	    void               apply(state const& config);

	  private:
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

	    std::string            _log_file;
	    std::string            _log_archive_path;
	    std::string            _debug_file;
	    unsigned long      _debug_limit;
	    unsigned long      _debug_level;
	    unsigned int       _debug_verbosity;
	    unsigned long      _stdout_id;
	    unsigned long      _stderr_id;
	    unsigned long      _syslog_id;
	    unsigned long      _file_id;
	    unsigned long      _debug_id;
	  };
	}
      }
    }
  }
}

#endif // !CCE_CONFIGURATION_APPLIER_LOGGING_HH
