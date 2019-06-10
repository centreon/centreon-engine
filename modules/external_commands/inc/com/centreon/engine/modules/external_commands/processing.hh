/*
** Copyright 2011-2019 Centreon
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
#  include <unordered_map>
#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/configuration/applier/state.hh"
#  include "com/centreon/engine/contact.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/contactgroup.hh"
#  include "com/centreon/engine/host.hh"
#  include "com/centreon/engine/hostgroup.hh"
#  include "com/centreon/engine/service.hh"
#  include "com/centreon/engine/servicegroup.hh"
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
      static void _wrapper_enable_host_and_child_notifications(host* hst);
      static void _wrapper_disable_host_and_child_notifications(host* hst);
      static void _wrapper_enable_all_notifications_beyond_host(host* hst);
      static void _wrapper_disable_all_notifications_beyond_host(host* hst);
      static void _wrapper_enable_host_svc_notifications(host* hst);
      static void _wrapper_disable_host_svc_notifications(host* hst);
      static void _wrapper_enable_host_svc_checks(host* hst);
      static void _wrapper_disable_host_svc_checks(host* hst);
      static void _wrapper_set_host_notification_number(
                    host* hst,
                    char* args);
      static void _wrapper_send_custom_host_notification(
                    host* hst,
                    char* args);
      static void _wrapper_enable_service_notifications(host* hst);
      static void _wrapper_disable_service_notifications(host* hst);
      static void _wrapper_enable_service_checks(host* hst);
      static void _wrapper_disable_service_checks(host* hst);
      static void _wrapper_enable_passive_service_checks(host* hst);
      static void _wrapper_disable_passive_service_checks(host* hst);
      static void _wrapper_set_service_notification_number(
                    service* svc,
                    char* args);
      static void _wrapper_send_custom_service_notification(
                    service* svc,
                    char* args);

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

        host* hst(nullptr);
        umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
          it(configuration::applier::state::instance().hosts().find(get_host_id(name)));
        if (it != configuration::applier::state::instance().hosts().end())
          hst = it->second.get();

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

        host* hst(nullptr);
        umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
          it(configuration::applier::state::instance().hosts().find(get_host_id(name)));
        if (it != configuration::applier::state::instance().hosts().end())
          hst = it->second.get();

        if (!hst)
          return ;
        (*fptr)(hst, args + strlen(name) + 1);
      }

      template <void (*fptr)(host*)>
      static void _redirector_hostgroup(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;

        char* group_name(my_strtok(args, ";"));

        hostgroup* group(nullptr);
        hostgroup_map::const_iterator
          it{hostgroup::hostgroups.find(group_name)};
        if (it != hostgroup::hostgroups.end())
          group = it->second.get();
        if (!group)
          return ;

        for (host_map::iterator
               it(group->members.begin()),
               end(group->members.begin());
             it != end;
             ++it)
          if (it->second)
            (*fptr)(it->second.get());
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

      template <void (*fptr)(service*)>
      static void _redirector_servicegroup(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;

        char* group_name(my_strtok(args, ";"));
        servicegroup_map::const_iterator sg_it{servicegroup::servicegroups.find(group_name)};
        if (sg_it == servicegroup::servicegroups.end())
          return ;
        servicegroup* group{sg_it->second.get()};

        for (service_map::iterator
               it(group->members.begin()),
               end(group->members.end());
             it != end;
             ++it)
          if (it->second.get())
            (*fptr)(it->second.get());
      }

      template <void (*fptr)(host*)>
      static void _redirector_servicegroup(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;

        char* group_name(my_strtok(args, ";"));
        servicegroup_map::const_iterator sg_it{servicegroup::servicegroups.find(group_name)};
        if (sg_it == servicegroup::servicegroups.end())
          return ;
        servicegroup* group{sg_it->second.get()};

        host* last_host{nullptr};
        for (service_map::iterator
               it(group->members.begin()),
               end(group->members.end());
             it != end;
             ++it) {
          host* hst(nullptr);
          umap<uint64_t,
               std::shared_ptr<com::centreon::engine::host>>::const_iterator
              found(configuration::applier::state::instance().hosts().find(
                  get_host_id(it->first.first)));
          if (found != configuration::applier::state::instance().hosts().end())
            hst = found->second.get();

          if (!hst || hst == last_host)
            continue;
          (*fptr)(hst);
          last_host = hst;
        }
      }

      template <void (*fptr)(contact*)>
      static void _redirector_contact(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;

        char* name(my_strtok(args, ";"));
        contact* cntc(configuration::applier::state::instance().find_contact(name));
        if (!cntc)
          return ;
        (*fptr)(cntc);
      }

      template <void (*fptr)(contact*)>
      static void _redirector_contactgroup(
                    int id,
                    time_t entry_time,
                    char* args) {
        (void)id;
        (void)entry_time;

        char* group_name(my_strtok(args, ";"));
        contactgroup* group(configuration::applier::state::instance().find_contactgroup(group_name));
        if (!group)
          return ;

        for (std::unordered_map<std::string, contact *>::const_iterator
               it(group->get_members().begin()),
               end(group->get_members().end());
             it != end;
             ++it)
          if (it->second)
            (*fptr)(it->second);
      }

      std::unordered_map<std::string, command_info> _lst_command;
      mutable concurrency::mutex      _mutex;
    };
  }
}

CCE_END()

#endif // !CCE_MOD_EXTCMD_PROCESSING_HH
