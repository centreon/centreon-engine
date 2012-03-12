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

#ifndef CCE_COMMANDS_BASIC_PROCESS_HH
#  define CCE_COMMANDS_BASIC_PROCESS_HH

#  include <QByteArray>
#  include <QIODevice>
#  include <QProcess>
#  include <QSocketNotifier>
#  include <QString>
#  include <QStringList>

namespace                          com {
  namespace                        centreon {
    namespace                      engine {
      namespace                    commands {
        /**
         *  @class basic_process com/centreon/engine/commands/basic_proces.hh
         *  @brief Basic process is a reimplementation of QProcess
         *         without use select function.
         *
         *  Basic process provide a simple interface to execute
         *  some process.
         */
        class                      basic_process : public QIODevice {
          Q_OBJECT

        public:
                                   basic_process(QObject* parent = 0);
          virtual                  ~basic_process() throw ();
          bool                     atEnd() const;
          qint64                   bytesAvailable() const;
          qint64                   bytesToWrite() const;
          bool                     canReadLine() const;
          void                     close();
          void                     closeReadChannel(
                                     QProcess::ProcessChannel channel);
          void                     closeWriteChannel();
          QProcess::ProcessError   error() const;
          int                      exitCode() const;
          QProcess::ExitStatus     exitStatus() const;
          bool                     isSequential() const;
          // QString                  nativeArguments() const;
          Q_PID                    pid() const;
          // ProcessChannelMode       processChannelMode() const;
          // QProcessEnvironment      processEnvironment() const;
          QByteArray               readAllStandardError();
          QByteArray               readAllStandardOutput();
          QProcess::ProcessChannel readChannel() const;
          // void                     setNativeArguments(
          //                            QString const& arguments);
          // void                     setProcessChannelMode(
          //                            ProcessChannelMode mode);
          // void                     setProcessEnvironment(
          //                            QProcessEnvironment const& environment);
          void                     setReadChannel(
                                     QProcess::ProcessChannel channel);
          // void                     setStandardErrorFile(
          //                            QString const& fileName,
          //                            OpenMode mode = Truncate);
          // void                     setStandardInputFile(
          //                            QString const& fileName);
          // void                     setStandardOutputFile(
          //                            QString const& fileName,
          //                            OpenMode mode = Truncate);
          // void                     setStandardOutputProcess(
          //                            QProcess* destination);
          void                     setWorkingDirectory(
                                     QString const& dir);
          void                     start(
                                     QString const& program,
                                     QStringList const& arguments,
                                     OpenMode mode = ReadWrite);
          void                     start(
                                     QString const& program,
                                     OpenMode mode = ReadWrite);
          QProcess::ProcessState   state() const;
          bool                     waitForBytesWritten(
                                     int msecs = 30000);
          bool                     waitForFinished(int msecs = 30000);
          bool                     waitForReadyRead(int msecs = 30000);
          bool                     waitForStarted(int msecs = 30000);
          QString                  workingDirectory() const;

        signals:
          void                     error(QProcess::ProcessError error);
          void                     finished(
                                     int exitCode,
                                     QProcess::ExitStatus exitStatus);
          void                     readyReadStandardError();
          void                     readyReadStandardOutput();
          void                     started();
          void                     stateChanged(
                                     QProcess::ProcessState newState);

        public slots:
          void                     kill();
          void                     terminate();

        protected:
          virtual qint64           readData(char* data, qint64 maxlen);
          void                     setProcessState(
                                     QProcess::ProcessState state);
          virtual void             setupChildProcess();
          virtual qint64           writeData(
                                     char const* data,
                                     qint64 len);

        private:
                                   basic_process(
                                     basic_process const& right);
          basic_process&           operator=(
                                     basic_process const& right);
          static char**            _build_args(
                                     QString const& program,
                                     QStringList const& arguments);
          static int               _chdir(char const* wd) throw ();
          static void              _clean_args(char** args) throw ();
          static void              _close(int& fd) throw ();
          void                     _close_pipes() throw ();
          static int               _dup2(
                                     int fildes,
                                     int fildes2) throw ();
          void                     _emit_finished();
          void                     _internal_copy(
                                     basic_process const& right);
          static bool              _read(int fd, QByteArray* str);
          static qint64            _read(
                                     int fd,
                                     void* buffer,
                                     qint64 nbyte) throw ();
          static void              _set_cloexec(int fd);
          void                     _start_process(OpenMode mode);
          static pid_t             _waitpid(
                                     pid_t pid,
                                     int* status,
                                     int options) throw ();

          char**                   _args;
          QProcess::ProcessChannel _channel;
          // QProcessEnvironment      _environment;
          QSocketNotifier*         _notifier_dead;
          QSocketNotifier*         _notifier_error;
          QSocketNotifier*         _notifier_output;
          QProcess::ProcessError   _perror;
          pid_t                    _pid;
          int                      _pipe_dead[2];
          int                      _pipe_err[2];
          int                      _pipe_in[2];
          int                      _pipe_out[2];
          QString                  _program;
          QProcess::ProcessState   _pstate;
          QByteArray               _standard_error;
          QByteArray               _standard_output;
          int                      _status;
          bool                     _want_err;
          bool                     _want_in;
          bool                     _want_out;
          QString                  _working_directory;

        private slots:
          void                     _notification_dead();
          void                     _notification_standard_error();
          void                     _notification_standard_output();
        };
      }
    }
  }
}

#endif // !CCE_COMMANDS_BASIC_PROCESS_HH
