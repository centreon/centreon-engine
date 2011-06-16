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

#include <QDebug>
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
    _max_check_for_restart(-1),
    _max_time_for_restart(-1),
    _is_good_version(false),
    _active_timer(false) {
  connect(this, SIGNAL(_wait_ending()), &_loop, SLOT(quit()));
  connect(&_process, SIGNAL(readyReadStandardError()),
	  this, SLOT(_ready_read()));
  connect(&_process, SIGNAL(readyReadStandardOutput()),
	  this, SLOT(_ready_read()));

  _req_func.insert(request::version_r, &command::_req_version_r);
  _req_func.insert(request::execute_r, &command::_req_execute_r);
  _req_func.insert(request::quit_r, &command::_req_quit_r);
  _start();
}

/**
 *  Default copy constructor
 *
 *  @param[in] right The copy class.
 */
connector::command::command(command const& right)
  : commands::command(right), _active_timer(false) {
  connect(this, SIGNAL(_wait_ending()), &_loop, SLOT(quit()));
  connect(&_process, SIGNAL(readyReadStandardError()),
	  this, SLOT(_ready_read()));
  connect(&_process, SIGNAL(readyReadStandardOutput()),
	  this, SLOT(_ready_read()));
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
    _max_time_for_restart = right._max_time_for_restart;
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
				      int timeout) {
  (void)macros;
  timeout = (timeout > 0 ? timeout * 1000 : -1);

  _mutex.lock();
  if (_process.state() != QProcess::Running) {
    _mutex.unlock();
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
  _process.write(data.constData(), data.size());

  if (_active_timer == false && timeout > 0) {
    _active_timer = true;
    QTimer::singleShot(timeout, this, SLOT(_timeout()));
  }
  _mutex.unlock();

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
			     int timeout,
			     result& res) {
  (void)macros;
  timeout = (timeout > 0 ? timeout * 1000 : -1);

  _mutex.lock();
  if (_process.state() != QProcess::Running) {
    _mutex.unlock();
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
  write(1, data.constData(), data.size());
  _process.write(data.constData(), data.size());

  if (_active_timer == false && timeout > 0) {
    _active_timer = true;
    QTimer::singleShot(timeout, this, SLOT(_timeout()));
  }
  _mutex.unlock();

  _loop.exec();

  _mutex.lock();
  QHash<unsigned long, result>::iterator it = _results.find(id);
  res = it.value();
  _results.erase(it);
  _mutex.unlock();
}

QString const& connector::command::get_process() const throw() {
  return (_process_command);
}

/**
 *  Get the max number check before restarting the process.
 *
 *  @return The max check for restart process.
 */
int connector::command::get_max_check_for_restart() const throw() {
  return (_max_check_for_restart);
}

/**
 *  Set the max number check before
 *
 *  @param[in] value The max check for restart process.
 */
void connector::command::set_max_check_for_restart(int value) throw() {
  _max_check_for_restart = (value > 0 ? value : -1);
}

/**
 *  Get the max time running process before restarting the process.
 *
 *  @return The max time for restart process.
 */
int connector::command::get_max_time_for_restart() const throw() {
  return (_max_time_for_restart);
}

/**
 *  Set the max time running process before restarting the process.
 *
 *  @param[in] value The max time for restart process.
 */
void connector::command::set_max_time_for_restart(int value) throw() {
  _max_time_for_restart = (value > 0 ? value : -1);
}

/**
 *  Slot notify when timeout occur.
 */
void connector::command::_timeout() {
  _mutex.lock();
  _active_timer = false;
  QDateTime now = QDateTime::currentDateTime();
  QHash<unsigned long, request_info>::iterator it = _queries.begin();
  while (it != _queries.end()) {
    request_info& info = it.value();
    qint64 diff_time = now.toMSecsSinceEpoch() - info.start_time.toMSecsSinceEpoch();
    if (diff_time > info.timeout) {
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
      qint64 diff_time = now.toMSecsSinceEpoch() - it->start_time.toMSecsSinceEpoch();
      _active_timer = true;
      QTimer::singleShot(diff_time > 0 ? diff_time : 1, this, SLOT(_timeout()));
      break;
    }
  }
  _mutex.unlock();
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
  static QByteArray data;
  QList<QByteArray> responses;

  _mutex.lock();
  data += _process.readAllStandardOutput();
  while (data.size() > 0) {
    int pos = data.indexOf(request::cmd_ending());
    if (pos < 0) {
      break;
    }
    responses.push_back(data.left(pos));
    data.remove(0, pos + request::cmd_ending().size());
  }
  _mutex.unlock();

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
  _mutex.lock();
  // _process.closeReadChannel(QProcess::StandardError);
  _process.start(_process_command);
  _process.waitForStarted(-1);
  if (_process.state() == QProcess::NotRunning) {
    _mutex.unlock();
    throw (engine_error() << "impossible to start '" << _process_command
	   << ", " << _process.errorString() << "'.");
  }

  connect(&_process, SIGNAL(stateChanged(QProcess::ProcessState)),
	  this, SLOT(_state_change(QProcess::ProcessState)));

  version_query version;
  QByteArray const& data = version.build();
  _process.write(data.constData(), data.size());
  _mutex.unlock();

  _loop.exec();

  _mutex.lock();
  if (_is_good_version == false) {
    _mutex.unlock();
    throw (engine_error() << "bad process version.");
  }

  for (QHash<unsigned long, request_info>::iterator it = _queries.begin(), end = _queries.end();
       it != end;
       ++it) {
    QByteArray const& data = it->req->build();
    _process.write(data.constData(), data.size());
  }
  _mutex.unlock();
}

/**
 *  Quit properly the current process.
 */
void connector::command::_exit() {
  _mutex.lock();
  disconnect(&_process, SIGNAL(stateChanged(QProcess::ProcessState)),
	     this, SLOT(_state_change(QProcess::ProcessState)));

  if (_process.state() == QProcess::NotRunning) {
    _mutex.unlock();
    return;
  }

  QEventLoop loop;
  connect(this, SIGNAL(_process_ending()), &loop, SLOT(quit()));
  QTimer::singleShot(5000, &loop, SLOT(quit()));

  quit_query quit;
  QByteArray const& data = quit.build();
  _process.write(data.constData(), data.size());
  _mutex.unlock();

  loop.exec();

  _mutex.lock();
  _process.waitForFinished(1000);
  if (_process.state() == QProcess::Running) {
    _process.kill();
  }
  _mutex.unlock();
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
  version_response* res = static_cast<version_response*>(req);

  _mutex.lock();
  if (res->get_major() < ENGINE_MAJOR
      || (res->get_major() == ENGINE_MAJOR
	  && res->get_minor() <= ENGINE_MINOR)) {
    _is_good_version = true;
  }
  else {
    _is_good_version = false;
  }
  _mutex.unlock();
  emit _wait_ending();
}

/**
 *  Process execution response request.
 *
 *  @param[in] req The request to process.
 */
void connector::command::_req_execute_r(request* req) {
  execute_response* response = static_cast<execute_response*>(req);

  _mutex.lock();
  QHash<unsigned long, request_info>::iterator it = _queries.find(response->get_command_id());
  if (it == _queries.end()) {
    _mutex.unlock();
    return;
  }
  request_info info = it.value();
  _queries.erase(it);
  _mutex.unlock();

  bool is_timeout = false;
  if (info.timeout > 0) {
    is_timeout = response->get_end_time().toMSecsSinceEpoch()
      - info.start_time.toMSecsSinceEpoch() > info.timeout;
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

  if (info.waiting_result == false) {
    emit command_executed(res);
  }
  else {
    _mutex.lock();
    _results.insert(res.get_command_id(), res);
    _mutex.unlock();
  }
  emit _wait_ending();
}
