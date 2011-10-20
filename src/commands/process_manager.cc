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

#include <QMutexLocker>
#include <QMetaType>
#include <sys/wait.h>
#include <errno.h>
#include "logging/logger.hh"
#include "commands/process_manager.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

process_manager* process_manager::_instance = NULL;

/**
 *  Default constructor.
 */
process_manager::process_manager()
  : QThread(),
    _fds(NULL),
    _fds_size(0),
    _is_modify(false),
    _quit(false) {
  qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
  qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
  start();
}

/**
 *  Default destructor.
 */
process_manager::~process_manager() throw() {
  _quit = true;
  wait();
  delete[] _fds;
}

/**
 *  Get instance of process_manager singleton.
 *
 *  @return An instance on the process_manager.
 */
process_manager& process_manager::instance() {
  if (_instance == NULL)
    _instance = new process_manager();
  return (*_instance);
}

/**
 *  Cleanup the engine singleton.
 */
void process_manager::cleanup() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Add a new basic process in the process manager. (WARNING:
 *  you need to lock the process manager before call this
 *  function and unlock it after).
 *
 *  @param[in] p The basic_process to add in the process_manager.
 */
void process_manager::add_process(basic_process* p) {
  if (p) {
    _processes_by_fd.insert(p->_pipe_out[0], p);
    _processes_by_fd.insert(p->_pipe_err[0], p);
    _processes_by_pid.insert(p->_pid, p);
    _is_modify = true;
  }
}

/**
 *  Remove a basic process (WARNING: you need to lock the process
 *  manager before call this function and unlock it after).
 *
 *  @param[in] p The basic_process to remove in the process_manager.
 */
void process_manager::remove_process(basic_process* p) {
  if (p) {
    _processes_by_fd.remove(p->_pipe_out[0]);
    _processes_by_fd.remove(p->_pipe_err[0]);
    _processes_by_pid.remove(p->_pid);
    _is_modify = true;
  }
}

/**
 *  Lock the process manager.
 */
void process_manager::lock() {
  _mtx.lock();
}

/**
 *  Unlock the process manager.
 */
void process_manager::unlock() {
  _mtx.unlock();
}

/**
 *  The main loop.
 */
void process_manager::run() {
  while (!(_quit && !_fds_size)) {
    _wait_processes();

    if (_is_modify)
      _build_pollfd();

    int ret = poll(_fds, _fds_size, 10);
    if (ret == 0 || (ret == -1 && errno == EINTR))
      continue;
    else if (ret == -1) {
      logger(log_runtime_warning, basic)
	<< "poll failed (" << strerror(errno) << ")";
      continue;
    }

    QMutexLocker locker(&_mtx);

    int j(0);
    for (unsigned int i(0); i < _fds_size && j < ret; ++i) {
      if (_fds[i].revents)
        ++j;

      if (_fds[i].revents & POLLIN) {
	QHash<int, basic_process*>::iterator it(_processes_by_fd.find(_fds[i].fd));
        if (it != _processes_by_fd.end())
          it.value()->_read_fd(_fds[i].fd);
      }
      else if (_fds[i].revents & (POLLHUP | POLLNVAL | POLLERR)) {
	QHash<int, basic_process*>::iterator it(_processes_by_fd.find(_fds[i].fd));
        if (it == _processes_by_fd.end()) {
          logger(log_runtime_warning, basic)
            << "file descriptor (" << _fds[i].fd << ") not found.";
          continue;
        }
        basic_process* p(it.value());
        p->_close_fd(_fds[i].fd);
        if (!p->_pid)
          p->_finish();

        if (_fds[i].revents & (POLLNVAL | POLLERR))
          logger(log_runtime_warning, basic)
            << "file descriptor " << _fds[i].fd << " is invalid.";

        _processes_by_fd.erase(it);
        _is_modify = true;
      }
    }
  }
}

/**
 *  Build the pollfd array.
 */
void process_manager::_build_pollfd() {
  QMutexLocker locker(&_mtx);

  delete[] _fds;
  _fds_size = 0;
  _fds = new pollfd[_processes_by_fd.size()];

  for (QHash<int, basic_process*>::const_iterator it(_processes_by_fd.begin()),
         end(_processes_by_fd.end());
       it != end;
       ++it)
    if (it.key()) {
      _fds[_fds_size].fd = it.key();
      _fds[_fds_size++].events = POLLIN;
    }

  _is_modify = false;
}

/**
 *  Wait processes check if some basic process are finish.
 */
void process_manager::_wait_processes() {
  int status;
  while (true) {
    pid_t pid(waitpid(-1, &status, WNOHANG));
    if (pid <= 0)
      break;

    QMutexLocker locker(&_mtx);
    QHash<pid_t, basic_process*>::iterator it = _processes_by_pid.find(pid);
    if (it != _processes_by_pid.end()) {
      basic_process* p(it.value());
      p->_set_status(status);
      p->_finish();
      _processes_by_pid.erase(it);
    }
  }
}
