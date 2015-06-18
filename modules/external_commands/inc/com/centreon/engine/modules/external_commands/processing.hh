/*
** Copyright 2011-2013,2015 Merethis
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

#ifndef CCE_MOD_EXTCMD_PROCESSING_HH
#  define CCE_MOD_EXTCMD_PROCESSING_HH

#  include <cstring>
#  include <map>
#  include <string>
#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/objects/contact.hh"
#  include "com/centreon/engine/objects/contactsmember.hh"
#  include "com/centreon/engine/objects/contactgroup.hh"
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/hostsmember.hh"
#  include "com/centreon/engine/objects/hostgroup.hh"
#  include "com/centreon/engine/objects/service.hh"
#  include "com/centreon/engine/objects/servicesmember.hh"
#  include "com/centreon/engine/objects/servicegroup.hh"
#  include "com/centreon/unordered_hash.hh"
#  include "find.hh"

CCE_BEGIN()

namespace         modules {
  namespace       external_commands {
    class         processing {
    public:
                  processing();
                  ~processing() throw ();
      bool        execute(char const* cmd) const;
      bool        is_thread_safe(char const* cmd) const;

    private:
      struct      command_info {
                  command_info(
                    int _id = 0,
                    void (*_func)(int, time_t, char*) = NULL,
                    bool is_thread_safe = false)
                    : id(_id),
                      func(_func),
                      thread_safe(is_thread_safe) {}
                  ~command_info() throw () {}
        int       id;
        void (*   func)(int id, time_t entry_time, char* args);
        bool      thread_safe;
      };

      static void _wrapper_read_state_information();
      static void _wrapper_save_state_information();
      static void _wrapper_save_status_information();
      static void _wrapper_enable_host_svc_checks(host* hst);
      static void _wrapper_disable_host_svc_checks(host* hst);
      static void _wrapper_enable_service_checks(host* hst);
      static void _wrapper_disable_service_checks(host* hst);

      template <void (*fptr)()>
      static void _redirector(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;
        (void)args;
        (*fptr)();
      }

      template <int (*fptr)()>
      static void _redirector(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;
        (void)args;
        (*fptr)();
      }

      template <void (*fptr)(int, char*)>
      static void _redirector(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)entry_time;
        (*fptr)(id, args);
      }

      template <int (*fptr)(int, char*)>
      static void _redirector(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)entry_time;
        (*fptr)(id, args);
      }

      template <int (*fptr)(int, time_t, char*)>
      static void _redirector(
                    int id,
                    time_t entry_time,
                    char* args) {
        (*fptr)(id, entry_time, args);
      }

      template <void (*fptr)(host*)>
      static void _redirector_host(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;

        char* name(my_strtok(args, ";"));
        host* hst(::find_host(name));
        if (!hst)
          return ;
        (*fptr)(hst);
      }

      template <void (*fptr)(host*, char*)>
      static void _redirector_host(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;

        char* name(my_strtok(args, ";"));
        host* hst(::find_host(name));
        if (!hst)
          return ;
        (*fptr)(hst, args + strlen(name) + 1);
      }

      template <void (*fptr)(service*)>
      static void _redirector_service(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;

        char* name(my_strtok(args, ";"));
        char* description(my_strtok(NULL, ";"));
        service* svc(::find_service(name, description));
        if (!svc)
          return ;
        (*fptr)(svc);
      }

      template <void (*fptr)(service*, char*)>
      static void _redirector_service(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;

        char* name(my_strtok(args, ";"));
        char* description(my_strtok(NULL, ";"));
        service* svc(::find_service(name, description));
        if (!svc)
          return ;
        (*fptr)(svc, args + strlen(name) + strlen(description) + 2);
      }

      umap<std::string, command_info> _lst_command;
      mutable concurrency::mutex      _mutex;
    };
  }
}

CCE_END()

#endif // !CCE_MOD_EXTCMD_PROCESSING_HH
