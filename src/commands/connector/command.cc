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

#include <QMutexLocker>
#include <QEventLoop>
#include "com/centreon/engine/commands/connector/command.hh"
#include "com/centreon/engine/commands/connector/error_response.hh"
#include "com/centreon/engine/commands/connector/execute_response.hh"
#include "com/centreon/engine/commands/connector/quit_query.hh"
#include "com/centreon/engine/commands/connector/request_builder.hh"
#include "com/centreon/engine/commands/connector/version_query.hh"
#include "com/centreon/engine/commands/connector/version_response.hh"
#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/version.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

/**
 *  Default constructor.
 *
 *  @param[in] connector_name The command name.
 *  @param[in] connector_line The deamon process name.
 *  @param[in] command_name   The command name.
 *  @param[in] command_line   The command line.
 */
connector::command::command(QString const& connector_name,
			    QString const& connector_line,
                            QString const& command_name,
			    QString const& command_line)
  : commands::command(command_name, command_line),
    _connector_name(connector_name),
    _connector_line(connector_line),
    _max_check_for_restart(DEFAULT_MAX_CHECK),
    _nbr_check(0),
    _is_good_version(false),
    _active_timer(false),
    _is_exiting(false),
    _state_already_change(false) {
  _req_func.insert(request::version_r, &command::_req_version_r);
  _req_func.insert(request::execute_r, &command::_req_execute_r);
  _req_func.insert(request::quit_r, &command::_req_quit_r);
  _req_func.insert(request::error_r, &command::_req_error_r);
  _start();
}

/**
 *  Default copy constructor
 *
 *  @parame[in] right The copy class.
 */
connector::command::command(command const& right)
  : commands::command(right), _active_timer(false) {
  operator=(right);
}

/**
 *  Default destructor.
 */
connector::command::~command() throw() {
  _exit();
}

/**
 *  Default copy operator.
 *
 *  @param[in] right The copy class.
 *
 *  @return This object.
 */
