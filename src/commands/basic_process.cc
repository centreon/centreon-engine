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

#include <QBuffer>
#include <QTimer>
#include <QEventLoop>
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "error.hh"
#include "commands/basic_process.hh"

using namespace com::centreon::engine::commands;

/**
 *  Default constructor.
 *
 *  @param[in] parent The parent of this object (optional parameter).
 */
basic_process::basic_process(QObject* parent)
  : QIODevice(parent),
    _notifier_output(NULL),
    // _notifier_error(NULL),
    _notifier_dead(NULL),
    _channel(QProcess::StandardOutput),
    _perror(QProcess::UnknownError),
    _pstate(QProcess::NotRunning),
    _pid(0),
    _status(0) {
  for (unsigned int i(0); i < 2; ++i) {
    _pipe_out[i] = -1;
    // _pipe_err[i] = -1;
    // _pipe_in[i] = -1;
    _pipe_dead[i] = -1;
  }
}

/**
 *  Default destructor.
 */
basic_process::~basic_process() throw() {
  if (_pstate != QProcess::NotRunning) {
    kill();
    waitForFinished(-1);
  }
  delete _notifier_output;
  // delete _notifier_error;
  delete _notifier_dead;
}

/**
 *  Close the selected channel (standard output or standard error).
 *
 *  @param[in] channel The selected channel ({ StandardOutput, StandardError }).
 */
void basic_process::closeReadChannel(QProcess::ProcessChannel channel) {
  // int& fd(channel == QProcess::StandardOutput ? _pipe_out[0] : _pipe_err[0]);
  // _close(fd);
  if (channel == QProcess::StandardOutput)
    _close(_pipe_out[0]);
}

/**
 *  Close the standard input.
 */
void basic_process::closeWriteChannel() {
  // _close(_pipe_in[1]);
}

//  QStringList basic_process::environment() const {
//    return (_environment.toStringList());
//  }

/**
 *  Get the process error.
 *
 *  @return The process error ({ FailedToStart, Crashed, Timedout, WriteError, ReadError, UnknownError }).
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
  return (WIFSIGNALED(_status) ? QProcess::CrashExit : QProcess::NormalExit);
}

//  QString basic_process::nativeArguments() const {
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
QByteArray basic_process::readAllStandardError() {
  // QByteArray error(_standard_error);
  // _standard_error.clear();
  // return (error);
  return (QByteArray());
}

/**
 *  Read all the content of the standard output.
 *
 *  @return The data.
 */
QByteArray basic_process::readAllStandardOutput() {
  QByteArray output(_standard_output);
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

//  void basic_process::setEnvironment(QStringList const& environment) {
//    setProcessEnvironment(QProcessEnvironmentPrivate::fromList(environment));
//  }

//  void basic_process::setNativeArguments(QString const& arguments) {
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
}

//  void basic_process::setStandardErrorFile(QString const& fileName, OpenMode mode) {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

//  void basic_process::setStandardInputFile(QString const& fileName) {
//    throw (engine_error() << "basic_process: " << Q_FUNC_INFO << " not implemented yet.");
//  }

//  void basic_process::setStandardOutputFile(QString const& fileName, OpenMode mode) {
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
void basic_process::setWorkingDirectory(QString const& dir) {
  _working_directory = dir;
}

/**
 *  Execute the process with all arguments if none is already running.
 *
 *  @param[in] program   The binary name.
 *  @param[in] arguments The arguments of the program.
 *  @param[in] mode      Set the openning mode.
 */
void basic_process::start(QString const& program, QStringList const& arguments, OpenMode mode) {
  if (_pstate != QProcess::NotRunning)
    return;

  _program = program;
  _arguments = arguments;

  _start_process(mode);
}

/**
 *  Execute the command line if none process already running.
 *
 *  @param[in] program The binary name.
 *  @param[in] mode    Set the openning mode.
 */
