.. _main_cfg_opt:

Main Configuration File Options
*******************************

Notes
=====

When creating and/or editing configuration files, keep the following in
mind:

  * Lines that start with a '#' character are taken to be comments and
    are not processed
  * Variables names must begin at the start of the line - no white space
    is allowed before the name
  * Variable names are case-sensitive

Sample Configuration File
=========================

.. note::
   A sample main configuration file
   ``/etc/centreon-engine/centengine.cfg`` is installed for you when you
   follow the :ref:`quickstart installation guide <exploit_quickstart>`.

Config File Location
====================

The main configuration file is usually named centengine.cfg and located
in the ``/etc/centreon-engine/`` directory.

Configuration File Variables
============================

Below you will find descriptions of each main Centreon Engine
configuration file option...

.. _main_cfg_opt_log_file:

Log File
--------

This variable specifies where Centreon Engine should create its main log
file. This should be the first variable that you define in your
configuration file, as Centreon Engine will try to write errors that it
finds in the rest of your configuration data to this file. Log rotation
must be accomplished by an external program like *logrotate*.

=========== ================================================
**Format**  log_file=<file_name>
**Example** log_file=/var/log/centreon-engine/centengine.log
=========== ================================================

.. _main_cfg_opt_object_configuration_file:

Object Configuration File
-------------------------

This directive is used to specify an
:ref:`object configuration file <object_configuration_overview>`
containing object definitions that Centreon Engine should use for
monitoring. Object configuration files contain definitions for hosts,
services, commands, etc. You can seperate your configuration
information into several files and specify multiple cfg_file=
statements to have each of them processed.

=========== ==========================================
**Format**  cfg_file=<file_name>
**Example** cfg_file=/etc/centreon-engine/hosts.cfg
            cfg_file=/etc/centreon-engine/services.cfg
            cfg_file=/etc/centreon-engine/commands.cfg
=========== ==========================================

.. _main_cfg_opt_object_configuration_directory:

Object Configuration Directory
------------------------------

This directive is used to specify a directory which contains
:ref:`object configuration files <object_configuration_overview>`
that Centreon Engine should use for monitoring. All files in the
directory with a .cfg extension are processed as object config
files. Additionally, Centreon Engine will recursively process all config
files in subdirectories of the directory you specify here. You can
seperate your configuration files into different directories and specify
multiple cfg_dir= statements to have all config files in each directory
processed.

=========== =====================================
**Format**  cfg_dir=<directory_name>
**Example** cfg_dir=/etc/centreon-engine/commands
            cfg_dir=/etc/centreon-engine/services
            cfg_dir=/etc/centreon-engine/hosts
=========== =====================================

.. _main_cfg_opt_resource_file:

Resource File
-------------

This is used to specify an optional resource file that can contain
$USERn$ :ref:`macro <understanding_macros>` definitions. $USERn$ macros
are useful for storing usernames, passwords, and items commonly used in
command definitions (like directory paths). You can include multiple
resource files by adding multiple resource_file statements to the main
config file - Centreon Engine will process them all. See the sample
resource.cfg file in the ``sample-config/`` subdirectory of the Centreon
Engine distribution for an example of how to define $USERn$ macros.

=========== ===============================================
**Format**  resource_file=<file_name>
**Example** resource_file=/etc/centreon-engine/resource.cfg
=========== ===============================================

.. _main_cfg_opt_status_file:

Status File
-----------

This is the file that Centreon Engine uses to store the current status
information. This file is no longer written at regular intervals.
Instead it is written in response to the SAVE_STATUS_INFORMATION
external command.

=========== ===============================================
**Format**  status_file=<file_name>
**Example** status_file=/var/log/centreon-engine/status.dat
=========== ===============================================

.. _main_cfg_opt_event_handler:

Event Handler Option
--------------------

This option determines whether or not Centreon Engine will run
:ref:`event handlers <event_handlers>` when it initially
(re)starts. If this option is disabled, Centreon Engine will not run any
host or service event handlers.

=========== ===========================
**Format**  enable_event_handlers=<0/1>
**Example** enable_event_handlers=1
=========== ===========================

