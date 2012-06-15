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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <QBuffer>
#include <QEventLoop>
#include <QTimer>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "com/centreon/engine/commands/basic_process.hh"
#include "com/centreon/engine/commands/command_line.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::commands;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] parent The parent of this object (optional parameter).
 */
basic_process::basic_process(QObject* parent)
  : QIODevice(parent),
    _channel(QProcess::StandardOutput),
    _notifier_dead(NULL),
    _notifier_error(NULL),
    _notifier_output(NULL),
    _perror(QProcess::UnknownError),
    _pid((pid_t)-1),
    _pstate(QProcess::NotRunning),
    _status(0),
    _want_err(true),
    _want_in(true),
    _want_out(true) {
  for (unsigned int i(0); i < 2; ++i) {
    _pipe_dead[i] = -1;
    _pipe_err[i] = -1;
    _pipe_in[i] = -1;
    _pipe_out[i] = -1;
  }
}

/**
 *  Destructor.
 */
basic_process::~basic_process() throw () {
  if (_pstate != QProcess::NotRunning) {
    kill();
    waitForFinished(-1);
  }
  delete _notifier_dead;
  delete _notifier_error;
  delete _notifier_output;
}

/**
 *  Check if the process is finished and no data is available.
 *
 *  @return True if the process is not running, and all
 *          data are read, false otherwise.
 */
bool basic_process::atEnd() const {
  QByteArray const* buffer((_channel == QProcess::StandardOutput)
                           ? &_standard_output
                           : &_standard_error);
  return (QIODevice::atEnd() && (!isOpen() || buffer->isEmpty()));
}

/**
 *  Get number of bytes available in the current channel.
 *
 *  @return The number of bytes available.
 */
qint64 basic_process::bytesAvailable() const {
  QByteArray const* buffer(_channel == QProcess::StandardOutput
                           ? &_standard_output
                           : &_standard_error);
  return (QIODevice::bytesAvailable() + buffer->size());
}

/**
 *  Get number bytes to write in the current channel.
 *
 *  @return The number of bytes to write.
 */
qint64 basic_process::bytesToWrite() const {
  return (0);
}

/**
 *  Get if a line is available in the current channel.
 *
 *  @return True if a line is available.
 */
bool basic_process::canReadLine() const {
  // XXX : QBuffer might modify our QByteArrays
  QBuffer buffer(_channel == QProcess::StandardOutput
                 ? const_cast<QByteArray*>(&_standard_output)
                 : const_cast<QByteArray*>(&_standard_error));
  return (buffer.canReadLine() || QIODevice::canReadLine());
}

/**
 *  Close all communication with the process and kill it.
 */
void basic_process::close() {
  emit aboutToClose();
  while (waitForBytesWritten(-1))
    ;
  kill();
  waitForFinished(-1);
  QIODevice::close();
  return ;
}

/**
 *  Close the selected channel (standard output or standard error).
 *
 *  @param[in] channel The selected channel ({ StandardOutput,
 *                     StandardError }).
 */
void basic_process::closeReadChannel(QProcess::ProcessChannel channel) {
  if (QProcess::StandardOutput == channel) {
    _close(_pipe_out[0]);
    _want_out = false;
  }
  else {
    _close(_pipe_err[0]);
    _want_err = false;
  }
  return ;
}

/**
 *  Close the standard input.
 */
void basic_process::closeWriteChannel() {
  _close(_pipe_in[1]);
  _want_in = false;
  return ;
}

/**
 *  Get the process error.
 *
 *  @return The process error ({ FailedToStart, Crashed, Timedout,
 *          WriteError, ReadError, UnknownError }).
 */
QProcess::ProcessError basic_process::error() const {
  return (_perror);
}

/**
 *  Get the exit code of the process.
 *
 *  @return The exit code of the process.
 */
int basic_process::exitCode() const {
  return (WEXITSTATUS(_status));
}

/**
 *  Get the exit status of the process.
 *
 *  @return The exit status ({ NormalExit, CrashExit }).
 */
