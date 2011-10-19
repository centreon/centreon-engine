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

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "commands/process_manager.hh"
#include "commands/basic_process.hh"

#include <iostream>

using namespace com::centreon::engine::commands;

/**
 *  Default constructor.
 */
basic_process::basic_process()
  : QObject(),
    _perror(QProcess::UnknownError),
    _state(QProcess::NotRunning),
    _pid(0),
    _status(0),
    _internal_state(not_running) {

  for (unsigned int i(0); i < 2; ++i) {
    _pipe_out[i] = 0;
    _pipe_err[i] = 0;
    _pipe_in[i] = 0;
  }
}

/**
 *  Default destructor.
 */
basic_process::~basic_process() throw() {
  waitForFinished(-1);
  _cleanup();
}

/**
 *  Close the specify read channel.
 *
 *  @param[in] channel Specify the process channels to close.
 */
void basic_process::closeReadChannel(QProcess::ProcessChannel channel) {
  QMutexLocker locker(&_mtx);
  int* fd(channel == QProcess::StandardOutput ? &_pipe_out[0] : &_pipe_err[0]);
  process_manager::instance().remove_fd(*fd);
  if (*fd) {
    close(*fd);
    *fd = 0;
  }
}

/**
 *  Close the write channel.
 */
void basic_process::closeWriteChannel() {
  QMutexLocker locker(&_mtx);
  if (_pipe_in[1]) {
    close(_pipe_in[1]);
    _pipe_in[1] = 0;
  }
}

/**
 *  Get the basic process error.
 *
 *  @return The error id.
 */
QProcess::ProcessError basic_process::error() const {
  QMutexLocker locker(&_mtx);
  return (_perror);
}

/**
 *  Get the exit code number.
 *
 *  @return the exit code.
 */
int basic_process::exitCode() const {
  QMutexLocker locker(&_mtx);
  return (WIFEXITED(_status) ? WEXITSTATUS(_status) : 0);
}

/**
 *  Get the exit status code.
 *
 *  @return The exit status.
 */
QProcess::ExitStatus basic_process::exitStatus() const {
  QMutexLocker locker(&_mtx);
  return (WIFSIGNALED(_status) ? QProcess::CrashExit : QProcess::NormalExit);
}

/**
 *  Get the pid.
 *
 *  @return The pid.
 */
Q_PID basic_process::pid() const {
  QMutexLocker locker(&_mtx);
  return (static_cast<Q_PID>(_pid));
}

/**
 *  Read all data available from the standard output.
 *
 *  @return Returns all data available from the standard output.
 */
QByteArray basic_process::readAllStandardOutput() {
  QMutexLocker locker(&_mtx);
  QByteArray out(_output);
  _output.clear();
  return (out);
}

/**
 *  Read all data available from the standard error.
 *
 *  @return Returns all data available from the standard error.
 */
QByteArray basic_process::readAllStandardError() {
  QMutexLocker locker(&_mtx);
  QByteArray err(_error);
  _error.clear();
  return (err);
}

/**
 *  Get the error string description.
 *
 *  @return The error string description.
 */
QString basic_process::errorString() const {
  return ("");
}

/**
 *  Send data to the process.
 *
 *  @param[in] data The data to write on the process pipe.
 *
 *  @return Returns the number of bytes that were actually written,
 *  or -1 if an error occurred.
 */
int basic_process::write(QByteArray const& data) const throw() {
  QMutexLocker locker(&_mtx);
  return (::write(_pipe_in[1], data.constData(), data.size()));
}

/**
 *  Start program with there own arguments.
 *
 *  @param[in] program   The programe name.
 *  @param[in] arguments The programe arguments.
 */
void basic_process::start(QString const& program, QStringList const& arguments) {
  char** args(new char*[arguments.size() + 2]);
  args[0] = qstrdup(qPrintable(program));

  unsigned int i(1);
  for (QStringList::const_iterator it(arguments.begin()),
         end(arguments.end());
       it != end;
       ++it)
    args[i++] = qstrdup(qPrintable(*it));
  args[i] = NULL;

  _start(args);

  for (unsigned int i(0); args[i]; ++i)
    delete[] args[i];
  delete[] args;
}

/** 
 *  Start program like a command line.
 *
 *  @param[in] program The command line.
 */
void basic_process::start(QString const& program) {
  QStringList args(_split_command_line(program));
  QString prog = args.first();
  args.removeFirst();
  start(prog, args);
}

/**
 *  Get the state of the process.
 *
 *  @return A QProcessProcessState { NotRunning, Starting, Running }.
 */
QProcess::ProcessState basic_process::state() const {
  QMutexLocker locker(&_mtx);
  return (_state);
}

/**
 *  Blocks until the process has started or until msecs
 *  milliseconds have passed.
 *
 *  @param[in] msecs The timeout on milliseconds.
 *
 *  @retrun True if the process started, false if timeout.
 */
bool basic_process::waitForStarted(int msecs) {
  QMutexLocker locker(&_mtx);
  if (_internal_state != not_running)
    return (true);
  _cond.wait(&_mtx, msecs == -1 ? ULONG_MAX : msecs);
  return (_internal_state != not_running);
}

/**
 *  Blocks until the process hos finished or until msecs
 *  milliseconds hove passed.
 *
 *  @param[in] msecs The timeout on milliseconds.
 *
 *  @return True if the process finished, false if timeout.
 */
