#ifndef CCE_COMMANDS_PROCESS_MANAGER_HH
# define CCE_COMMANDS_PROCESS_MANAGER_HH

# include <QHash>
# include <QThread>
# include <QMutex>
# include <poll.h>
# include "basic_process.hh"

namespace com {
  namespace centreon {
    namespace engine {
      namespace commands {
        /**
         *  @class process_manager commands/process_manager.hh
         *  @brief Process manager mangage all basic process.
         *
         *  Process manager mangage all basic process, it notify
         *  when a process finish or recive somme data. This class
         *  is a singleton and it woks in a thread.
         */
        class                          process_manager : protected QThread {
          Q_OBJECT
        public:
          static process_manager&      instance();
          static void                  cleanup();

          void                         add_process(basic_process* p);
          void                         remove_process(basic_process* p);

        protected:
          void                         run();

        private:
                                       process_manager();
                                       process_manager(process_manager const&);
                                       ~process_manager() throw();

          process_manager&             operator=(process_manager const&);

          void                         _build_pollfd();
          void                         _wait_processes();

          QMutex                       _mtx;
          QHash<int, basic_process*>   _processes_by_fd;
          QHash<pid_t, basic_process*> _processes_by_pid;
          pollfd*                      _fds;
          static process_manager*      _instance;
          unsigned int                 _fds_size;
          bool                         _is_modify;
          bool                         _quit;
        };
      }
    }
  }
}

#endif // !CCE_COMMANDS_PROCESS_MANAGER_HH