QProcess::ExitStatus basic_process::exitStatus() const {
  return (WIFEXITED(_status)
          ? QProcess::NormalExit
          : QProcess::CrashExit);
}

/**
 *  This IODevice is sequential.
 *
 *  @return Always true.
 */
bool basic_process::isSequential() const {
  return (true);
}

//  std::string basic_process::nativeArguments() const {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

/**
 *  Get the pid of the process.
 *
 *  @return The pid.
 */
Q_PID basic_process::pid() const {
  return (static_cast<Q_PID>(_pid));
}

//  ProcessChannelMode basic_process::processChannelMode() const {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

// QProcessEnvironment basic_process::processEnvironment() const {
//   return (_environment);
// }

/**
 *  Read all the content of the standard error.
 *
 *  @return The data.
 */
std::string basic_process::readAllStandardError() {
  std::string error(_standard_error);
  _standard_error.clear();
  return (error);
}

/**
 *  Read all the content of the standard output.
 *
 *  @return The data.
 */
std::string basic_process::readAllStandardOutput() {
  std::string output(_standard_output);
  _standard_output.clear();
  return (output);
}

/**
 *  Get the current channel.
 *
 *  @return The channel ({ StandardOutput, StandardError }).
 */
QProcess::ProcessChannel basic_process::readChannel() const {
  return (_channel);
}

//  void basic_process::setNativeArguments(std::string const& arguments) {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

//  void basic_process::setProcessChannelMode(ProcessChannelMode mode) {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

//  void basic_process::setProcessEnvironment(QProcessEnvironment const& environment) {
//    _environment = environment;
//  }

/**
 *  Set the current channel.
 *
 *  @param[in] channel The channel ({ StandardOutput, StandardError }).
 */
void basic_process::setReadChannel(QProcess::ProcessChannel channel) {
  _channel = channel;
  return ;
}

//  void basic_process::setStandardErrorFile(std::string const& fileName, OpenMode mode) {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

//  void basic_process::setStandardInputFile(std::string const& fileName) {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

//  void basic_process::setStandardOutputFile(std::string const& fileName, OpenMode mode) {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

//  void basic_process::setStandardOutputProcess(QProcess* destination) {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

/**
 *  Set the working directory of the process.
 *
 *  @param[in] dir The working directory.
 */
void basic_process::setWorkingDirectory(std::string const& dir) {
  _working_directory = dir;
  return ;
}

/**
 *  Execute the process with all arguments if none is already running.
 *
 *  @param[in] program   The binary name.
 *  @param[in] arguments The arguments of the program.
 *  @param[in] mode      Set the openning mode.
 */
void basic_process::start(
                      std::string const& program,
                      std::list<std::string> const& arguments,
                      OpenMode mode) {
  if (_pstate == QProcess::NotRunning) {
    command_line cmdline(program, arguments);
    _args = cmdline.get_argv();
    _start_process(mode);
  }
  return;
}

/**
 *  Execute the command line if none process already running.
 *
 *  @param[in] program The binary name.
 *  @param[in] mode    Set the openning mode.
 */
void basic_process::start(std::string const& program, OpenMode mode) {
  if (_pstate == QProcess::NotRunning) {
    command_line cmdline(program);
    _args = cmdline.get_argv();
    _start_process(mode);
  }
  return;
}

/**
 *  Get the current state of the process.
 *
 *  @return The state ({ NotRunning, Starting, Running }).
 */
QProcess::ProcessState basic_process::state() const {
  return (_pstate);
}

/**
 *  Blocks until the process as data to write, or
 *  until timeout have passed.
 *
 *  @param[in] msecs The timeout.
 *
 *  @return True if process has data to write, otherwise false.
 */
bool basic_process::waitForBytesWritten(int msecs) {
  (void)msecs;
  if (_pstate == QProcess::NotRunning)
    return (false);
  // XXX: not implemented yet.
  return (true);
}

/**
 *  Blocks until the process has finished, or
 *  until timeout have passed.
 *
 *  @param[in] msecs The timeout.
 *
 *  @return True if process finished, otherwise false.
 */