.. note::
   If you have :ref:`state retention <main_cfg_opt_state_retention>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <main_cfg_opt_state_retention_file>`).
   If you want to change this option, you'll have to use the appropriate
   :ref:`external command <external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Disable event handlers
    * 1 = Enable event handlers (default)

.. _main_cfg_opt_external_command_check_interval:

External Command Check Interval
-------------------------------

This is the amount of time to wait between external command checks. The
default time time unit is the second (if you do not specify any suffix)
but some other suffixes are known such as "m" for minutes, "h" for
hours and "d" for days.

=========== =====================================
**Format**  command_check_interval=<xxx>[s,m,h,d]
**Example** command_check_interval=1
=========== =====================================

.. note::
   By setting this value to -1, Centreon Engine will check for external
   commands as often as possible. Each time Centreon Engine checks for
   external commands it will read and process all commands present in
   the :ref:`command file <main_cfg_opt_external_command_file>`
   before continuing on with its other duties. More information on
   external commands can be found :ref:`here <external_commands>`.

.. _main_cfg_opt_external_command_file:

External Command File
---------------------

This is the file that Centreon Engine will check for external commands
to process. The external command file is implemented as a named pipe
(FIFO), which is created when Centreon Engine starts and removed when it
shuts down. If the file exists when Centreon Engine starts, the Centreon
Engine process will terminate with an error message. More information on
external commands can be found :ref:`here <external_commands>`.

=========== =======================================================
**Format**  command_file=<file_name>
**Example** command_file=/var/log/centreon-engine/rw/centengine.cmd
=========== =======================================================

.. _main_cfg_opt_external_command_buffer_slots:

External Command Buffer Slots
-----------------------------

=========== =================================
**Format**  external_command_buffer_slots=<#>
**Example** external_command_buffer_slots=512
=========== =================================

.. note::
   This is an advanced feature. This option determines how many buffer
   slots Centreon Engine will reserve for caching external commands that
   have been read from the external command file by a worker thread, but
   have not yet been processed by the main thread of the Centreon Engine
   deamon. Each slot can hold one external command, so this option
   essentially determines how many commands can be buffered. For
   installations where you process a large number of passive checks
   (e.g. :ref:`distributed setups <distributed_monitoring>`),
   you may need to increase this number.

.. _main_cfg_opt_state_retention_file:

State Retention File
--------------------

This is the file that Centreon Engine will use for storing status
information before it shuts down. When Centreon Engine is restarted
it will use the information stored in this file for setting the initial
states of services and hosts before it starts monitoring anything.

=========== ===========================================================
**Format**  state_retention_file=<file_name>
**Example** state_retention_file=/var/log/centreon-engine/retention.dat
=========== ===========================================================

Automatic State Retention Update Interval
-----------------------------------------

This setting determines how often (in minutes) that Centreon Engine will
automatically save retention data during normal operation. If you set
this value to 0, Centreon Engine will not save retention data at regular
intervals, but it will still save retention data before shutting down or
restarting.

=========== ===================================
**Format**  retention_update_interval=<minutes>
**Example** retention_update_interval=60
=========== ===================================

Syslog Logging Option
---------------------

This variable determines whether messages are logged to the syslog
facility on your local host. Values are as follows:

  * 0 = Don't use syslog facility
  * 1 = Use syslog facility

=========== ================
**Format**  use_syslog=<0/1>
**Example** use_syslog=1
=========== ================

.. _main_cfg_opt_service_check_retry_logging:

Service Check Retry Logging Option
----------------------------------

This variable determines whether or not service check retries are
logged. Service check retries occur when a service check results in a
non-OK state, but you have configured Centreon Engine to retry the
service more than once before responding to the error. Services in this
situation are considered to be in "soft" states. Logging service check
retries is mostly useful when attempting to debug Centreon Engine or
test out service :ref:`event handlers <event_handlers>`.

  * 0 = Don't log service check retries
  * 1 = Log service check retries

=========== =========================
**Format**  log_service_retries=<0/1>
**Example** log_service_retries=1
=========== =========================

.. _main_cfg_opt_host_check_retry_logging:

Host Check Retry Logging Option
-------------------------------

This variable determines whether or not host check retries are
logged. Logging host check retries is mostly useful when attempting to
debug Centreon Engine or test out host
:ref:`event handlers <event_handlers>`.

  * 0 = Don't log host check retries
  * 1 = Log host check retries

=========== ======================
**Format**  log_host_retries=<0/1>
**Example** log_host_retries=1
=========== ======================

Event Handler Logging Option
----------------------------

This variable determines whether or not service and host
:ref:`event handlers <event_handlers>` are logged.

Event handlers are optional commands that can be run whenever a service
or hosts changes state. Logging event handlers is most useful when
debugging Centreon Engine or first trying out your event handler
scripts.

  * 0 = Don't log event handlers
  * 1 = Log event handlers

=========== ========================
**Format**  log_event_handlers=<0/1>
**Example** log_event_handlers=1
=========== ========================

Initial States Logging Option
-----------------------------

This variable determines whether or not Centreon Engine will force all
initial host and service states to be logged, even if they result in an
OK state. Initial service and host states are normally only logged when
there is a problem on the first check. Enabling this option is useful if
you are using an application that scans the log file to determine
long-term state statistics for services and hosts.

  * 0 = Don't log initial states (default)
  * 1 = Log initial states

=========== ========================
**Format**  log_initial_states=<0/1>
**Example** log_initial_states=1
=========== ========================

External Command Logging Option
-------------------------------

This variable determines whether or not Centreon Engine will log
:ref:`external commands <external_commands>` that it receives
from the :ref:`external command file <main_cfg_opt_external_command_file>`.

=========== ===========================
**Format**  log_external_commands=<0/1>
**Example** log_external_commands=1
=========== ===========================

.. note::
   This option does not control whether or not
   :ref:`passive service checks <passive_checks>`
   (which are a type of external command) get logged. To enable or
   disable logging of passive checks, use the
   :ref:`log_passive_checks <main_cfg_opt_passive_check_logging>`
   option.

    * 0 = Don't log external commands
    * 1 = Log external commands (default)

.. _main_cfg_opt_passive_check_logging:

Passive Check Logging Option
----------------------------

This variable determines whether or not Centreon Engine will log
:ref:`passive host and service checks <passive_checks>` that it receives
from the :ref:`external command file <main_cfg_opt_external_command_file>`.
If you are setting up a
:ref:`distributed monitoring environment <distributed_monitoring>`
or plan on handling a large number of passive checks on a regular basis,
you may wish to disable this option so your log file doesn't get too
large.

  * 0 = Don't log passive checks
  * 1 = Log passive checks (default)

=========== ========================
**Format**  log_passive_checks=<0/1>
**Example** log_passive_checks=1
=========== ========================

.. _main_cfg_opt_global_host_event_handler:

Global Host Event Handler Option
--------------------------------

This option allows you to specify a host event handler command that is
to be run for every host state change. The global event handler is
executed immediately prior to the event handler that you have optionally
specified in each host definition. The command argument is the short
name of a command that you define in your
:ref:`object configuration file <object_configuration_overview>`.
The maximum amount of time that this command can run is controlled by
the :ref:`event_handler_timeout <main_cfg_opt_event_handler_timeout>`
option. More information on event handlers can be found
:ref:`here <event_handlers>`.

=========== ==============================================
**Format**  global_host_event_handler=<command>
**Example** global_host_event_handler=log-host-event-to-db
=========== ==============================================

.. _main_cfg_opt_global_service_event_handler:

Global Service Event Handler Option
-----------------------------------

This option allows you to specify a service event handler command that
is to be run for every service state change. The global event handler is
executed immediately prior to the event handler that you have optionally
specified in each service definition. The command argument is the short
name of a command that you define in your
:ref:`object configuration file <object_configuration_overview>`.
The maximum amount of time that this command can run is controlled by
the :ref:`event_handler_timeout <main_cfg_opt_event_handler_timeout>`
option. More information on event handlers can be found
:ref:`here <event_handlers>`.

=========== ====================================================
**Format**  global_service_event_handler=<command>
**Example** global_service_event_handler=log-service-event-to-db
=========== ====================================================

Inter-Check Sleep Time
----------------------

This is the number of seconds that Centreon Engine will sleep before
checking to see if the next service or host check in the scheduling
queue should be executed.

=========== ====================
**Format**  sleep_time=<seconds>
**Example** sleep_time=1
=========== ====================

.. note::
   That Centreon Engine will only sleep after it "catches up" with
   queued service checks that have fallen behind.

.. _main_cfg_opt_maximum_concurrent_service_checks:

Maximum Concurrent Service Checks
---------------------------------

This option allows you to specify the maximum number of service checks
that can be run in parallel at any given time. Specifying a value of 1
for this variable essentially prevents any service checks from being run
in parallel. Specifying a value of 0 (the default) does not place any
restrictions on the number of concurrent checks. You'll have to modify
this value based on the system resources you have available on the
machine that runs Centreon Engine, as it directly affects the maximum
load that will be imposed on the system (processor utilization, memory,
etc.). More information on how to estimate how many concurrent checks
you should allow can be found :ref:`here <scheduling_service_and_host>`.

=========== ==================================
**Format**  max_concurrent_checks=<max_checks>
**Example** max_concurrent_checks=20
=========== ==================================

.. _main_cfg_opt_check_result_reaper_frequency:

Check Result Reaper Frequency
-----------------------------

This option allows you to control the frequency in seconds of check
result "reaper" events. "Reaper" events process the results from host
and service checks that have finished executing. These events consitute
the core of the monitoring logic in Centreon Engine.

=========== ====================================================
**Format**  check_result_reaper_frequency=<frequency_in_seconds>
**Example** check_result_reaper_frequency=5
=========== ====================================================

.. _main_cfg_opt_predictive_host_dependency_checks:

Predictive Host Dependency Checks Option
----------------------------------------

This option determines whether or not Centreon Engine will execute
predictive checks of hosts that are being depended upon (as defined in
:ref:`host <obj_def_host_dependency>` dependencies") for a particular
host when it changes state. Predictive checks help ensure that the
dependency logic is as accurate as possible. More information on how
predictive checks work can be found
:ref:`here <host_and_service_dependencies>`.

  * 0 = Disable predictive checks
  * 1 = Enable predictive checks (default)

=========== ==============================================
**Format**  enable_predictive_host_dependency_checks=<0/1>
**Example** enable_predictive_host_dependency_checks=1
=========== ==============================================

.. _main_cfg_opt_predictive_service_dependency_checks:

Predictive Service Dependency Checks Option
-------------------------------------------

This option determines whether or not Centreon Engine will execute
predictive checks of services that are being depended upon (as defined
in :ref:`service dependencies <obj_def_service_dependency>`)
for a particular service when it changes state. Predictive checks help
ensure that the dependency logic is as accurate as possible. More
information on how predictive checks work can be found
:ref:`here <host_and_service_dependencies>`.

  * 0 = Disable predictive checks
  * 1 = Enable predictive checks (default)

=========== =================================================
**Format**  enable_predictive_service_dependency_checks=<0/1>
**Example** enable_predictive_service_dependency_checks=1
=========== =================================================

.. _main_cfg_opt_cached_host_check_horizon:

Cached Host Check Horizon
-------------------------

This option determines the maximum amount of time (in seconds) that the
state of a previous host check is considered current. Cached host states
(from host checks that were performed more recently than the time
specified by this value) can improve host check performance
immensely. Too high of a value for this option may result in
(temporarily) inaccurate host states, while a low value may result in a
performance hit for host checks. Use a value of 0 if you want to disable
host check caching. More information on cached checks can be found
:ref:`here <cached_checks>`.

=========== ===================================
**Format**  cached_host_check_horizon=<seconds>
**Example** cached_host_check_horizon=15
=========== ===================================

.. _main_cfg_opt_cached_service_check_horizon:

Cached Service Check Horizon
----------------------------

This option determines the maximum amount of time (in seconds) that the
state of a previous service check is considered current. Cached service
states (from service checks that were performed more recently than the
time specified by this value) can improve service check performance when
a lot of :ref:`service dependencies <obj_def_service_dependency>`
are used. Too high of a value for this option may result in inaccuracies
in the service dependency logic. Use a value of 0 if you want to disable
service check caching. More information on cached checks can be found
:ref:`here <cached_checks>`.

=========== ======================================
**Format**  cached_service_check_horizon=<seconds>
**Example** cached_service_check_horizon=15
=========== ======================================

.. _main_cfg_opt_use_setpgid:

Use Setpgid
-----------

This option allow to change plugin process group into they own process
group id. This option protect Centreon Engine process from child miss
used or bug.

For example, if we use nagios check_ping, check_dns, check_dig or
check_rbl, don't disable this option, because, these checks can call
kill -KILL 0 on timeout (this is a bug from these plugins) and kill the
engine if the PGID is the same as the engine.

For maximum performance, this option must be disable.

  * 0 = Don't use setpgid
  * 1 = Use setpgid (default)

=========== =================
**Format**  use_setpgid=<0/1>
**Example** use_setpgid=1
=========== =================

.. _main_cfg_opt_flap_detection:

Flap Detection Option
---------------------

This option determines whether or not Centreon Engine will try and
detect hosts and services that are "flapping". Flapping occurs when a
host or service changes between states too frequently. When Centreon
Engine detects that a host or service is flapping, it will internally
flag it as being flapping until it stops flapping. Flap detection is
very experimental at this point, so use this feature with caution!
More information on how flap detection and handling works can be
found :ref:`here <flapping_detection>`.

=========== ===========================
**Format**  enable_flap_detection=<0/1>
**Example** enable_flap_detection=0
=========== ===========================

.. note::
   If you have :ref:`state retention <main_cfg_opt_state_retention>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <main_cfg_opt_state_retention_file>`).
   If you want to change, you'll have to use the appropriate
   :ref:`external command <external_commands>` or change it via
   the web interface.

    * 0 = Don't enable flap detection (default)
    * 1 = Enable flap detection

.. _main_cfg_opt_low_service_flap_threshold:

Low Service Flap Threshold
--------------------------

This option is used to set the low threshold for detection of service
flapping. For more information on how flap detection and handling works
(and how this option affects things) read
:ref:`this <flapping_detection>`.

=========== ====================================
**Format**  low_service_flap_threshold=<percent>
**Example** low_service_flap_threshold=25.0
=========== ====================================

.. _main_cfg_opt_high_service_flap_threshold:

High Service Flap Threshold
---------------------------

This option is used to set the high threshold for detection of service
flapping. For more information on how flap detection and handling works
(and how this option affects things) read
:ref:`this <flapping_detection>`.

=========== =====================================
**Format**  high_service_flap_threshold=<percent>
**Example** high_service_flap_threshold=50.0
=========== =====================================

.. _main_cfg_opt_low_host_flap_threshold:

Low Host Flap Threshold
-----------------------

This option is used to set the low threshold for detection of host
flapping. For more information on how flap detection and handling works
(and how this option affects things) read
:ref:`this <flapping_detection>`.

=========== =================================
**Format**  low_host_flap_threshold=<percent>
**Example** low_host_flap_threshold=25.0
=========== =================================

.. _main_cfg_opt_high_host_flap_threshold:

High Host Flap Threshold
------------------------

This option is used to set the high threshold for detection of host
flapping. For more information on how flap detection and handling works
(and how this option affects things) read
:ref:`this <flapping_detection>`.

=========== ==================================
**Format**  high_host_flap_threshold=<percent>
**Example** high_host_flap_threshold=50.0
=========== ==================================

.. _main_cfg_opt_soft_state_dependencies:

Soft State Dependencies Option
------------------------------

This option determines whether or not Centreon Engine will use soft
state information when checking
:ref:`host and service dependencies <host_and_service_dependencies>`.
Normally Centreon Engine will only use the latest hard host or service
state when checking dependencies. If you want it to use the latest state
(regardless of whether its a soft or hard
:ref:`state type <state_types>`), enable this option.

  * 0 = Don't use soft state dependencies (default)
  * 1 = Use soft state dependencies

=========== =============================
**Format**  soft_state_dependencies=<0/1>
**Example** soft_state_dependencies=0
=========== =============================

.. _main_cfg_opt_service_check_timeout:

Service Check Timeout
---------------------

This is the maximum number of seconds that Centreon Engine will allow
service checks to run. If checks exceed this limit, they are killed and
a CRITICAL state is returned. A timeout error will also be logged.

There is often widespread confusion as to what this option really
does. It is meant to be used as a last ditch mechanism to kill off
plugins which are misbehaving and not exiting in a timely manner. It
should be set to something high (like 60 seconds or more), so that each
service check normally finishes executing within this time limit. If a
service check runs longer than this limit, Centreon Engine will kill it
off thinking it is a runaway processes.

=========== ===============================
**Format**  service_check_timeout=<seconds>
**Example** service_check_timeout=60
=========== ===============================

.. _main_cfg_opt_host_check_timeout:

Host Check Timeout
------------------

This is the maximum number of seconds that Centreon Engine will allow
host checks to run. If checks exceed this limit, they are killed and a
CRITICAL state is returned and the host will be assumed to be DOWN. A
timeout error will also be logged.

There is often widespread confusion as to what this option really
does. It is meant to be used as a last ditch mechanism to kill off
plugins which are misbehaving and not exiting in a timely manner. It
should be set to something high (like 60 seconds or more), so that each
host check normally finishes executing within this time limit. If a host
check runs longer than this limit, Centreon Engine will kill it off
thinking it is a runaway processes.

=========== ============================
**Format**  host_check_timeout=<seconds>
**Example** host_check_timeout=60
=========== ============================

.. _main_cfg_opt_event_handler_timeout:

Event Handler Timeout
---------------------

This is the maximum number of seconds that Centreon Engine will allow
:ref:`event handlers <event_handlers>` to be run. If an event
handler exceeds this time limit it will be killed and a warning will be
logged.

There is often widespread confusion as to what this option really
does. It is meant to be used as a last ditch mechanism to kill off
commands which are misbehaving and not exiting in a timely manner. It
should be set to something high (like 60 seconds or more), so that each
event handler command normally finishes executing within this time
limit. If an event handler runs longer than this limit, Centreon Engine
will kill it off thinking it is a runaway processes.

=========== ===============================
**Format**  event_handler_timeout=<seconds>
**Example** event_handler_timeout=60
=========== ===============================

.. _main_cfg_opt_obsessive_compulsive_service_processor_timeout:

Obsessive Compulsive Service Processor Timeout
----------------------------------------------

This is the maximum number of seconds that Centreon Engine will allow an
:ref:`obsessive compulsive service processor <main_cfg_opt_obsessive_compulsive_service_processor_command>`
command" to be run. If a command exceeds this time limit it will be
killed and a warning will be logged.

=========== ======================
**Format**  ocsp_timeout=<seconds>
**Example** ocsp_timeout=5
=========== ======================

.. _main_cfg_opt_obsessive_compulsive_host_processor_timeout:

Obsessive Compulsive Host Processor Timeout
-------------------------------------------

This is the maximum number of seconds that Centreon Engine will allow an
:ref:`obsessive compulsive host processor <main_cfg_opt_obsessive_compulsive_host_processor_command>`
command" to be run. If a command exceeds this time limit it will be
killed and a warning will be logged.

=========== ======================
**Format**  ochp_timeout=<seconds>
**Example** ochp_timeout=5
=========== ======================

.. _main_cfg_opt_obsess_over_services:

Obsess Over Services Option
---------------------------

This value determines whether or not Centreon Engine will "obsess" over
service checks results and run the
:ref:`obsessive compulsive service processor command <main_cfg_opt_obsessive_compulsive_service_processor_command>`
you define. I know - funny name, but it was all I could think of. This
option is useful for performing
:ref:`distributed monitoring <distributed_monitoring>`.
If you're not doing distributed monitoring, don't enable this option.

  * 0 = Don't obsess over services (default)
  * 1 = Obsess over services

=========== ==========================
**Format**  obsess_over_services=<0/1>
**Example** obsess_over_services=1
=========== ==========================

.. _main_cfg_opt_obsessive_compulsive_service_processor_command:

Obsessive Compulsive Service Processor Command
----------------------------------------------

This option allows you to specify a command to be run after every
service check. This command is executed after any
:ref:`event handler <event_handlers>`. The command argument is
the short name of a :ref:`command definition <obj_def_command>`
that you define in your object configuration file. The maximum amount of
time that this command can run is controlled by the
:ref:`ocsp_timeout <main_cfg_opt_obsessive_compulsive_service_processor_timeout>`
option. More information on distributed monitoring can be found
:ref:`here <distributed_monitoring>`. This command is only
executed if the :ref:`obsess_over_services <main_cfg_opt_obsess_over_services>`
option is enabled globally and if the obsess_over_service directive in
the :ref:`service definition <obj_def_service>`
is enabled.

=========== ======================================
**Format**  ocsp_command=<command>
**Example** ocsp_command=obsessive_service_handler
=========== ======================================

.. _main_cfg_opt_obsess_over_hosts:

Obsess Over Hosts Option
------------------------

This value determines whether or not Centreon Engine will "obsess" over
host checks results and run the
:ref:`obsessive compulsive host processor command <main_cfg_opt_obsessive_compulsive_host_processor_command>`
you define. I know - funny name, but it was all I could think of. This
option is useful for performing
:ref:`distributed monitoring <distributed_monitoring>`. If
you're not doing distributed monitoring, don't enable this option.

  * 0 = Don't obsess over hosts (default)
  * 1 = Obsess over hosts

=========== =======================
**Format**  obsess_over_hosts=<0/1>
**Example** obsess_over_hosts=1
=========== =======================

.. _main_cfg_opt_obsessive_compulsive_host_processor_command:

Obsessive Compulsive Host Processor Command
-------------------------------------------

This option allows you to specify a command to be run after every host
check. This command is executed after any
:ref:`event handler <event_handlers>`. The command argument is
the short name of a :ref:`command definition <obj_def_command>`
that you define in your object configuration file. The maximum amount of
time that this command can run is controlled by the
:ref:`ochp_timeout <main_cfg_opt_obsessive_compulsive_host_processor_timeout>`
option. More information on distributed monitoring can be found
:ref:`here <distributed_monitoring>`. This command is only
executed if the :ref:`obsess_over_hosts <main_cfg_opt_obsess_over_hosts>`
option is enabled globally and if the obsess_over_host directive in the
:ref:`host definition <obj_def_host>`
is enabled.

=========== ===================================
**Format**  ochp_command=<command>
**Example** ochp_command=obsessive_host_handler
=========== ===================================

.. _main_cfg_opt_service_freshness_checking:

Service Freshness Checking Option
---------------------------------

This option determines whether or not Centreon Engine will periodically
check the "freshness" of service checks. Enabling this option is useful
for helping to ensure that :ref:`passive service checks <passive_checks>`
are received in a timely manner. More information on freshness checking
can be found :ref:`here <freshness_checks>`.

  * 0 = Don't check service freshness
  * 1 = Check service freshness (default)

=========== =============================
**Format**  check_service_freshness=<0/1>
**Example** check_service_freshness=0
=========== =============================

.. _main_cfg_opt_service_freshness_check_interval:

Service Freshness Check Interval
--------------------------------

This setting determines how often Centreon Engine will periodically check
the "freshness" of service check results. If you have disabled service
freshness checking (with the
:ref:`check_service_freshness <main_cfg_opt_service_freshness_checking>`
option), this option has no effect. More information on freshness
checking can be found :ref:`here <freshness_checks>`. The default time
unit is the second (if you specify no prefix) but other prefixes are
known, such as "m" for minutes, "h" for hours and "d" for days.

=========== ==================================================
**Format**  service_freshness_check_interval=<number>[s,m,h,d]
**Example** service_freshness_check_interval=60
=========== ==================================================

.. _main_cfg_opt_host_freshness_checking:

Host Freshness Checking Option
------------------------------

This option determines whether or not Centreon Engine will periodically
check the "freshness" of host checks. Enabling this option is useful for
helping to ensure that :ref:`passive host checks <passive_checks>` are
received in a timely manner. More information on freshness checking can
be found :ref:`here <freshness_checks>`.

  * 0 = Don't check host freshness
  * 1 = Check host freshness (default)

=========== ==========================
**Format**  check_host_freshness=<0/1>
**Example** check_host_freshness=0
=========== ==========================

.. _main_cfg_opt_host_freshness_check_interval:

Host Freshness Check Interval
-----------------------------

This setting determines how often Centreon Engine will periodically
check the "freshness" of host check results. If you have disabled
host freshness checking (with the
:ref:`check_host_freshness <main_cfg_opt_host_freshness_checking>`
option), this option has no effect. More information on freshness
checking can be found :ref:`here <freshness_checks>`. The default
time unit is the second (if you specify no prefix) but other
prefixes are known, such as "m" for minutes, "h" for hours and "d"
for days.

=========== ===============================================
**Format**  host_freshness_check_interval=<number>[s,m,h,d]
**Example** host_freshness_check_interval=60
=========== ===============================================

Additional Freshness Threshold Latency Option
---------------------------------------------

This option determines the number of seconds Centreon Engine will add to
any host or services freshness threshold it automatically calculates
(e.g. those not specified explicity by the user). More information on
freshness checking can be found
:ref:`here <freshness_checks>`.

=========== ================================
**Format**  additional_freshness_latency=<#>
**Example** additional_freshness_latency=15
=========== ================================

Timezone Option
---------------

This option allows you to override the default timezone that this
instance of Centreon Engine runs in. Useful if you have multiple
instances of Centreon Engine that need to run from the same server, but
have different local times associated with them. If not specified,
Centreon Engine will use the system configured timezone.

=========== ========================
**Format**  timezone=:<tz>
**Example** timezone=:US/Mountain
=========== ========================

Illegal Object Name Characters
------------------------------

This option allows you to specify illegal characters that cannot be used
in host names, service descriptions, or names of other object
types. Centreon Engine will allow you to use most characters in object
definitions, but I recommend not using the characters shown in the
example above. Doing may result in security issues.

=========== =============================================
**Format**  illegal_object_name_chars=<chars...>
**Example** illegal_object_name_chars=`~!$%^&*"\|'<>?,()=
=========== =============================================