void basic_process::start(QString const& program, OpenMode mode) {
  if (_pstate != QProcess::NotRunning)
    return;

  _arguments = _split_command_line(program);
  _program = _arguments.first();
  _arguments.removeFirst();

  _start_process(mode);
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

  if (_pstate == QProcess::NotRunning
      || (_channel == QProcess::StandardOutput && _pipe_out[0] == -1))
    // || (_channel == QProcess::StandardError && _pipe_err[0] == -1))
    return (false);
  // XXX: not implemented yet.
  return (false);
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
QString basic_process::workingDirectory() const {
  return (_working_directory);
}

/**
 *  Get number bytes available in the current channel.
 *
 *  @return The number of bytes available.
 */
qint64 basic_process::bytesAvailable() const {
  if (_channel != QProcess::StandardOutput)
    return (0);
  return (QIODevice::bytesAvailable() + _standard_output.size());
  // QByteArray const* buffer(_channel == QProcess::StandardOutput
  //                          ? &_standard_output
  //                          : &_standard_error);
  // return (QIODevice::bytesAvailable() + buffer->size());
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
 *  This IODevice is sequential.
 *
 *  @return Always true.
 */
bool basic_process::isSequential() const {
  return (true);
}

/**
 *  Get if a line is available in the current channel.
 *
 *  @return True if a line is available.
 */
bool basic_process::canReadLine() const {
  if (_channel != QProcess::StandardOutput)
    return (false);
  // QBuffer buffer(_channel == QProcess::StandardOutput
  //                ? const_cast<QByteArray*>(&_standard_output)
  //                : const_cast<QByteArray*>(&_standard_error));
  QBuffer buffer(const_cast<QByteArray*>(&_standard_output));
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
}

/**
 *  Get if the process is finished and no data are available.
 *
 *  @return True if the process is not running, and all
 *  data are read, othrewise false.
 */
bool basic_process::atEnd() const {
  if (_channel != QProcess::StandardOutput)
    return (true);
  return (QIODevice::atEnd() && (!isOpen() || _standard_output.isEmpty()));
  // QByteArray const* buffer(_channel == QProcess::StandardOutput
  //                          ? &_standard_output
  //                          : &_standard_error);
  // return (QIODevice::atEnd() && (!isOpen() || buffer->isEmpty()));
}

/**
 *  Send a KILL signal to the process.
 */
void basic_process::kill() {
  if (_pid)
    ::kill(_pid, SIGKILL);
}

/**
 *  Send a TERM signal to the process.
 */
void basic_process::terminate() {
  if (_pid)
    ::kill(_pid, SIGTERM);
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

}

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
 *  Write data on the standard input.
 *
 *  @param[in] data The buffer to write.
 *  @param[in] len  The len of the buffer.
 *
 *  @return Return the number of bytes written.
 */
qint64 basic_process::writeData(char const* data, qint64 len) {
  // return (::write(_pipe_in[1], data, len));
  (void)data;
  (void)len;
  return (0);
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

// /**
//  *  Slot call when something append (data to read,
//  *  close pipe) on the standard error.
//  *  if data are available, all data is read and
//  *  readyReadStandardError is emit and if the
//  *  pipe is close the notification system is close.
//  */
// void basic_process::_notification_standard_error() {
//   if (_pipe_err[0] == -1 || !_notifier_error)
//     return;

//   if (_read(_pipe_err[0], &_standard_error))
//     emit readyReadStandardError();
//   else {
//     _notifier_error->setEnabled(false);
//     _notifier_error->deleteLater();
//     _notifier_error = NULL;

//     _close(_pipe_err[0]);
//     _emit_finished();
//   }
// }

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

  // _close(_pipe_in[1]);
  _close(_pipe_dead[0]);

  _waitpid(_pid, &_status, 0);
  _pid = 0;

  setProcessState(QProcess::NotRunning);
  _emit_finished();
}

/**
 *  Start the process and initialize all
 *  notification system and all internal
 *  variables.
 *
 *  @param[in] mode Set the mode of IODevice.
 */
void basic_process::_start_process(OpenMode mode) {
  int old_fd(-1);
  char** args(NULL);
  try {
    if ((old_fd = dup(1)) == -1)
      throw (engine_error() << "start process failed on dup: "
             << strerror(errno));

    QIODevice::open(mode);

    _standard_output.clear();
    // _standard_error.clear();
    delete _notifier_output;
    // delete _notifier_error;
    delete _notifier_dead;
    _perror = QProcess::UnknownError;
    _status = 0;

    args = _build_args(_program, _arguments);
    _chdir(qPrintable(_working_directory));

    if (pipe(_pipe_out) == -1
        // || pipe(_pipe_err) == -1
        // || pipe(_pipe_in) == -1
        || pipe(_pipe_dead) == -1)
      throw (engine_error() << "start process failed on pipe: "
             << strerror(errno));

    if (_dup2(_pipe_out[1], 1) == -1)
      // || _dup2(_pipe_err[1], 2) == -1
      // || _dup2(_pipe_in[0], 0) == -1) {
      throw (engine_error() << "start process failed on dup2: "
             << strerror(errno));

    _close(_pipe_out[1]);
    // _close(_pipe_err[1]);
    // _close(_pipe_in[0]);

    _set_cloexec(_pipe_out[0]);
    // _set_cloexec(_pipe_err[0]);
    // _set_cloexec(_pipe_in[1]);

    setProcessState(QProcess::Starting);

    if ((_pid = vfork()) == -1)
      throw (engine_error() << "start process failed on fork: "
             << strerror(errno));

    if (!_pid) {
      execvp(args[0], args);
      ::_exit(-1);
    }

    _close(_pipe_dead[1]);

    _notifier_output = new QSocketNotifier(
                             _pipe_out[0],
                             QSocketNotifier::Read,
                             this);
    QObject::connect(_notifier_output, SIGNAL(activated(int)),
                     this, SLOT(_notification_standard_output()));

    // _notifier_error = new QSocketNotifier(
    //                         _pipe_err[0],
    //                         QSocketNotifier::Read,
    //                         this);
    // QObject::connect(_notifier_error, SIGNAL(activated(int)),
    //                  this, SLOT(_notification_standard_error()));

    _notifier_dead = new QSocketNotifier(
                           _pipe_dead[0],
                           QSocketNotifier::Read,
                           this);
    QObject::connect(
               _notifier_dead,
               SIGNAL(activated(int)),
               this,
               SLOT(_notification_dead()));

    setProcessState(QProcess::Running);
    emit started();
  }
  catch (std::exception const& e) {
    _close_pipe();
    setProcessState(QProcess::NotRunning);
    setErrorString(e.what());
    _perror = QProcess::FailedToStart;
    emit error(_perror);
  }

  if (old_fd != -1)
    dup2(old_fd, 1);
  _clean_args(args);
}

/**
 *  Close all pipe open.
 */
void basic_process::_close_pipe() throw() {
  for (unsigned int i(0); i < 2; ++i) {
    _close(_pipe_out[i]);
    // _close(_pipe_err[i]);
    // _close(_pipe_in[i]);
    _close(_pipe_dead[i]);
  }
}

/**
 *  Emit finished signal if all data of standard
 *  output, standard error as read, if all data
 *  was write in standard input and if the process
 *  was dead.
 */
void basic_process::_emit_finished() {
  if (_pipe_out[0] == -1
      // && _pipe_err[0] == -1
      // && _pipe_in[1] == -1
      && _pid == 0)
    emit finished(exitCode(), exitStatus());
}

/**
 *  Internal read to fill a QByteArray.
 *
 *  @param[in] fd  The file descriptor to read.
 *  @param[in] str The buffer to fill.
 *
 *  @return True if data was read, otherwise false.
 */
bool basic_process::_read(int fd, QByteArray* str) {
  qint64 len(_available_bytes(fd));
  if (len <= 0)
    return (false);

  char buffer[1024];
  while (len > 0) {
    qint64 min(static_cast<qint64>(sizeof(buffer)) > len
               ? len : static_cast<qint64>(sizeof(buffer)));
    qint64 size(_read(fd, buffer, min));
    if (size <= 0)
      return (false);
    str->append(buffer, size);
    len -= size;
  }
  return (true);
}

/**
 *  Release memory.
 *
 *  @param[in] args Arguments array to release memory.
 */
void basic_process::_clean_args(char** args) throw () {
  for (unsigned int i(0); args[i]; ++i)
    delete[] args[i];
  delete[] args;
}

/**
 *  Get the number of bytes available on the specific
 *  file descriptor.
 *
 *  @param[in] fd The file descriptor.
 *
 *  @return The number of bytes available.
 */
qint64 basic_process::_available_bytes(int fd) throw() {
  int nbytes(0);
  int ret(::ioctl(fd, FIONREAD, (char*)&nbytes));
  return (ret != -1 ? nbytes : -1);
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
pid_t basic_process::_waitpid(pid_t pid, int* status, int options) throw() {
  pid_t ret;
  do {
    ret = ::waitpid(pid, status, options);
  } while (ret == -1 && errno == EINTR);
  return (ret);
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
qint64 basic_process::_read(int fd, void* buffer, qint64 nbyte) throw() {
  size_t ret;
  do {
    ret = ::read(fd, buffer, static_cast<size_t>(nbyte));
  } while (ret == static_cast<size_t>(-1) && errno == EINTR);
  return (static_cast<int>(ret) == -1 ? -1 : static_cast<qint64>(ret));
}

/**
 *  Like C library close, without signal internupt.
 *
 *  @param[in] fd The file descriptor to close.
 */
void basic_process::_close(int& fd) throw() {
  if (fd == -1)
    return;

  int ret;
  do {
    ret = ::close(fd);
  } while (ret == -1 && errno == EINTR);
  fd = -1;
}

/**
 *  Like C library chdir, without signal interupt.
 *
 *  @param[in] working_directory The working directory path.
 */
int basic_process::_chdir(char const* working_directory) throw() {
  if (!working_directory || working_directory[0] == 0)
    return (0);

  int ret;
  do {
    ret = chdir(working_directory);
  } while (ret == -1 && errno == EINTR);
  return (ret);
}

/**
 *  Like C library dup2, without signal internupt.
 *
 *  @param[in] files  old file descriptor.
 *  @param[in] files2 new file descriptor.
 */
int basic_process::_dup2(int fildes, int fildes2) throw() {
  int ret;
  do {
    ret = dup2(fildes, fildes2);
  } while (ret == -1 && errno == EINTR);
  return (ret);
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
  for (QStringList::const_iterator it(arguments.begin()),
         end(arguments.end());
       it != end;
       ++it)
    args[i++] = qstrdup(qPrintable(*it));
  args[i] = NULL;
  return (args);
}

/**
 *  Set the close-on-exec flag on the file descriptor.
 *
 *  @param[in] fd The file descriptor to set close on exec.
 */
void basic_process::_set_cloexec(int fd) {
  int flags(fcntl(fd, F_GETFL));
  if (flags < 0)
    throw (engine_error() << "Could not get file descriptor flags: "
           << strerror(errno));
  if (fcntl(fd, F_SETFL, flags | FD_CLOEXEC) == -1)
    throw (engine_error() << "Could not set close-on-exec flag: "
           << strerror(errno));
}

/**
 *  Split command line on array of string.
 *
 *  @param[in] command_line The command line to split.
 *
 *  @return Array of string.
 */
QStringList basic_process::_split_command_line(QString const& command_line) {
  QStringList args;
  QString tmp;
  int count(0);
  bool in(false);

  for (int i(0), end(command_line.size()); i < end; ++i) {
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
