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

#ifndef CCE_COMMANDS_RESULT_HH
# define CCE_COMMANDS_RESULT_HH

# include <QString>
# include <QTime>
# include <sys/time.h>

namespace                com {
  namespace              centreon {
    namespace            engine {
      namespace          commands {
	/**
	 *  @class result result.hh
	 *  @brief Result contain the result of execution process.
	 *
	 *  Result contain the result of execution process (output, retvalue,
	 *  execution time).
	 */
	class            result {
	public:
	                 result(unsigned long cmd_id = 0,
				QString const& stdout = "",
				QString const& stderr = "",
				timeval const& start_time = timeval(),
				timeval const& end_time = timeval(),
				int retval = 0,
				bool is_timeout = false,
				bool exit_ok = true);
	                 result(result const& right);
	                 ~result() throw();

	  result&        operator=(result const& right);
	  bool           operator==(result const& right) const throw();
	  bool           operator!=(result const& right) const throw();

	  unsigned long  get_cmd_id() const throw();
	  int            get_retval() const throw();
	  double         get_execution_time() const throw();
	  timeval const& get_start_time() const throw();
	  timeval const& get_end_time() const throw();
	  QString const& get_stdout() const throw();
	  QString const& get_stderr() const throw();
	  bool           get_exited_ok() const throw();
	  bool           get_is_timeout() const throw();

	  void           set_cmd_id(unsigned long id) throw();
	  void           set_retval(int retval) throw();
	  void           set_start_time(timeval const& tv) throw();
	  void           set_end_time(timeval const& tv) throw();
	  void           set_stdout(QString const& str);
	  void           set_stderr(QString const& str);
	  void           set_exited_ok(bool value) throw();
	  void           set_is_timeout(bool value) throw();

	private:
	  QString        _stdout;
	  QString        _stderr;
	  timeval        _start_time;
	  timeval        _end_time;
	  unsigned long  _cmd_id;
	  int            _retval;
	  bool           _is_timeout;
	  bool           _exited_ok;
	};
      }
    }
  }
}

#endif // !CCE_COMMANDS_RESULT_HH
