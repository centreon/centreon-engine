#ifndef CCE_COMMANDS_BASIC_PROCESS_HH
# define CCE_COMMANDS_BASIC_PROCESS_HH

# include <QProcess>
# include <QObject>
# include <QByteArray>
# include <QString>
# include <QStringList>
# include <QWaitCondition>
# include <QMutex>
# include <sys/types.h>

namespace com {
  namespace centreon {
    namespace engine {
      namespace commands {
        /**
         *  @class basic_process commands/basic_process.hh
         *  @brief Basic process is a simplify implementation
         *  of QProcess to by pass somme Qt bug (with using
         *  select) and avec better performences with using
         *  vfork.
         */
        class                    basic_process : public QObject {
          friend class           process_manager;
          Q_OBJECT
        public:
                                 basic_process();
                                 ~basic_process() throw();

          void                   closeReadChannel(QProcess::ProcessChannel channel);
          void                   closeWriteChannel();

          // QStringList            environment() const;

          QProcess::ProcessError error() const;
          int                    exitCode() const;
          QProcess::ExitStatus   exitStatus() const;

          // QString                nativeArguments() const;

          Q_PID                  pid() const;

          QByteArray             readAllStandardOutput();
          QByteArray             readAllStandardError();

          void                   start(QString const& program, QStringList const& arguments);
          void                   start(QString const& program);

          QProcess::ProcessState state() const;

          bool                   waitForStarted(int msecs = 3000);
          bool                   waitForFinished(int msecs = 3000);

          QString                errorString() const;
          int                    write(QByteArray const& data) const throw();

        public slots:
          void                   kill();
          void                   terminate();

        signals:
          void                   error(QProcess::ProcessError error);
          void                   finished(int exitCode,
                                          QProcess::ExitStatus exitStatus);
          void                   readyReadStandardError();
          void                   readyReadStandardOutput();
          void                   started();
          void                   stateChanged(QProcess::ProcessState newState);

        protected:
          virtual void           setupChildProcess();

        private:
          enum                   e_state {
            not_running = 0,
            running = 1,
            ended = 2
          };

                                 basic_process(basic_process const&);
          basic_process&         operator=(basic_process const&);

          void                   _start(char** args);
          void                   _finish() throw();
          void                   _close_all() throw();
          void                   _read_fd(int fd);
          void                   _close_fd(int fd);
          void                   _set_status(int status);
          static QStringList     _split_command_line(QString const& command_line);

          QWaitCondition         _cond_started;
          QWaitCondition         _cond_ended;
          mutable QMutex         _mtx;
          QByteArray             _output;
          QByteArray             _error;
          QProcess::ProcessError _perror;
          QProcess::ProcessState _state;
          pid_t                  _pid;
          int                    _status;
          e_state                _internal_state;
          int                    _pipe_out[2];
          int                    _pipe_err[2];
          int                    _pipe_in[2];
        };
      }
    }
  }
}

#endif // !CCE_COMMANDS_BASIC_PROCESS_HH
