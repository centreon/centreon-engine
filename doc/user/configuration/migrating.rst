Migrating from Nagios
*********************

While Nagios and Centreon Engine are mostly compatible (they share the
same configuration file format, internal logic, ...) some slight
differences occur.

Bug Report and Feature Request
==============================

Centreon Engine is based Nagios, which has shown over the years its
extreme robustness. However as any other piece of software Centreon
Engine might from time to time suffer from bugs. In those hopefully rare
cases, Merethis would be glad to receive bug reports from users which
are necessary for fast bug resolution.

Merethis also welcomes feature requests and code patches. Do not
hesitate to write to us !

Privilege Drop
==============

With Nagios, a user **had to** define a *nagios_user* variable and a
*nagios_group* variable in the main configuration file. These variables
were used by the program itself to drop privileges when the process was
started. Centreon Engine delegates this feature to the system
administrator. However he is not left alone, as Centreon Engine provide
some startup scripts for various platforms (please refer to
:ref:`Running Centreon Engine <running_start_stop>`
section for more information). User and group used by the init script
can be configured when compiling Centreon Engine as explained in the
:ref:`Compiling Centreon Engine <user_installation_using_sources_build>`
section.

Daemonization
=============

Nagios provides a *-d* flag that allows process to daemonize and run in
background. In Centreon Engine, this feature must be provided by the
user which runs Centreon Engine. However, init scripts handle the
daemonization of Centreon Engine on supported platforms.

As a result, the *daemon_dumps_core* variable is no longer supported. If
a user wish to coredump Centreon Engine, it has to be done as per his
operating system's manual.

Command Execution
=================

When executing commands, Nagios used to feed them to a shell for
command-line like parsing. This feature does not seem to be used a lot
and add an extra load on supervision servers that do not rely on it. As
a consequence, the shell execution has been removed from the execution
sequence. Users that wish to restore previous behavior must write a
shell script that will call their original command. They will then
replace the command in Centreon Engine with the shell script.