bool basic_process::waitForFinished(int msecs) {
  QMutexLocker locker(&_mtx);
  if (_internal_state == ended)
    return (true);
  _cond.wait(&_mtx, msecs == -1 ? ULONG_MAX : msecs);
  return (_internal_state == ended);
}

/**
 *  Kill the process with SIGKILL signal.
 */
void basic_process::kill() {
  QMutexLocker locker(&_mtx);
  if (_pid)
    ::kill(_pid, SIGKILL);
}

/**
 *  Kill the process with SIGTERM signal.
 */
void basic_process::terminate() {
  QMutexLocker locker(&_mtx);
  if (_pid)
    ::kill(_pid, SIGTERM);
}

/**
 *  Internal methode to start program.
 *
 *  @param[in] args The arguments of the programe.
 */
void basic_process::_start(char** args) {
  if (_pid)
    return;

  _cleanup();

  if (pipe(_pipe_out) == -1
      || pipe(_pipe_err) == -1
      || pipe(_pipe_in) == -1
      || (_pid = vfork()) == -1) {
    _perror = QProcess::FailedToStart;
    _internal_state = ended;
    emit error(_perror);
    return;
  }

  if (!_pid) {
    close(_pipe_out[0]);
    close(_pipe_err[0]);
    close(_pipe_in[1]);
    if (dup2(_pipe_out[1], 1) != -1
        && dup2(_pipe_err[1], 2) != -1
        && dup2(_pipe_in[0], 0) != -1) {
      close(_pipe_in[0]);
      close(_pipe_out[1]);
      close(_pipe_err[1]);
      execvp(args[0], args);
    }
    std::cout << "error: " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  QMutexLocker locker(&_mtx);

  close(_pipe_out[1]);
  close(_pipe_err[1]);
  close(_pipe_in[0]);

  emit started();
  _state = QProcess::Starting;
  emit stateChanged(_state);
  _state = QProcess::Running;
  emit stateChanged(_state);
  _internal_state = running;

  process_manager::instance().add_process(this);
}

/**
 *  This methode is call by the process manager
 *  when a basic process was finish to update
 *  status and emit all qt signal.
 */
void basic_process::_finish() throw() {
  QMutexLocker locker(&_mtx);

  _pid = 0;
  if (_pipe_out[0] || _pipe_err[0])
    return;

  emit finished(WIFEXITED(_status) ? WEXITSTATUS(_status) : 0,
                WIFSIGNALED(_status) ? QProcess::CrashExit : QProcess::NormalExit);
  _state = QProcess::NotRunning;
  emit stateChanged(_state);

  if (WIFSIGNALED(_status) == QProcess::CrashExit) {
    _perror = QProcess::Crashed;
    emit error(_perror);
  }

  _internal_state = ended;

  _cond.wakeOne();
}

/**
 *  Cleanup all internal data of basic process.
 */
void basic_process::_cleanup() throw() {
  _output.clear();
  _error.clear();
  _perror = QProcess::UnknownError;
  _state = QProcess::NotRunning;
  _pid = 0;
  _status = 0;
  _internal_state = not_running;

  for (unsigned int i(0); i < 2; ++i) {
    if (_pipe_out[i]) {
      close(_pipe_out[i]);
      _pipe_out[i] = 0;
    }
    if (_pipe_err[i]) {
      close(_pipe_err[i]);
      _pipe_err[i] = 0;
    }
    if (_pipe_in[i]) {
      close(_pipe_in[i]);
      _pipe_in[i] = 0;
    }
  }
}

/**
 *  This methode is call by the process manager
 *  when the standard output or the standard error
 *  channel has some data to read.
 *
 *  @param[in] fd The pipe fd to read.
 */
void basic_process::_read_fd(int fd) {
  QMutexLocker locker(&_mtx);
  if (fd == _pipe_out[0]) {
    char buf[1024];
    int ret = read(fd, buf, sizeof(buf));
    if (ret > 0) {
      _output.append(buf, ret);
      emit readyReadStandardOutput();
    }
  }
  else if (fd == _pipe_err[0]) {
    char buf[1024];
    int ret = read(fd, buf, sizeof(buf));
    if (ret > 0) {
      _error.append(buf, ret);
      emit readyReadStandardError();
    }
  }
}

/**
 *  This methode is call by the process manager
 *  when one of pipe was close by the child.
 *
 *  @param[in] fd The pipe fd to close.
 */
void basic_process::_close_fd(int fd) {
  QMutexLocker locker(&_mtx);

  if (fd == _pipe_out[0]) {
    close(_pipe_out[0]);
    _pipe_out[0] = 0;
  }
  else if (fd == _pipe_err[0]) {
    close(_pipe_err[0]);
    _pipe_err[0] = 0;
  }
}

/**
 *  Split a command line in a array of arguments.
 *
 *  @param[in] command_line The command line.
 *
 *  @return The array of arguments.
 */
QStringList basic_process::_split_command_line(QString const& command_line) {
  QStringList args;
  QString tmp;
  int count(0);
  bool in(false);

  for (int i = 0, end = command_line.size(); i < end; ++i) {
    if (command_line.at(i) == QLatin1Char('"')) {
      ++count;
      if (count == 3) {
        count = 0;
        tmp += command_line.at(i);
      }
      continue;
    }
    if (count) {
      if (count == 1)
        in = !in;
      count = 0;
    }
    if (!in && command_line.at(i).isSpace()) {
      if (!tmp.isEmpty()) {
        args += tmp;
        tmp.clear();
      }
    }
    else
      tmp += command_line.at(i);
  }
  if (!tmp.isEmpty())
    args += tmp;
  return (args);
}
