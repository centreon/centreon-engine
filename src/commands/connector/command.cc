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

#include <QMutexLocker>
#include <QEventLoop>
#include "globals.hh"
#include "macros.hh"
#include "objects.hh"
#include "logging/logger.hh"
#include "commands/result.hh"
#include "commands/connector/request_builder.hh"
#include "commands/connector/execute_response.hh"
#include "commands/connector/version_response.hh"
#include "commands/connector/version_query.hh"
#include "commands/connector/quit_query.hh"
#include "commands/connector/command.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

/**
 *  Default constructor.
 *
 *  @param[in] name         The command name.
 *  @param[in] command_line The command line.
 *  @param[in] process_name The deamon process name.
 */
connector::command::command(QString const& name,
			    QString const& command_line,
			    QString const& process_command)
  : commands::command(name, command_line),
    _process_command(process_command),
    _max_check_for_restart(DEFAULT_MAX_CHECK),
    _nbr_check(0),
    _is_good_version(false),
    _active_timer(false) {
  _req_func.insert(request::version_r, &command::_req_version_r);
  _req_func.insert(request::execute_r, &command::_req_execute_r);
  _req_func.insert(request::quit_r, &command::_req_quit_r);
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
    _process_command = right._process_command;
    _req_func = right._req_func;
    _max_check_for_restart = right._max_check_for_restart;
    _nbr_check = right._nbr_check;
    _is_good_version = right._is_good_version;
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
    throw (engine_error() << _process_command << " not running.");
  }

  unsigned long id = ++_id;

  QDateTime now = QDateTime::currentDateTime();
  QSharedPointer<request> query(new execute_query(id,
						  processed_cmd,
						  now,
						  timeout));
  request_info info = { query, now, timeout, false };

  _queries.insert(id, info);

  QByteArray const& data = query->build();
  _process->write(data.constData(), data.size());

  logger(dbg_commands, basic) << "connector command (id=" << id
			      << ") start '" << processed_cmd << "'.";

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
    throw (engine_error() << _process_command << " not running.");
  }

  unsigned long id = ++_id;

  QDateTime now = QDateTime::currentDateTime();
  QSharedPointer<request> query(new execute_query(id,
						  processed_cmd,
						  now,
						  timeout));
  request_info info = { query, now, timeout, true };
  _queries.insert(id, info);

  QByteArray const& data = query->build();
  _process->write(data.constData(), data.size());

  logger(dbg_commands, basic) << "connector command (id=" << id
			      << ") start '" << processed_cmd << "'.";

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
 * Get the connector process.
 *
 *  @return The connector process.
 */
QString const& connector::command::get_process() const throw() {
  return (_process_command);
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
      it = _queries.erase(it);
      if (info.waiting_result == false) {
	emit command_executed(res);
      }
      else {
	_results.insert(id, res);
      }
      emit _wait_ending();
      continue;
    }
    break;
  }

  for (QHash<unsigned long, request_info>::const_iterator it = _queries.begin(), end = _queries.end();
       it != end;
       ++it) {
    if (it->timeout > 0) {
      unsigned int diff_time = now.toTime_t() - it->start_time.toTime_t();
      _active_timer = true;
      QTimer::singleShot(diff_time > 0 ? diff_time : 1, this, SLOT(_timeout()));
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
      _exit();
      _start();
    }
    catch (std::exception const& e) {
      logger(log_runtime_warning, basic) << e.what();
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
  for (QList<QByteArray>::const_iterator it = responses.begin(), end = responses.end();
       it != end;
       ++it) {
    try {
      QSharedPointer<request> req = req_builder.build(*it);
      QHash<request::e_type, void (command::*)(request*)>::iterator
	it = _req_func.find(req->get_id());
      if (it == _req_func.end()) {
	logger(log_runtime_warning, basic) << "bad request id.";
	continue;
      }

      (this->*(it.value()))(&(*req));
    }
    catch (std::exception const& e) {
      logger(log_runtime_warning, basic) << e.what();
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
  _process = QSharedPointer<QProcess>(new QProcess, &QObject::deleteLater);
  connect(&(*_process), SIGNAL(readyReadStandardOutput()),
	  this, SLOT(_ready_read()));

  _process->closeReadChannel(QProcess::StandardError);
  _process->start(_process_command);
  _process->waitForStarted(-1);
  if (_process->state() == QProcess::NotRunning) {
    throw (engine_error() << "impossible to start '" << _process_command
	   << ", " << _process->errorString() << "'.");
  }

  connect(&(*_process), SIGNAL(stateChanged(QProcess::ProcessState)),
	  this, SLOT(_state_change(QProcess::ProcessState)));

  version_query version;
  QByteArray const& data = version.build();
  _process->write(data.constData(), data.size());

  QEventLoop loop;
  connect(this, SIGNAL(_wait_ending()), &loop, SLOT(quit()));

  locker.unlock();
  loop.exec();
  locker.relock();

  if (_is_good_version == false) {
    throw (engine_error() << "bad process version.");
  }

  for (QHash<unsigned long, request_info>::iterator it = _queries.begin(), end = _queries.end();
       it != end;
       ++it) {
    QByteArray const& data = it->req->build();
    _process->write(data.constData(), data.size());
  }

  logger(log_info_message, basic)
    << "connector start" << _process_command << ".";
}

/**
 *  Quit properly the current process.
 */
void connector::command::_exit() {
  QMutexLocker locker(&_mutex);

  if (_process.isNull() == true) {
    return;
  }

  disconnect(&(*_process), SIGNAL(stateChanged(QProcess::ProcessState)),
	     this, SLOT(_state_change(QProcess::ProcessState)));

  if (_process->state() == QProcess::NotRunning) {
    return;
  }

  QEventLoop loop;
  connect(this, SIGNAL(_process_ending()), &loop, SLOT(quit()));
  QTimer::singleShot(5000, &loop, SLOT(quit()));

  quit_query quit;
  QByteArray const& data = quit.build();
  _process->write(data.constData(), data.size());

  locker.unlock();
  loop.exec();
  locker.relock();

  _process->waitForFinished(1000);
  if (_process->state() == QProcess::Running) {
    _process->kill();
    logger(log_info_message, basic)
      << "connector kill " << _process_command << ".";
  }
  else {
    logger(log_info_message, basic)
      << "connector stop" << _process_command << ".";
  }
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

  if (res->get_major() < ENGINE_MAJOR
      || (res->get_major() == ENGINE_MAJOR
	  && res->get_minor() <= ENGINE_MINOR)) {
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
    << "connector command (id=" << res.get_command_id() << ") finished.";

  if (info.waiting_result == false) {
    emit command_executed(res);
  }
  else {
    QMutexLocker locker(&_mutex);
    _results.insert(res.get_command_id(), res);
  }
  emit _wait_ending();
}
