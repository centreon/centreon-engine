===================
Centreon Engine 1.3
===================

**********
What's new
**********

Qt library removal
==================

We decided to stop using the Qt library. For the XML part, we now use
the *xerces-c* library and for the other part, we use the
*Centreon-Clib* library. This replacement allows us to have a better
integration with all UNIX systems and better performance.

Performance improvement
=======================

* Some algorithms were replaced (*map* instead of *unordered map* when possible)
* The execution system now uses less resource (possibility to disable
  :ref:`use_setpgid <main_cfg_opt_use_setpgid>`)

Variables deprecation
=====================

Some variables have been deprecated:

* `temp_file`, `temp_path`, `check_result_path` and
  `max_check_result_file_age` are used in *nagios* to get check result
  information from the file system, but in Centreon Engine the
  execution system writes directly in memory
* `enable_embedded_perl`, `use_embedded_perl_implicitly`, `p1_file`
  and `auth_file` are used in nagios by the embedded perl
  interpretor. In Centreon Engine, this mechanism is provided into the
  Perl connector.  Engine.
* `lock_file`, `nagios_user` and `nagios_group` are removed because
  these mechanisms are not used by the Centreon Engine startup script
* `bare_update_check` and `check_for_updates` are removed. If you need
  to update Centreon Engine, use your package manager
* `free_child_process_memory` and `child_processes_fork_twice` are
  removed. Centreon Engine forks just once and child memory is always
  released by the system
* `daemon_dumps_core` is removed, you can create core dumps all the
  time if you are allowed to
* `log_archive_path` and `log_rotation_method` are removed. Centreon
  Engine relies on standard *logrotate* daemons

New *logrotate* script
======================

The *logrotate* mechanism has changed. Now we use the classical UNIX
mechanism to manage log rotation (ie. based on *logrotate* daemons).

Improved Centreon compatibility
===============================

Default permissions on the status file have been changed to allow
Centreon reading it.