bool basic_process::waitForFinished(int msecs) {
  QEventLoop loop;
  QObject::connect(
    this,
    SIGNAL(finished(int, QProcess::ExitStatus)),
    &loop,
    SLOT(quit()));
  QObject::connect(
    this,
    SIGNAL(error(QProcess::ProcessError)),
    &loop,
    SLOT(quit()));
  if (msecs && _pstate == QProcess::Running) {
    if (msecs > 0)
      QTimer::singleShot(msecs, &loop, SLOT(quit()));
    loop.exec();
  }
  return (_pstate == QProcess::NotRunning);
}

/**
 *  Blocks until the process has data to read, or
 *  until timeout have passed.
 *
 *  @param[in] msecs The timeout.
 *
 *  @return True if process has data to read, otherwise false.
 */
bool basic_process::waitForReadyRead(int msecs) {
  (void)msecs;
  if ((QProcess::NotRunning == _pstate)
      || ((QProcess::StandardOutput == _channel)
          && (-1 == _pipe_out[0]))
      || ((QProcess::StandardError == _channel)
          && (-1 == _pipe_err[0])))
    return (false);
  // XXX: not implemented yet.
  return (false);
}

/**
 *  Blocks until the process has started, or
 *  until timeout have passed.
 *
 *  @param[in] msecs The timeout.
 *
 *  @return True if process started, otherwise false.
 */
bool basic_process::waitForStarted(int msecs) {
  QEventLoop loop;
  QObject::connect(
    this,
    SIGNAL(started()),
    &loop,
    SLOT(quit()));
  QObject::connect(
    this,
    SIGNAL(error(QProcess::ProcessError)),
    &loop,
    SLOT(quit()));
  if (msecs && _pstate == QProcess::Starting) {
    if (msecs > 0)
      QTimer::singleShot(msecs, &loop, SLOT(quit()));
    loop.exec();
  }
  return (_pstate == QProcess::Running);
}

/**
 *  Get the working directory path, if a
 *  working directory was set.
 *
 *  @return The working directory path.
 */
std::string basic_process::workingDirectory() const {
  return (_working_directory);
}

/**
 *  Send a KILL signal to the process.
 */
void basic_process::kill() {
  if (_pid != (pid_t)-1)
    ::kill(_pid, SIGKILL);
  return ;
}

/**
 *  Send a TERM signal to the process.
 */
void basic_process::terminate() {
  if (_pid != (pid_t)-1)
    ::kill(_pid, SIGTERM);
  return ;
}

/**************************************
*                                     *
*          Protected Methods          *
*                                     *
**************************************/

/**
 *  Read data on the current channel.
 *
 *  @param[in] data   The read buffer.
 *  @param[in] maxlen The max number of bytes to read.
 *
 *  @return Return the number of bytes read.
 */
qint64 basic_process::readData(char* data, qint64 maxlen) {
  if (_channel != QProcess::StandardOutput)
    return (0);
  qint64 to_read(qMin((int)maxlen, _standard_output.size()));
  if (to_read > 0) {
    memcpy(data, _standard_output.constData(), to_read);
    _standard_output.right(_standard_output.size() - to_read);
  }
  return (to_read);
}

/**
 *  Set the state of the process.
 *
 *  @param[in] state The state of the process
 *  ({ NotRunning, Starting, Running }).
 */
void basic_process::setProcessState(QProcess::ProcessState state) {
  if (_pstate != state) {
    _pstate = state;
    emit stateChanged(state);
  }
}

/**
 *  This function is called in the child process
 *  just before the program is executed.
 */
void basic_process::setupChildProcess() {
  return ;
}

/**
 *  Write data on the standard input.
 *
 *  @param[in] data The buffer to write.
 *  @param[in] len  The len of the buffer.
 *
 *  @return Return the number of bytes written.
 */