.. _main_cfg_opt_illegal_macro_output_characters:

Illegal Macro Output Characters
-------------------------------

This option allows you to specify illegal characters that should be
stripped from :ref:`macros <understanding_macros>` before being used in
event handlers and other commands. This DOES NOT affect macros used in
service or host check commands. You can choose to not strip out the
characters shown in the example above, but I recommend you do not do
this. Some of these characters are interpreted by the shell
(i.e. the backtick) and can lead to security problems. The following
macros are stripped of the characters you specify::

  $HOSTOUTPUT$, $HOSTPERFDATA$, $HOSTACKAUTHOR$, $HOSTACKCOMMENT$, $SERVICEOUTPUT$, $SERVICEPERFDATA$, $SERVICEACKAUTHOR$, and $SERVICEACKCOMMENT$

=========== ======================================
**Format**  illegal_macro_output_chars=<chars...>
**Example** illegal_macro_output_chars=`~$^&"\|'<>
=========== ======================================

Event Broker Options
--------------------

This option controls what (if any) data gets sent to the event broker
and, in turn, to any loaded event broker modules. This is an advanced
option. When in doubt, either broker nothing (if not using event broker
modules) or broker everything (if using event broker modules). Possible
values are shown below.

  * 0 = Broker nothing
  * -1 = Broker everything
  * # = See BROKER_* definitions in source code (include/broker.h) for
      other values that can be OR'ed together

=========== ========================
**Format**  event_broker_options=<#>
**Example** event_broker_options=-1
=========== ========================

Event Broker Modules
--------------------

This directive is used to specify an event broker module that should by
loaded by Centreon Engine at startup. Use multiple directives if you
want to load more than one module. Arguments that should be passed to
the module at startup are seperated from the module path by a space.

=========== ================================================
**Format**  broker_module=<modulepath> [moduleargs]
**Example** broker_module=/usr/local/centengine/bin/ndomod.o
            cfg_file=/etc/centreon-engine/ndomod.cfg
=========== ================================================

.. note::
   Do NOT overwrite modules while they are being used by Centreon Engine
   or Centreon Engine will crash in a fiery display of SEGFAULT
   glory. This is a bug/limitation either in dlopen(), the kernel,
   and/or the filesystem. And maybe Centreon Engine...

The correct/safe way of updating a module is by using one of these
methods:

  * Shutdown Centreon Engine, replace the module file, restart Centreon
    Engine
  * While Centreon Engine is running... delete the original module file,
    move the new module file into place, restart Centreon Engine

.. _main_cfg_opt_debug_file:

Debug File
----------

This option determines where Centreon Engine should write debugging
information. What (if any) information is written is determined by the
:ref:`debug_level <main_cfg_opt_debug_file>` and
:ref:`debug_verbosity <main_cfg_opt_debug_verbosity>` options. You can
have Centreon Engine automaticaly rotate the debug file when it reaches
a certain size by using the
:ref:`max_debug_file_size <main_cfg_opt_max_debug_file_size>` option.

=========== ====================================================
**Format**  debug_file=<file_name>
**Example** debug_file=/var/log/centreon-engine/centengine.debug
=========== ====================================================

.. _main_cfg_opt_debug_level:

Debug Level
-----------

This option determines what type of information Centreon Engine should
write to the :ref:`debug_file <main_cfg_opt_debug_file>`.  This value is
a logical OR of the values below.

  * -1 = Log everything
  * 0 = Log nothing (default)
  * 1 = Function enter/exit information
  * 2 = Config information
  * 4 = Process information
  * 8 = Scheduled event information
  * 16 = Host/service check information
  * 64 = Event broker information

=========== ===============
**Format**  debug_level=<#>
**Example** debug_level=24
=========== ===============

.. _main_cfg_opt_debug_verbosity:

Debug Verbosity
---------------

This option determines how much debugging information Centreon Engine
should write to the :ref:`debug_file <main_cfg_opt_debug_file>`.

  * 0 = Basic information
  * 1 = More detailed information (default)
  * 2 = Highly detailed information

=========== ===================
**Format**  debug_verbosity=<#>
**Example** debug_verbosity=1
=========== ===================

.. _main_cfg_opt_max_debug_file_size:

Maximum Debug File Size
-----------------------

This option determines the maximum size (in bytes) of the
:ref:`debug file <main_cfg_opt_debug_file>`.  If the file grows larger
than this size, it will be renamed with a .old extension. If a file
already exists with a .old extension it will automatically be
deleted. This helps ensure your disk space usage doesn't get out of
control when debugging Centreon Engine.

=========== ===========================
**Format**  max_debug_file_size=<#>
**Example** max_debug_file_size=1000000
=========== ===========================
