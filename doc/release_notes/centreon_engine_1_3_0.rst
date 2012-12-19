===================
Centreon Engine 1.3
===================

**********
What's new
**********

Remove Qt library
=================

We decide to remove Qt library. For the XML part, we use xerces-c
library and for the other part we use Centreon-Clib library. This
replacement allow us to have a better integration with all unix system
and better performences.

Improve performence
===================

* Replace some algorithms (map by unordered map when is possible).
* The execution system uses less resources (possible to disable
  :ref:`use_setpgid <main_cfg_opt_use_setpgid>`).

Remove variables
================

Some variables are deprecated now:

  * `temp_file`, `temp_path`, `check_result_path` and
    `max_check_result_file_age` are use in nagios for get check result
    information from the file system, but in Centreon Engine the
    execution system write directly in memory.
  * `enable_embedded_perl`, `use_embedded_perl_implicitly`, `p1_file`
    and `auth_file` are use in nagios for the embedded perl, but this
    mechanism is provide into the Connector P with Centreon Engine.
  * `lock_file`, `nagios_user` and `nagios_group` are remove because
    these mechanism are use by the Centreon Engine startup script.
  * `bare_update_check` and `check_for_updates` are remove if you need
    to update Centreon Engine use your package manager.
  * `free_child_process_memory` and `child_processes_fork_twice` are
    remove. Centreon Engine fork juste once and child memory was always
    release by the system.
  * `daemon_dumps_core` is remove, you can create dumps core all the
    time if you have the rights.
  * `log_archive_path` and `log_rotation_method` are remove to use
    logrotate daemon.

Add logrotate script
====================

The log rotate mechanism as change. Now we use unix system to manage log
rotation base on logrotate daemon.


Improve Centreon compatibility
==============================

The status file change read permission to allow centreon to read it.