connector::command& connector::command::operator=(connector::command const& right) {
  if (this != &right) {
    _exit();
    commands::command::operator=(right);
    _connector_name = right._connector_name;
    _connector_line = right._connector_line;
    _req_func = right._req_func;
    _max_check_for_restart = right._max_check_for_restart;
    _nbr_check = right._nbr_check;
    _is_good_version = right._is_good_version;
    _active_timer = right._active_timer;
    _is_exiting = right._is_exiting;
    _state_already_change = right._state_already_change;
    _start();
  }
  return (*this);
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
commands::command* connector::command::clone() const {
  return (new connector::command(*this));
}

/**
 *  Run a command.
 *
 *  @param[in] args    The command arguments.
 *  @param[in] macros  The macros data struct.
 *  @param[in] timeout The command timeout.
 *
 *  @return The command id.
 */
unsigned long connector::command::run(QString const& processed_cmd,
				      nagios_macros const& macros,
				      unsigned int timeout) {
  (void)macros;

  QMutexLocker locker(&_mutex);

  if (_nbr_check > _max_check_for_restart) {
    locker.unlock();
    _exit();
    _start();
    locker.relock();
  }

  if (_process->state() != QProcess::Running) {
    throw (engine_error() << "connector \"" << _name << "\" not running.");
  }

  unsigned long id = get_uniq_id();

  QDateTime now = QDateTime::currentDateTime();
  QSharedPointer<request> query(new execute_query(id,
						  processed_cmd,
						  now,
						  timeout));
  request_info info = { query, now, timeout, false };

  _queries.insert(id, info);

  _process->write(query->build());

  logger(dbg_commands, basic)
    << "connector \"" << _name << "\" start (id="
    << id << ") '" << processed_cmd << "'.";

  if (_active_timer == false && timeout > 0) {
    _active_timer = true;
    QTimer::singleShot(timeout * 1000, this, SLOT(_timeout()));
  }

  return (id);
}

/**
 *  Run a command and wait the result.
 *
 *  @param[in]  args    The command arguments.
 *  @param[in]  macros  The macros data struct.
 *  @param[in]  timeout The command timeout.
 *  @param[out] res     The result of the command.
 */
void connector::command::run(QString const& processed_cmd,
			     nagios_macros const& macros,
			     unsigned int timeout,
			     result& res) {
  (void)macros;

  QMutexLocker locker(&_mutex);

  if (_nbr_check > _max_check_for_restart) {
    locker.unlock();
    _exit();
    _start();
    locker.relock();
  }

  if (_process->state() != QProcess::Running) {
    throw (engine_error() << "connector \"" << _name << "\" not running.");
  }

  unsigned long id = get_uniq_id();

  QDateTime now = QDateTime::currentDateTime();
  QSharedPointer<request> query(new execute_query(id,
						  processed_cmd,
						  now,
						  timeout));
  request_info info = { query, now, timeout, true };
  _queries.insert(id, info);

  _process->write(query->build());

  logger(dbg_commands, basic)
    << "connector \"" << _name << "\" start (id="
    << id << ") '" << processed_cmd << "'.";

  if (_active_timer == false && timeout > 0) {
    _active_timer = true;
    QTimer::singleShot(timeout * 1000, this, SLOT(_timeout()));
  }

  while (true) {
    QEventLoop loop;
    connect(this, SIGNAL(_wait_ending()), &loop, SLOT(quit()));

    locker.unlock();
    loop.exec();
    locker.relock();

    QHash<unsigned long, result>::iterator it = _results.find(id);
    if (it != _results.end()) {
      res = it.value();
      _results.erase(it);
      break;
    }
  }
}

/**
 * Get the connector name.
 *
 *  @return The connector name.
 */
QString const& connector::command::get_connector_name() const throw() {
  return (_connector_name);
}

/**
 * Get the connector line.
 *
 *  @return The connector line.
 */
QString const& connector::command::get_connector_line() const throw() {
  return (_connector_line);
}

/**
 *  Get the max number check before restarting the process.
 *
 *  @return The max check for restart process.
 */
unsigned long connector::command::get_max_check_for_restart() throw() {
  QMutexLocker locker(&_mutex);
  return (_max_check_for_restart);
}

/**
 *  Set the max time running process before restarting the process.
 *
 *  @param[in] value The max time for restart process.
 */
void connector::command::set_max_check_for_restart(unsigned long value) throw() {
  QMutexLocker locker(&_mutex);
  _max_check_for_restart = value;
}

/**
 *  Slot notify when timeout occur.
 */
void connector::command::_timeout() {
  QMutexLocker locker(&_mutex);

  _active_timer = false;
  QDateTime now = QDateTime::currentDateTime();
  QHash<unsigned long, request_info>::iterator it = _queries.begin();
  while (it != _queries.end()) {
    request_info& info = it.value();
    unsigned int diff_time = now.toTime_t() - info.start_time.toTime_t();
    if (diff_time >= info.timeout) {
      unsigned long id = it.key();
      result res(id,
		 "",
		 "(Process Timeout)",
		 info.start_time,
		 now,
		 STATE_CRITICAL,
		 true,
		 true);
      if (info.waiting_result == false) {
	emit command_executed(res);
      }
      else {
	_results.insert(id, res);
      }
      it = _queries.erase(it);
      emit _wait_ending();
      continue;
    }
    break;
  }

  for (QHash<unsigned long, request_info>::const_iterator it
         = _queries.begin(), end = _queries.end();
       it != end;
       ++it) {
    if (it->timeout > 0) {
      unsigned int diff_time = now.toTime_t() - it->start_time.toTime_t();
      _active_timer = true;
      QTimer::singleShot(diff_time > 0 ? diff_time : 1,
                         this,
                         SLOT(_timeout()));
      break;
    }
  }
}

/**
 *  Slot notify when the process state change and restart it if not running.
 */
void connector::command::_state_change(QProcess::ProcessState new_state) {
  if (new_state == QProcess::NotRunning) {
    try {
      if (_state_already_change == false) {
        _state_already_change = true;
        _exit();
        _start();
        _state_already_change = false;
      }
      else
        emit _wait_ending();
    }
    catch (std::exception const& e) {
      logger(log_runtime_warning, basic)
	<< "connector \"" << _name <<  "\" error: " << e.what();
    }
  }
}

/**
 *  Slot notify when process as output data.
 */
void connector::command::_ready_read() {
  QList<QByteArray> responses;

  {
    QMutexLocker locker(&_mutex);

    _read_data += _process->readAllStandardOutput();
    while (_read_data.size() > 0) {
      int pos = _read_data.indexOf(request::cmd_ending());
      if (pos < 0) {
	break;
      }
      responses.push_back(_read_data.left(pos));
      _read_data.remove(0, pos + request::cmd_ending().size());
    }
  }

  request_builder& req_builder = request_builder::instance();
  for (QList<QByteArray>::const_iterator it = responses.begin(),
         end = responses.end();
       it != end;
       ++it) {
    try {
      QSharedPointer<request> req = req_builder.build(*it);
      QHash<request::e_type, void (command::*)(request*)>::iterator
	it = _req_func.find(req->get_id());
      if (it == _req_func.end()) {
	logger(log_runtime_warning, basic)
	  << "connector \"" << _name << "\" bad request id.";
	continue;
      }

      (this->*(it.value()))(&(*req));
    }
    catch (std::exception const& e) {
      logger(log_runtime_warning, basic)
	<< "connector \"" << _name << "\" error: "<< e.what();
    }
  }
}

/**
 *  Restart the processed command.
 */
void connector::command::_start() {
  QMutexLocker locker(&_mutex);
  _nbr_check = 0;

  if (_process.isNull() == false) {
    disconnect(&(*_process), SIGNAL(readyReadStandardOutput()),
  	       this, SLOT(_ready_read()));
    disconnect(&(*_process), SIGNAL(stateChanged(QProcess::ProcessState)),
  	       this, SLOT(_state_change(QProcess::ProcessState)));
  }
  _process = QSharedPointer<basic_process>(new basic_process,
                                           &QObject::deleteLater);
  connect(&(*_process), SIGNAL(readyReadStandardOutput()),
	  this, SLOT(_ready_read()));

  _process->closeReadChannel(QProcess::StandardError);
  _process->start(_connector_line);
  if (_process->waitForStarted(-1) == false) {
    throw (engine_error() << _process->errorString());
  }

  connect(&(*_process), SIGNAL(stateChanged(QProcess::ProcessState)),
	  this, SLOT(_state_change(QProcess::ProcessState)));

  version_query version;
  _process->write(version.build());

  QEventLoop loop;
  connect(this, SIGNAL(_wait_ending()), &loop, SLOT(quit()));

  locker.unlock();
  loop.exec();
  locker.relock();

  if (_is_good_version == false) {
    throw (engine_error() << "bad process version.");
  }

  for (QHash<unsigned long, request_info>::iterator it = _queries.begin(),
         end = _queries.end();
       it != end;
       ++it)
    _process->write(it->req->build());

  logger(log_info_message, basic)
    << "connector \"" << _name << "\" start.";
}

/**
 *  Quit properly the current process.
 */
void connector::command::_exit() {
  QMutexLocker locker(&_mutex);

  if (_process.isNull() == true || _is_exiting == true) {
    return;
  }

  disconnect(&(*_process), SIGNAL(stateChanged(QProcess::ProcessState)),
	     this, SLOT(_state_change(QProcess::ProcessState)));

  if (_process->state() == QProcess::NotRunning) {
    return;
  }

  _is_exiting = true;

  QEventLoop loop;
  connect(this, SIGNAL(_process_ending()), &loop, SLOT(quit()));
  QTimer::singleShot(5000, &loop, SLOT(quit()));

  quit_query quit;
  _process->write(quit.build());

  locker.unlock();
  loop.exec();
  locker.relock();

  _process->waitForFinished(1000);
  if (_process->state() == QProcess::Running) {
    _process->kill();
    logger(log_info_message, basic)
      << "connector \"" << _name << "\" kill.";
  }
  else {
    logger(log_info_message, basic)
      << "connector \"" << _name << "\" stop.";
  }

  _is_exiting = false;
}

/**
 *  Process quit response request.
 *
 *  @param[in] req The request to process.
 */
void connector::command::_req_quit_r(request* req) {
  (void)req;
  emit _process_ending();
}

/**
 *  Process version response request.
 *
 *  @param[in] req The request to process.
 */
void connector::command::_req_version_r(request* req) {
  QMutexLocker locker(&_mutex);
  version_response* res = static_cast<version_response*>(req);

  if (res->get_major() < CENTREON_ENGINE_VERSION_MAJOR
      || (res->get_major() == CENTREON_ENGINE_VERSION_MAJOR
	  && res->get_minor() <= CENTREON_ENGINE_VERSION_MINOR)) {
    _is_good_version = true;
  }
  else {
    _is_good_version = false;
  }
  emit _wait_ending();
}

/**
 *  Process execution response request.
 *
 *  @param[in] req The request to process.
 */
void connector::command::_req_execute_r(request* req) {
  execute_response* response = static_cast<execute_response*>(req);
  request_info info;

  {
    QMutexLocker locker(&_mutex);
    QHash<unsigned long, request_info>::iterator
      it = _queries.find(response->get_command_id());
    if (it == _queries.end()) {
      return;
    }
    info = it.value();
    _queries.erase(it);
    ++_nbr_check;
  }

  bool is_timeout = false;
  if (info.timeout > 0) {
    is_timeout = response->get_end_time().toTime_t()
      - info.start_time.toTime_t() > info.timeout;
  }

  result res(response->get_command_id(),
	     "",
	     "",
	     info.start_time,
	     response->get_end_time(),
	     STATE_CRITICAL,
	     is_timeout,
	     true);

  if (response->get_is_executed() == false) {
    res.set_stderr("(" + response->get_stderr() + ")");
    res.set_is_executed(false);
  }
  else if (is_timeout == true) {
    res.set_stderr("(Process Timeout)");
  }
  else {
    if (response->get_exit_code() < -1 || response->get_exit_code() > 3) {
      res.set_exit_code(STATE_UNKNOWN);
    }
    else {
      res.set_exit_code(response->get_exit_code());
    }

    res.set_stderr(response->get_stderr());
    res.set_stdout(response->get_stdout());
  }

  logger(dbg_commands, basic)
    << "connector \"" << _name << "\" finished (id="
    << res.get_command_id() << ").";

  if (info.waiting_result == false) {
    emit command_executed(res);
  }
  else {
    QMutexLocker locker(&_mutex);
    _results.insert(res.get_command_id(), res);
  }
  emit _wait_ending();
}

/**
 *  Process error response request.
 *
 *  @param[in] req The request to process.
 */
void connector::command::_req_error_r(request* req) {
  error_response* response = static_cast<error_response*>(req);

  switch (response->get_code()) {
  case error_response::info:
    logger(log_info_message, basic)
      << "connector \"" << _connector_name << "\" " << response->get_message();
    break;

  case error_response::warning:
    logger(log_runtime_warning, basic)
      << "connector \"" << _connector_name << "\" " << response->get_message();
    break;

  case error_response::error:
    logger(log_runtime_error, basic)
      << "connector \"" << _connector_name << "\" " << response->get_message();
    _exit();
    break;
  }
}
