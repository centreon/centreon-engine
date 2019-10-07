/*
** Copyright 2012-2013 Centreon
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** For more information : contact@centreon.com
*/

#ifndef CCE_PROCESS_MANAGER_POSIX_HH
#define CCE_PROCESS_MANAGER_POSIX_HH

#include <poll.h>
#include <list>
#include <map>
#include <unordered_map>
#include "com/centreon/concurrency/mutex.hh"
#include "com/centreon/concurrency/thread.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

class process;
class process_listener;

/**
 *  @class process_manager process_manager_posix.hh
 * "com/centreon/process_manager_posix.hh"
 *  @brief This class manage process.
 *
 *  This class is a singleton and manage process.
 */
class process_manager : public concurrency::thread {
 public:
  void add(process* p);
  static process_manager& instance();

 private:
  struct orphan {
    orphan(pid_t _pid = 0, int _status = 0) : pid(_pid), status(_status) {}
    pid_t pid;
    int status;
  };
  process_manager();
  process_manager(process_manager const& p);
  ~process_manager() throw();
  process_manager& operator=(process_manager const& p);
  static void _close(int& fd) throw();
  void _close_stream(int fd) throw();
  void _erase_timeout(process* p);
  void _kill_processes_timeout() throw();
  unsigned int _read_stream(int fd) throw();
  void _run();
  void _update_ending_process(process* p, int status) throw();
  void _update_list();
  void _wait_orphans_pid() throw();
  void _wait_processes() throw();

  pollfd* _fds;
  unsigned int _fds_capacity;
  int _fds_exit[2];
  unsigned int _fds_size;
  concurrency::mutex _lock_processes;
  std::list<orphan> _orphans_pid;
  std::unordered_map<int, process*> _processes_fd;
  std::unordered_map<pid_t, process*> _processes_pid;
  std::multimap<unsigned int, process*> _processes_timeout;
  bool _update;
};

CCE_END()

#endif  // !CCE_PROCESS_MANAGER_POSIX_HH