qint64 basic_process::writeData(char const* data, qint64 len) {
  return ((_pipe_in[1] >= 0)
          ? ::write(_pipe_in[1], data, len)
          : 0);
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
basic_process::basic_process(basic_process const& right) : QIODevice() {
  _internal_copy(right);
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
basic_process& basic_process::operator=(basic_process const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Create and array of arguments to call execvp.
 *
 *  @param[in] progname  The program name.
 *  @param[in] arguments The program arguments.
 *
 *  @return Array of arguments.
 */
char** basic_process::_build_args(QString const& program, QStringList const& arguments) {
  char** args(new char*[arguments.size() + 2]);
  args[0] = qstrdup(qPrintable(program));
  unsigned int i(1);
  for (QStringList::const_iterator
         it(arguments.begin()),
         end(arguments.end());
       it != end;
       ++it)
    args[i++] = qstrdup(qPrintable(*it));
  args[i] = NULL;
  return (args);
}

/**
 *  Like C library chdir, without signal interupt.
 *
 *  @param[in] wd The working directory path.
 */
int basic_process::_chdir(char const* wd) throw () {
  // Argument checking.
  if (!wd || wd[0] == 0)
    return (0);

  // Action.
  int ret;
  do {
    ret = chdir(wd);
  } while (ret == -1 && errno == EINTR);
  return (ret);
}

/**
 *  Release memory.
 *
 *  @param[in] args Arguments array to release memory.
 */
void basic_process::_clean_args(char** args) throw () {
  for (unsigned int i(0); args[i]; ++i)
    delete [] args[i];
  delete [] args;
  return ;
}

/**
 *  Like C library close, without signal internupt.
 *
 *  @param[in] fd The file descriptor to close.
 */
void basic_process::_close(int& fd) throw () {
  // Argument checking.
  if (fd == -1)
    return ;

  // Action.
  int ret;
  do {
    ret = ::close(fd);
  } while (ret == -1 && errno == EINTR);
  fd = -1;
  return ;
}

/**
 *  Close all pipe open.
 */
void basic_process::_close_pipes() throw () {
  for (unsigned int i(0); i < 2; ++i) {
    _close(_pipe_dead[i]);
    _close(_pipe_err[i]);
    _close(_pipe_in[i]);
    _close(_pipe_out[i]);
  }
  return ;
}

/**
 *  Like C library dup2, without signal internupt.
 *
 *  @param[in] files  old file descriptor.
 *  @param[in] files2 new file descriptor.
 */
int basic_process::_dup2(int fildes, int fildes2) throw () {
  int ret;
  do {
    ret = dup2(fildes, fildes2);
  } while (ret == -1 && errno == EINTR);
  return (ret);
}

/**
 *  Emit finished signal if all data of standard
 *  output, standard error as read, if all data
 *  was write in standard input and if the process
 *  was dead.
 */
void basic_process::_emit_finished() {
  if ((-1 == _pipe_err[0])
      && (-1 == _pipe_in[1])
      && (-1 == _pipe_out[0])
      && (0 == _pid))
    emit finished(exitCode(), exitStatus());
  return ;
}

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void basic_process::_internal_copy(basic_process const& right) {
  (void)right;
  assert(!"processes are not copyable (basic_process)");
  abort();
  return ;
}

/**
 *  Internal read to fill a std::string.
 *
 *  @param[in] fd  The file descriptor to read.
 *  @param[in] str The buffer to fill.
 *
 *  @return True if data was read, otherwise false.
 */
bool basic_process::_read(int fd, std::string* str) {
  char buffer[1024];
  qint64 size(_read(fd, buffer, sizeof(buffer)));
  if (size <= 0)
    return (false);
  str->append(buffer, size);
  return (true);
}

/**
 *  Like C library read, without signal internupt.
 *
 *  @param[in] fd     The file descriptor to read.
 *  @param[in] buffer The buffer to store data.
 *  @param[in] nbytes The maximum bytes to read.
 *
 *  @return The number of bytes to read.
 */
qint64 basic_process::_read(int fd, void* buffer, qint64 nbyte) throw () {
  size_t ret;
  do {
    ret = ::read(fd, buffer, static_cast<size_t>(nbyte));
  } while (ret == static_cast<size_t>(-1) && errno == EINTR);
  return (static_cast<int>(ret) == -1 ? -1 : static_cast<qint64>(ret));
}

/**
 *  Set the close-on-exec flag on the file descriptor.
 *
 *  @param[in] fd The file descriptor to set close on exec.
 */
void basic_process::_set_cloexec(int fd) {
  int flags(fcntl(fd, F_GETFL));
  if (flags < 0) {
    char const* msg(strerror(errno));
    throw (engine_error() << "Could not get file descriptor flags: "
           << msg);
  }
  if (fcntl(fd, F_SETFL, flags | FD_CLOEXEC) == -1) {
    char const* msg(strerror(errno));
    throw (engine_error() << "Could not set close-on-exec flag: "
           << msg);
  }
  return ;
}

/**
 *  Start the process and initialize all
 *  notification system and all internal
 *  variables.
 *
 *  @param[in] mode Set the mode of IODevice.
 */
void basic_process::_start_process(OpenMode mode) {
  int old_fd[3] = { -1, -1, -1 };
  try {
    // As we will use vfork, we need to backup standard FDs.
    if (((old_fd[0] = dup(STDIN_FILENO)) < 0)
        || ((old_fd[1] = dup(STDOUT_FILENO)) < 0)
        || ((old_fd[2] = dup(STDERR_FILENO)) < 0)) {
      char const* msg(strerror(errno));
      for (unsigned int i(0); i < 3; ++i)
        _close(old_fd[i]);
      throw (engine_error() << "start process failed on dup: " << msg);
    }

    // Backup FDs do not need to be inherited.
    for (unsigned int i(0); i < 3; ++i)
      _set_cloexec(old_fd[i]);

    // Initialize.
    QIODevice::open(mode);
    _standard_error.clear();
    _standard_output.clear();
    delete _notifier_dead;
    _notifier_dead = NULL;
    delete _notifier_error;
    _notifier_error = NULL;
    delete _notifier_output;
    _notifier_output = NULL;
    _perror = QProcess::UnknownError;
    _status = 0;

    _chdir(qPrintable(_working_directory));

    // Open communication pipes.
    if ((pipe(_pipe_dead) == -1)
        || (_want_err && (pipe(_pipe_err) == -1))
        || (_want_in && (pipe(_pipe_in) == -1))
        || (_want_out && (pipe(_pipe_out) == -1))) {
      char const* msg(strerror(errno));
      for (unsigned int i(0); i < 2; ++i) {
        _close(_pipe_err[i]);
        _close(_pipe_in[i]);
        _close(_pipe_out[i]);
      }
      throw (engine_error() << "start process failed on pipe: " << msg);
    }

    // Duplicate FDs.
    if ((_want_err && (_dup2(_pipe_err[1], STDERR_FILENO) == -1))
        || (_want_in && (_dup2(_pipe_in[0], STDIN_FILENO) == -1))
        || (_want_out && (_dup2(_pipe_out[1], STDOUT_FILENO) == -1))) {
      char const* msg(strerror(errno));
      throw (engine_error() << "start process failed on dup2: " << msg);
    }

    // Close useless pipes.
    _close(_pipe_err[1]);
    _close(_pipe_in[0]);
    _close(_pipe_out[1]);

    // Do not inheritate parent ends of the pipes.
    _set_cloexec(_pipe_dead[0]);
    if (_want_err)
      _set_cloexec(_pipe_err[0]);
    if (_want_in)
      _set_cloexec(_pipe_in[1]);
    if (_want_out)
      _set_cloexec(_pipe_out[0]);

    // Here we go !
    setProcessState(QProcess::Starting);
    bool env_macros(config.get_enable_environment_macros());
    if (!env_macros) {
      if (-1 == (_pid = vfork())) {
        char const* msg(strerror(errno));
        throw (engine_error()
               << "could not start process because of a vfork error: "
               << msg);
      }
    }
    else {
      if (-1 == (_pid = fork())) {
        char const* msg(strerror(errno));
        throw (engine_error()
               << "could not start process because of a fork error: "
               << msg);
      }
      setupChildProcess();
    }

    // Child execution.
    if (!_pid) {
      execvp(_args[0], _args);
      ::_exit(EXIT_FAILURE);
    }

    // Close dead pipe end.
    _close(_pipe_dead[1]);

    // Get notified if process exits.
    _notifier_dead = new QSocketNotifier(
                           _pipe_dead[0],
                           QSocketNotifier::Read,
                           this);
    QObject::connect(
               _notifier_dead,
               SIGNAL(activated(int)),
               this,
               SLOT(_notification_dead()));

    // Get notified of process' stderr.
    if (_want_err) {
      _notifier_error = new QSocketNotifier(
                              _pipe_err[0],
                              QSocketNotifier::Read,
                              this);
      QObject::connect(
                 _notifier_error,
                 SIGNAL(activated(int)),
                 this,
                 SLOT(_notification_standard_error()));
    }

    // Get notified of process' stdout.
    if (_want_out) {
      _notifier_output = new QSocketNotifier(
                               _pipe_out[0],
                               QSocketNotifier::Read,
                               this);
      QObject::connect(
                 _notifier_output,
                 SIGNAL(activated(int)),
                 this,
                 SLOT(_notification_standard_output()));
    }

    // Process if now fully running.
    setProcessState(QProcess::Running);
    emit started();
  }
  catch (std::exception const& e) {
    _close_pipes();
    setProcessState(QProcess::NotRunning);
    setErrorString(e.what());
    _perror = QProcess::FailedToStart;
    emit error(_perror);
  }

  // Restore original FDs.
  for (unsigned int i(0); i < 3; ++i)
    if (old_fd[i] >= 0) {
      dup2(old_fd[i], i);
      _close(old_fd[i]);
    }

  return ;
}

/**
 *  Like C library waitpid, without signal internupt.
 *
 *  @param[in] pid     Wait for the child whose process ID
 *                     is equal to the value of pid.
 *  @param[in] status  Store status information in the
 *                     variable to which it points.
 *  @param[in] options Specific constant of waitpid.
 *
 *  @return Retrun the pid on sucess, otherwise -1.
 */
pid_t basic_process::_waitpid(
                       pid_t pid,
                       int* status,
                       int options) throw () {
  pid_t ret;
  do {
    ret = ::waitpid(pid, status, options);
  } while (ret == -1 && errno == EINTR);
  return (ret);
}

/**
 *  Slot call when the process finished.
 *  The notification system is close and
 *  waitpid if call to get the status of
 *  the child.
 */
void basic_process::_notification_dead() {
  if (_pipe_dead[0] == -1 || !_notifier_dead)
    return;

  _notifier_dead->setEnabled(false);
  _notifier_dead->deleteLater();
  _notifier_dead = NULL;

  _close(_pipe_in[1]);
  _close(_pipe_dead[0]);

  _waitpid(_pid, &_status, 0);
  _pid = 0;

  setProcessState(QProcess::NotRunning);
  _emit_finished();
}

/**
 *  Slot call when something append (data to read,
 *  close pipe) on the standard error.
 *  if data are available, all data is read and
 *  readyReadStandardError is emit and if the
 *  pipe is close the notification system is close.
 */
void basic_process::_notification_standard_error() {
  if (_pipe_err[0] == -1 || !_notifier_error)
    return;

  if (_read(_pipe_err[0], &_standard_error))
    emit readyReadStandardError();
  else {
    _notifier_error->setEnabled(false);
    _notifier_error->deleteLater();
    _notifier_error = NULL;

    _close(_pipe_err[0]);
    _emit_finished();
  }
}

/**
 *  Slot call when something append (data to read,
 *  close pipe) on the standard output.
 *  if data are available, all data is read and
 *  readyReadStandardOutput is emit and if the
 *  pipe is close the notification system is close.
 */
void basic_process::_notification_standard_output() {
  if (_pipe_out[0] == -1 || !_notifier_output)
    return;

  if (_read(_pipe_out[0], &_standard_output))
    emit readyReadStandardOutput();
  else {
    _notifier_output->setEnabled(false);
    _notifier_output->deleteLater();
    _notifier_output = NULL;

    _close(_pipe_out[0]);
    _emit_finished();
  }
}

