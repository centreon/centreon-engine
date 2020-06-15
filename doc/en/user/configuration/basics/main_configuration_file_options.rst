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
finds in the rest of your configuration data to this file. If you have
:ref:`log rotation <main_cfg_opt_log_rotation_method>` enabled, this
file will automatically be rotated every hour, day, week, or month.

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
host groups, contacts, contact groups, services, commands, etc. You can
seperate your configuration information into several files and specify
multiple cfg_file= statements to have each of them processed.

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

.. _main_cfg_opt_object_cache_file:

Object Cache File
-----------------

This directive is used to specify a file in which a cached copy of
:ref:`object definitions <object_configuration_overview>`
should be stored. The cache file is (re)created every time Centreon
Engine is (re)started. It is intended to speed up config file caching
and allow you to edit the source
:ref:`object config files <main_cfg_opt_object_configuration_file>`
while Centreon Engine is running without affecting the output displayed.

=========== ========================================================
**Format**  object_cache_file=<file_name>
**Example** object_cache_file=/var/log/centreon-engine/objects.cache
=========== ========================================================

.. _main_cfg_opt_precached_object_file:

Precached Object File
---------------------

This directive is used to specify a file in which a pre-processed,
pre-cached copy of :ref:`object definitions <object_configuration_overview>`
should be stored. This file can be used to drastically improve startup
times in large/complex Centreon Engine installations. Read more
information on how to speed up start times
:ref:`here <fast_startup_options>`.

=========== ===============================================================
**Format**  precached_object_file=<file_name>
**Example** precached_object_file=/var/log/centreon-engine/objects.precache
=========== ===============================================================

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

.. _main_cfg_opt_temp_file:

Temp File
---------

This is a deprecated and ignored variable.

=========== =====================
**Format**  temp_file=<file_name>
=========== =====================

.. _main_cfg_opt_status_file:

Status File
-----------

This is the file that Centreon Engine uses to store the current status,
comment, and downtime information. This file is deleted every time
Centreon Engine stops and recreated when it starts.

=========== ===============================================
**Format**  status_file=<file_name>
**Example** status_file=/var/log/centreon-engine/status.dat
=========== ===============================================

Status File Update Interval
---------------------------

This setting determines how often (in seconds) that Centreon Engine will
update status data in the :ref:`status file <main_cfg_opt_status_file>`.
The minimum update interval is 1 second.

=========== ================================
**Format**  status_update_interval=<seconds>
**Example** status_update_interval=15
=========== ================================

.. _main_cfg_opt_notifications:

Notifications Option
--------------------

This option determines whether or not Centreon Engine will send out
:ref:`notifications <notifications>` when it initially (re)starts. If
this option is disabled, Centreon Engine will not send out notifications
for any host or service.

=========== ==========================
**Format**  enable_notifications=<0/1>
**Example** enable_notifications=1
=========== ==========================

.. note::
   If you have :ref:`state retention <main_cfg_opt_state_retention>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <main_cfg_opt_state_retention_file>`),
   unless you disable the :ref:`use_retained_program_state
   <main_cfg_opt_use_retained_program_state>`
   option. If you want to change this option when state retention is
   active (and the
   :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <external_commands>` or change it via the web
   interface. Values are as follows:

    * 0 = Disable notifications
    * 1 = Enable notifications (default)

.. _main_cfg_opt_service_check_execution:

Service Check Execution Option
------------------------------

This option determines whether or not Centreon Engine will execute
service checks when it initially (re)starts. If this option is disabled,
Centreon Engine will not actively execute any service checks and will
remain in a sort of "sleep" mode (it can still accept
:ref:`passive checks <passive_checks>` unless you've
:ref:`disabled them <main_cfg_opt_passive_service_check_acceptance>`).
This option is most often used when configuring backup monitoring
servers, as described in the documentation on
:ref:`redundancy <redundant_and_failover_monitoring>`,
or when setting up a :ref:`distributed <distributed_monitoring>`
monitoring environment.

=========== ============================
**Format**  execute_service_checks=<0/1>
**Example** execute_service_checks=1
=========== ============================

.. note::
   If you have :ref:`state retention <main_cfg_opt_state_retention>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <main_cfg_opt_state_retention_file>`),
   unless you disable the :ref:`use_retained_program_state
   <main_cfg_opt_use_retained_program_state>`
   option. If you want to change this option when state retention is
   active (and the :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Don't execute service checks
    * 1 = Execute service checks (default)

.. _main_cfg_opt_passive_service_check_acceptance:

Passive Service Check Acceptance Option
---------------------------------------

This option determines whether or not Centreon Engine will accept
:ref:`passive service checks <passive_checks>` when it initially
(re)starts. If this option is disabled, Centreon Engine will not accept
any passive service checks.

=========== ===================================
**Format**  accept_passive_service_checks=<0/1>
**Example** accept_passive_service_checks=1
=========== ===================================

.. note::
   If you have :ref:`state retention <main_cfg_opt_state_retention>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <main_cfg_opt_state_retention_file>`),
   unless you disable the :ref:`use_retained_program_state
   <main_cfg_opt_use_retained_program_state>`
   option. If you want to change this option when state retention is
   active (and the :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Don't accept passive service checks
    * 1 = Accept passive service checks (default)

Host Check Execution Option
---------------------------

This option determines whether or not Centreon Engine will execute
on-demand and regularly scheduled host checks when it initially
(re)starts. If this option is disabled, Centreon Engine will not
actively execute any host checks, although it can still accept
:ref:`passive host checks <passive_checks>` unless you've
:ref:`disabled them <main_cfg_opt_passive_host_check_acceptance>`).
This option is most often used when configuring backup monitoring
servers, as described in the documentation on
:ref:`redundancy <redundant_and_failover_monitoring>`,
or when setting up a :ref:`distributed <distributed_monitoring>`
monitoring environment.

=========== =====================================
**Format**  execute_host_checks=<0/1>
**Example** execute_host_checks=1
=========== =====================================

.. note::
   If you have :ref:`state retention <main_cfg_opt_state_retention>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <main_cfg_opt_state_retention_file>`),
   unless you disable the
   :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   option. If you want to change this option when state retention is
   active (and the :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Don't execute host checks
    * 1 = Execute host checks (default)

.. _main_cfg_opt_passive_host_check_acceptance:

Passive Host Check Acceptance Option
------------------------------------

This option determines whether or not Centreon Engine will accept
:ref:`passive host checks <passive_checks>` when it initially
(re)starts. If this option is disabled, Centreon Engine will not accept
any passive host checks.

=========== ================================
**Format**  accept_passive_host_checks=<0/1>
**Example** accept_passive_host_checks=1
=========== ================================

.. note::
   If you have :ref:`state retention <main_cfg_opt_state_retention>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <main_cfg_opt_state_retention_file>`),
   unless you disable the
   :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   option. If you want to change this option when state retention is
   active (and the
   :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Don't accept passive host checks
    * 1 = Accept passive host checks (default)

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
   :ref:`state retention file <main_cfg_opt_state_retention_file>`),
   unless you disable the
   :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   option. If you want to change this option when state retention is
   active (and the :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Disable event handlers
    * 1 = Enable event handlers (default)

.. _main_cfg_opt_log_rotation_method:

Log Rotation Method
-------------------

This is a deprecated and ignored variable. Use logrotate daemon.

=========== ===============================
**Format**  log_rotation_method=<n/h/d/w/m>
=========== ===============================

Log Archive Path
----------------

This is a deprecated and ignored variable.

=========== =======================
**Format**  log_archive_path=<path>
=========== =======================

.. _main_cfg_opt_external_command_check:

External Command Check Option
-----------------------------

This option determines whether or not Centreon Engine will check the
:ref:`command file <main_cfg_opt_external_command_file>`
for commands that should be executed. More information on external
commands can be found :ref:`here <external_commands>`.

  * 0 = Don't check external commands
  * 1 = Check external commands (default)

=========== =============================
**Format**  check_external_commands=<0/1>
**Example** check_external_commands=1
=========== =============================

.. _main_cfg_opt_external_command_check_interval:

External Command Check Interval
-------------------------------

If you specify a number with an "s" appended to it (i.e. 30s), this is
the number of seconds to wait between external command checks. If you
leave off the "s", this is the number of "time units" to wait between
external command checks. Unless you've changed the
:ref:`interval_length <main_cfg_opt_timing_interval_length>` value (as
defined below) from the default value of 60, this number will mean
minutes.

=========== ===============================
**Format**  command_check_interval=<xxx>[s]
**Example** command_check_interval=1
=========== ===============================

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

.. _main_cfg_opt_state_retention:

State Retention Option
----------------------

This option determines whether or not Centreon Engine will retain state
information for hosts and services between program restarts. If you
enable this option, you should supply a value for the
:ref:`state_retention_file <main_cfg_opt_state_retention_file>`
variable. When enabled, Centreon Engine will save all state information
for hosts and service before it shuts down (or restarts) and will read
in previously saved state information when it starts up again.

  * 0 = Don't retain state information
  * 1 = Retain state information (default)

=========== ==============================
**Format**  retain_state_information=<0/1>
**Example** retain_state_information=1
=========== ==============================

.. _main_cfg_opt_state_retention_file:

State Retention File
--------------------

This is the file that Centreon Engine will use for storing status,
downtime, and comment information before it shuts down. When Centreon
Engine is restarted it will use the information stored in this file for
setting the initial states of services and hosts before it starts
monitoring anything. In order to make Centreon Engine retain state
information between program restarts, you must enable the
:ref:`retain_state_information <main_cfg_opt_state_retention>`
option.

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
restarting. If you have disabled state retention (with the
:ref:`retain_state_information <main_cfg_opt_state_retention>`
option), this option has no effect.

=========== ===================================
**Format**  retention_update_interval=<minutes>
**Example** retention_update_interval=60
=========== ===================================

.. _main_cfg_opt_use_retained_program_state:

Use Retained Program State Option
---------------------------------

This setting determines whether or not Centreon Engine will set various
program-wide state variables based on the values saved in the retention
file. Some of these program-wide state variables that are normally saved
across program restarts if state retention is enabled include the
:ref:`enable_notifications <main_cfg_opt_notifications>`,
:ref:`enable_flap_detection <main_cfg_opt_flap_detection>`,
:ref:`enable_event_handlers <main_cfg_opt_event_handler>`,
:ref:`execute_service_checks <main_cfg_opt_service_check_execution>`,
and :ref:`accept_passive_service_checks <main_cfg_opt_passive_service_check_acceptance>`
options. If you do not have :ref:`state retention <main_cfg_opt_state_retention>`
enabled, this option has no effect.

  * 0 = Don't use retained program state
  * 1 = Use retained program state (default)

=========== ================================
**Format**  use_retained_program_state=<0/1>
**Example** use_retained_program_state=1
=========== ================================

.. _main_cfg_opt_use_retained_scheduling_info:

Use Retained Scheduling Info Option
-----------------------------------

This setting determines whether or not Centreon Engine will retain
scheduling info (next check times) for hosts and services when it
restarts. If you are adding a large number (or percentage) of hosts and
services, I would recommend disabling this option when you first restart
Centreon Engine, as it can adversely skew the spread of initial
checks. Otherwise you will probably want to leave it enabled.

  * 0 = Don't use retained scheduling info
  * 1 = Use retained scheduling info (default)

=========== ==================================
**Format**  use_retained_scheduling_info=<0/1>
**Example** use_retained_scheduling_info=1
=========== ==================================

Retained Host and Service Attribute Masks
-----------------------------------------

They are a deprecated and ignered variables.

=========== ========================================
**Format**  retained_host_attribute_mask=<number>
            retained_service_attribute_mask=<number>
=========== ========================================

Retained Process Attribute Masks
--------------------------------

They are a deprecated and ignered variables.

=========== ================================================
**Format**  retained_process_host_attribute_mask=<number>
            retained_process_service_attribute_mask=<number>
=========== ================================================

Retained Contact Attribute Masks
--------------------------------

These options determine which contact attributes are NOT retained across
program restarts. There are two masks because there are often separate
host and service contact attributes that can be changed. The values for
these options are a bitwise AND of values specified by the "MODATTR\_"
definitions in the include/common.h source code file. By default, all
process attributes are retained.

=========== ================================================
**Format**  retained_contact_host_attribute_mask=<number>
            retained_contact_service_attribute_mask=<number>
**Example** retained_contact_host_attribute_mask=0
            retained_contact_service_attribute_mask=0
=========== ================================================

.. note::
   This is an advanced feature. You'll need to read the Centreon Engine
   source code to use this option effectively.

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

Notification Logging Option
---------------------------

This variable determines whether or not notification messages are
logged. If you have a lot of contacts or regular service failures your
log file will grow relatively quickly. Use this option to keep contact
notifications from being logged.

  * 0 = Don't log notifications
  * 1 = Log notifications

=========== =======================
**Format**  log_notifications=<0/1>
**Example** log_notifications=1
=========== =======================

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

.. _main_cfg_opt_service_inter_check_delay_method:

Service Inter-Check Delay Method
--------------------------------

This option allows you to control how service checks are initially
"spread out" in the event queue. Using a "smart" delay calculation (the
default) will cause Centreon Engine to calculate an average check
interval and spread initial checks of all services out over that
interval, thereby helping to eliminate CPU load spikes. Using no delay
is generally not recommended, as it will cause all service checks to be
scheduled for execution at the same time. This means that you will
generally have large CPU spikes when the services are all executed in
parallel. More information on how to estimate how the inter-check delay
affects service check scheduling can be found
:ref:`here <scheduling_service_and_host>`. Values are as follows:

  * n = Don't use any delay - schedule all service checks to run
    immediately (i.e. at the same time!)
  * d = Use a "dumb" delay of 1 second between service checks
  * s = Use a "smart" delay calculation to spread service checks out
    evenly (default)
  * x.xx = Use a user-supplied inter-check delay of x.xx seconds

=========== =============================================
**Format**  service_inter_check_delay_method=<n/d/s/x.xx>
**Example** service_inter_check_delay_method=s
=========== =============================================

Maximum Service Check Spread
----------------------------

This option determines the maximum number of minutes from when Centreon
Engine starts that all services (that are scheduled to be regularly
checked) are checked. This option will automatically adjust the
:ref:`service <main_cfg_opt_service_inter_check_delay_method>`
inter-check delay method" (if necessary) to ensure that the initial
checks of all services occur within the timeframe you specify. In
general, this option will not have an affect on service check scheduling
if scheduling information is being retained using the
:ref:`use_retained_scheduling_info <main_cfg_opt_use_retained_scheduling_info>`
option. Default value is 30 (minutes).

=========== ==================================
**Format**  max_service_check_spread=<minutes>
**Example** max_service_check_spread=30
=========== ==================================

.. _main_cfg_opt_service_interleave_factor:

Service Interleave Factor
-------------------------

This variable determines how service checks are
interleaved. Interleaving allows for a more even distribution of service
checks, reduced load on remote hosts, and faster overall detection of
host problems. Setting this value to 1 is equivalent to not interleaving
the service checks (this is how versions of Centreon Engine previous to
0.0.5 worked). Set this value to s (smart) for automatic calculation of
the interleave factor unless you have a specific reason to change
it. You should see that the service check results are spread out as they
begin to appear. More information on how interleaving works can be found
:ref:`here <scheduling_service_and_host>`.

  * x = A number greater than or equal to 1 that specifies the
    interleave factor to use. An interleave factor of 1 is equivalent to
    not interleaving the service checks.
  * s = Use a "smart" interleave factor calculation (default)

=========== ===============================
**Format**  service_interleave_factor=<s|x>
**Example** service_interleave_factor=s
=========== ===============================

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

.. _main_cfg_opt_maximum_check_result_reaper_time:

Maximum Check Result Reaper Time
--------------------------------

This option allows you to control the maximum amount of time in seconds
that host and service check result "reaper" events are allowed to
run. "Reaper" events process the results from host and service checks
that have finished executing. If there are a lot of results to process,
reaper events may take a long time to finish, which might delay timely
execution of new host and service checks. This variable allows you to
limit the amount of time that an individual reaper event will run before
it hands control back over to Centreon Engine for other portions of the
monitoring logic.

=========== ======================================
**Format**  max_check_result_reaper_time=<seconds>
**Example** max_check_result_reaper_time=30
=========== ======================================

.. _main_cfg_opt_host_inter_check_delay_method:

Host Inter-Check Delay Method
-----------------------------

This option allows you to control how host checks that are scheduled to
be checked on a regular basis are initially "spread out" in the event
queue. Using a "smart" delay calculation (the default) will cause
Centreon Engine to calculate an average check interval and spread
initial checks of all hosts out over that interval, thereby helping to
eliminate CPU load spikes. Using no delay is generally not
recommended. Using no delay will cause all host checks to be scheduled
for execution at the same time. More information on how to estimate how
the inter-check delay affects host check scheduling can be found
:ref:`here <scheduling_service_and_host>`.Values are as follows:

  * n = Don't use any delay - schedule all host checks to run
    immediately (i.e. at the same time!)
  * d = Use a "dumb" delay of 1 second between host checks
  * s = Use a "smart" delay calculation to spread host checks out evenly
    (default)
  * x.xx = Use a user-supplied inter-check delay of x.xx seconds

=========== ==========================================
**Format**  host_inter_check_delay_method=<n/d/s/x.xx>
**Example** host_inter_check_delay_method=s
=========== ==========================================

Maximum Host Check Spread
-------------------------

This option determines the maximum number of minutes from when Centreon
Engine starts that all hosts (that are scheduled to be regularly
checked) are checked. This option will automatically adjust the
:ref:`host inter-check <main_cfg_opt_host_inter_check_delay_method>`
delay method" (if necessary) to ensure that the initial checks of all
hosts occur within the timeframe you specify. In general, this option
will not have an affect on host check scheduling if scheduling
information is being retained using the
:ref:`use_retained_scheduling_info <main_cfg_opt_use_retained_scheduling_info>`
option. Default value is 30 (minutes).

=========== ===============================
**Format**  max_host_check_spread=<minutes>
**Example** max_host_check_spread=30
=========== ===============================

.. _main_cfg_opt_timing_interval_length:

Timing Interval Length
----------------------

This is the number of seconds per "unit interval" used for timing in the
scheduling queue, re-notifications, etc. "Units intervals" are used in
the object configuration file to determine how often to run a service
check, how often to re-notify a contact, etc.

=========== =========================
**Format**  interval_length=<seconds>
**Example** interval_length=60
=========== =========================

.. note::
   The default value for this is set to 60, which means that a "unit
   value" of 1 in the object configuration file will mean 60 seconds (1
   minute). I have not really tested other values for this variable, so
   proceed at your own risk if you decide to do so!

.. _main_cfg_opt_auto_rescheduling:

Auto-Rescheduling Option
------------------------

This option determines whether or not Centreon Engine will attempt to
automatically reschedule active host and service checks to "smooth" them
out over time. This can help to balance the load on the monitoring
server, as it will attempt to keep the time between consecutive checks
consistent, at the expense of executing checks on a more rigid schedule.

=========== ============================
**Format**  auto_reschedule_checks=<0/1>
**Example** auto_reschedule_checks=1
=========== ============================

.. note::
   This is an experimental feature and may be removed in future
   versions. Enabling this option can degrade performance - rather than
   increase it - if used improperly!

Auto-Rescheduling Interval
--------------------------

This option determines how often (in seconds) Centreon Engine will
attempt to automatically reschedule checks. This option only has an
effect if the :ref:`auto_reschedule_checks <main_cfg_opt_auto_rescheduling>`
option is enabled. Default is 30 seconds.

=========== ====================================
**Format**  auto_rescheduling_interval=<seconds>
**Example** auto_rescheduling_interval=30
=========== ====================================

.. note::
   This is an experimental feature and may be removed in future
   versions. Enabling the auto-rescheduling option can degrade
   performance - rather than increase it - if used improperly!

Auto-Rescheduling Window
------------------------

This option determines the "window" of time (in seconds) that Centreon
Engine will look at when automatically rescheduling checks. Only host
and service checks that occur in the next X seconds (determined by this
variable) will be rescheduled. This option only has an effect if the
:ref:`auto_reschedule_checks <main_cfg_opt_auto_rescheduling>`
option is enabled. Default is 180 seconds (3 minutes).

=========== ==================================
**Format**  auto_rescheduling_window=<seconds>
**Example** auto_rescheduling_window=180
=========== ==================================

.. note::
   This is an experimental feature and may be removed in future
   versions. Enabling the auto-rescheduling option can degrade
   performance - rather than increase it - if used improperly!

.. _main_cfg_opt_aggressive_host_checking:

Aggressive Host Checking Option
-------------------------------

Centreon Engine tries to be smart about how and when it checks the
status of hosts. In general, disabling this option will allow Centreon
Engine to make some smarter decisions and check hosts a bit
faster. Enabling this option will increase the amount of time required
to check hosts, but may improve reliability a bit. Unless you have
problems with Centreon Engine not recognizing that a host recovered, I
would suggest not enabling this option.

  * 0 = Don't use aggressive host checking (default)
  * 1 = Use aggressive host checking

=========== ==================================
**Format**  use_aggressive_host_checking=<0/1>
**Example** use_aggressive_host_checking=0
=========== ==================================

.. _main_cfg_opt_translate_passive_host_checks:

Translate Passive Host Checks Option
------------------------------------

This option determines whether or not Centreon Engine will translate
DOWN/UNREACHABLE passive host check results to their "correct" state
from the viewpoint of the local Centreon Engine instance. This can be
very useful in distributed and failover monitoring installations. More
information on passive check state translation can be found
:ref:`here <passive_host_state_translation>`.

  * 0 = Disable check translation (default)
  * 1 = Enable check translation

=========== ===================================
**Format**  translate_passive_host_checks=<0/1>
**Example** translate_passive_host_checks=1
=========== ===================================

.. _main_cfg_opt_passive_host_checks_are_soft:

Passive Host Checks Are SOFT Option
-----------------------------------

This option determines whether or not Centreon Engine will treat
:ref:`passive host checks <passive_checks>` as HARD states or SOFT
states. By default, a passive host check result will put a host into a
:ref:`HARD state type <state_types>`. You can change this behavior by
enabling this option.

  * 0 = Passive host checks are HARD (default)
  * 1 = Passive host checks are SOFT

=========== ==================================
**Format**  passive_host_checks_are_soft=<0/1>
**Example** passive_host_checks_are_soft=1
=========== ==================================

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

.. _main_cfg_opt_large_installation_tweaks:

Large Installation Tweaks Option
--------------------------------

This option determines whether or not the Centreon Engine daemon will
take several shortcuts to improve performance. These shortcuts result in
the loss of a few features, but larger installations will likely see a
lot of benefit from doing so.

  * 0 = Don't use tweaks (default)
  * 1 = Use tweaks

=========== ===================================
**Format**  use_large_installation_tweaks=<0/1>
**Example** use_large_installation_tweaks=0
=========== ===================================

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

Child Process Memory Option
---------------------------

This is a deprecated and ignored variable.

=========== ===============================
**Format**  free_child_process_memory=<0/1>
=========== ===============================

Child Processes Fork Twice
--------------------------

This is a deprecated and ignored variable.

=========== ================================
**Format**  child_processes_fork_twice=<0/1>
=========== ================================

.. _main_cfg_opt_environment_macros:

Environment Macros Option
-------------------------

This option determines whether or not the Centreon Engine daemon will
make all standard :ref:`macros <standard_macros>` available as
environment variables to your check, notification, event hander,
etc. commands. In large Centreon Engine installations this can be
problematic because it takes additional memory and (more importantly)
CPU to compute the values of all macros and make them available to the
environment.

  * 0 = Don't make macros available as environment variables
  * 1 = Make macros available as environment variables (default)

=========== ===============================
**Format**  enable_environment_macros=<0/1>
**Example** enable_environment_macros=0
=========== ===============================

.. _main_cfg_opt_flap_detection:

Flap Detection Option
---------------------

This option determines whether or not Centreon Engine will try and
detect hosts and services that are "flapping". Flapping occurs when a
host or service changes between states too frequently, resulting in a
barrage of notifications being sent out. When Centreon Engine detects
that a host or service is flapping, it will temporarily suppress
notifications for that host/service until it stops flapping. Flap
detection is very experimental at this point, so use this feature with
caution! More information on how flap detection and handling works can
be found :ref:`here <flapping_detection>`.

=========== ===========================
**Format**  enable_flap_detection=<0/1>
**Example** enable_flap_detection=0
=========== ===========================

.. note::
   If you have :ref:`state retention <main_cfg_opt_state_retention>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <main_cfg_opt_state_retention_file>`),
   unless you disable the
   :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   option. If you want to change this option when state retention is
   active (and the
   :ref:`use_retained_program_state <main_cfg_opt_use_retained_program_state>`
   is enabled), you'll have to use the appropriate
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

.. _main_cfg_opt_notification_timeout:

Notification Timeout
--------------------

This is the maximum number of seconds that Centreon Engine will allow
notification commands to be run. If a notification command exceeds this
time limit it will be killed and a warning will be logged.

There is often widespread confusion as to what this option really
does. It is meant to be used as a last ditch mechanism to kill off
commands which are misbehaving and not exiting in a timely manner. It
should be set to something high (like 60 seconds or more), so that each
notification command finishes executing within this time limit. If a
notification command runs longer than this limit, Centreon Engine will
kill it off thinking it is a runaway processes.

=========== ==============================
**Format**  notification_timeout=<seconds>
**Example** notification_timeout=60
=========== ==============================

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

Performance Data Processor Command Timeout
------------------------------------------

This is the maximum number of seconds that Centreon Engine will allow a
:ref:`host performance data <main_cfg_opt_host_prefdata_processing_command>`
processor command" or
:ref:`service performance data processor command <main_cfg_opt_service_prefdata_processing_command>`
to be run. If a command exceeds this time limit it will be killed and a
warning will be logged.

=========== ==========================
**Format**  perfdata_timeout=<seconds>
**Example** perfdata_timeout=5
=========== ==========================

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
service check, which can be useful in
:ref:`distributed monitoring <distributed_monitoring>`. This
command is executed after any :ref:`event handler <event_handlers>`
or :ref:`notification <notifications>` commands. The command argument is
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
check, which can be useful in :ref:`distributed monitoring <distributed_monitoring>`.
This command is executed after any :ref:`event handler <event_handlers>`
or :ref:`notification <notifications>` commands. The command argument is
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

.. _main_cfg_opt_prefdata_processing:

Performance Data Processing Option
----------------------------------

This value determines whether or not Centreon Engine will process host
and service check :ref:`performance data <performance_data>`.

  * 0 = Don't process performance data (default)
  * 1 = Process performance data

=========== ==============================
**Format**  process_performance_data=<0/1>
**Example** process_performance_data=1
=========== ==============================

.. _main_cfg_opt_host_prefdata_processing_command:

Host Performance Data Processing Command
----------------------------------------

This option allows you to specify a command to be run after every host
check to process host :ref:`performance data <performance_data>`
that may be returned from the check. The command argument is the short
name of a :ref:`command <obj_def_command>`
definition" that you define in your object configuration file. This
command is only executed if the
:ref:`process_performance_data <main_cfg_opt_prefdata_processing>`
option is enabled globally and if the process_perf_data directive in the
:ref:`host definition <obj_def_host>`
is enabled.

=========== ===========================================
**Format**  host_perfdata_command=<command>
**Example** host_perfdata_command=process-host-perfdata
=========== ===========================================

.. _main_cfg_opt_service_prefdata_processing_command:

Service Performance Data Processing Command
-------------------------------------------

This option allows you to specify a command to be run after every
service check to process service :ref:`performance data <performance_data>`
that may be returned from the check. The command argument is the short
name of a :ref:`command definition <obj_def_command>`
that you define in your object configuration file. This command is only
executed if the :ref:`process_performance_data <main_cfg_opt_prefdata_processing>`
option is enabled globally and if the process_perf_data directive in the
:ref:`service definition <obj_def_service>`
is enabled.

=========== =================================================
**Format**  service_perfdata_command=<command>
**Example** service_perfdata_command=process-service-perfdata
=========== =================================================

.. _main_cfg_opt_host_prefdata_file:

Host Performance Data File
--------------------------

This option allows you to specify a file to which host
:ref:`performance data <performance_data>` will be written
after every host check. Data will be written to the performance file as
specified by the :ref:`host_perfdata_file_template <main_cfg_opt_host_prefdata_file_template>`
option. Performance data is only written to this file if the
:ref:`process_performance_data <main_cfg_opt_prefdata_processing>`
option is enabled globally and if the process_perf_data directive in the
:ref:`host definition <obj_def_host>`
is enabled.

=========== =============================================================
**Format**  host_perfdata_file=<file_name>
**Example** host_perfdata_file=/var/log/centreon-engine/host-perfdata.dat
=========== =============================================================

.. _main_cfg_opt_service_prefdata_file:

Service Performance Data File
-----------------------------

This option allows you to specify a file to which service
:ref:`performance data <performance_data>` will be written
after every service check. Data will be written to the performance file
as specified by the :ref:`service_perfdata_file_template <main_cfg_opt_service_prefdata_file_template>`
option. Performance data is only written to this file if the
:ref:`process_performance_data <main_cfg_opt_prefdata_processing>`
option is enabled globally and if the process_perf_data directive in the
:ref:`service definition <obj_def_service>`
is enabled.

=========== ===================================================================
**Format**  service_perfdata_file=<file_name>
**Example** service_perfdata_file=/var/log/centreon-engine/service-perfdata.dat
=========== ===================================================================

.. _main_cfg_opt_host_prefdata_file_template:

Host Performance Data File Template
-----------------------------------

This option determines what (and how) data is written to the
:ref:`host performance data file <main_cfg_opt_host_prefdata_file>`.
The template may contain :ref:`macros <understanding_macros>`,
special characters (\t for tab, \r for carriage return, \n for newline)
and plain text. A newline is automatically added after each write to the
performance data file.

=========== =======================================================================================
**Format**  host_perfdata_file_template=<template>
**Example** host_perfdata_file_template=[HOSTPERFDATA]\\t$HOSTNAME$\\t$HOSTOUTPUT$\\t$HOSTPERFDATA$
=========== =======================================================================================

.. _main_cfg_opt_service_prefdata_file_template:

Service Performance Data File Template
--------------------------------------

This option determines what (and how) data is written to the
:ref:`service performance data file <main_cfg_opt_service_prefdata_file>`.
The template may contain :ref:`macros <understanding_macros>`,
special characters (\t for tab, \r for carriage return, \n for newline)
and plain text. A newline is automatically added after each write to the
performance data file.

=========== ===================================================================================================================
**Format**  service_perfdata_file_template=<template>
**Example** service_perfdata_file_template=[SERVICEPERFDATA]\\t$HOSTNAME$\\t$SERVICEDESC$\\t$SERVICEOUTPUT$\\t$SERVICEPERFDATA$
=========== ===================================================================================================================

.. _main_cfg_opt_host_prefdata_file_mode:

Host Performance Data File Mode
-------------------------------

This option determines how the :ref:`host <main_cfg_opt_host_prefdata_file>`
performance data file" is opened. Unless the file is a named pipe you'll
probably want to use the default mode of append.

  * a = Open file in append mode (default)
  * w = Open file in write mode
  * p = Open in non-blocking read/write mode (useful when writing to
    pipes)

=========== ==============================
**Format**  host_perfdata_file_mode=<mode>
**Example** host_perfdata_file_mode=a
=========== ==============================

.. _main_cfg_opt_service_prefdata_file_mode:

Service Performance Data File Mode
----------------------------------

This option determines how the :ref:`service <main_cfg_opt_service_prefdata_file>`
performance data file" is opened. Unless the file is a named pipe you'll
probably want to use the default mode of append.

  * a = Open file in append mode (default)
  * w = Open file in write mode
  * p = Open in non-blocking read/write mode (useful when writing to
    pipes)

=========== =================================
**Format**  service_perfdata_file_mode=<mode>
**Example** service_perfdata_file_mode=a
=========== =================================

.. _main_cfg_opt_host_prefdata_file_processing_interval:

Host Performance Data File Processing Interval
----------------------------------------------

This option allows you to specify the interval (in seconds) at which the
:ref:`host performance data file <main_cfg_opt_host_prefdata_file>`
is processed using the :ref:`host performance data file <main_cfg_opt_host_prefdata_file_processing_command>`
processing command". A value of 0 indicates that the performance data
file should not be processed at regular intervals.

=========== ================================================
**Format**  host_perfdata_file_processing_interval=<seconds>
**Example** host_perfdata_file_processing_interval=0
=========== ================================================

.. _main_cfg_opt_service_prefdata_file_processing_interval:

Service Performance Data File Processing Interval
-------------------------------------------------

This option allows you to specify the interval (in seconds) at which the
:ref:`service performance data <main_cfg_opt_service_prefdata_file>`
file" is processed using the
:ref:`service performance data file processing command <main_cfg_opt_service_prefdata_file_processing_command>`.
A value of 0 indicates that the performance data file should not be
processed at regular intervals.

=========== ===================================================
**Format**  service_perfdata_file_processing_interval=<seconds>
**Example** service_perfdata_file_processing_interval=0
=========== ===================================================

.. _main_cfg_opt_host_prefdata_file_processing_command:

Host Performance Data File Processing Command
---------------------------------------------

This option allows you to specify the command that should be executed to
process the :ref:`host performance <main_cfg_opt_host_prefdata_file>`
data file". The command argument is the short name of a
:ref:`command definition <obj_def_command>`
that you define in your object configuration file. The interval at
which this command is executed is determined by the
:ref:`host_perfdata_file_processing_interval <main_cfg_opt_host_prefdata_file_processing_interval>`
directive.

=========== ================================================================
**Format**  host_perfdata_file_processing_command=<command>
**Example** host_perfdata_file_processing_command=process-host-perfdata-file
=========== ================================================================

.. _main_cfg_opt_service_prefdata_file_processing_command:

Service Performance Data File Processing Command
------------------------------------------------

This option allows you to specify the command that should be executed to
process the :ref:`service <main_cfg_opt_service_prefdata_file>`
performance data file". The command argument is the short name of a
:ref:`command definition <obj_def_command>`
that you define in your object configuration file. The interval at which
this command is executed is determined by the
:ref:`service_perfdata_file_processing_interval <main_cfg_opt_service_prefdata_file_processing_interval>`
directive.

=========== ======================================================================
**Format**  service_perfdata_file_processing_command=<command>
**Example** service_perfdata_file_processing_command=process-service-perfdata-file
=========== ======================================================================

Orphaned Service Check Option
-----------------------------

This option allows you to enable or disable checks for orphaned service
checks. Orphaned service checks are checks which have been executed and
have been removed from the event queue, but have not had any results
reported in a long time. Since no results have come back in for the
service, it is not rescheduled in the event queue. This can cause
service checks to stop being executed. Normally it is very rare for this
to happen - it might happen if an external user or process killed off
the process that was being used to execute a service check. If this
option is enabled and Centreon Engine finds that results for a
particular service check have not come back, it will log an error
message and reschedule the service check. If you start seeing service
checks that never seem to get rescheduled, enable this option and see if
you notice any log messages about orphaned services.

  * 0 = Don't check for orphaned service checks
  * 1 = Check for orphaned service checks (default)

=========== =================================
**Format**  check_for_orphaned_services=<0/1>
**Example** check_for_orphaned_services=1
=========== =================================

Orphaned Host Check Option
--------------------------

This option allows you to enable or disable checks for orphaned hoste
checks. Orphaned host checks are checks which have been executed and
have been removed from the event queue, but have not had any results
reported in a long time. Since no results have come back in for the
host, it is not rescheduled in the event queue. This can cause host
checks to stop being executed. Normally it is very rare for this to
happen - it might happen if an external user or process killed off the
process that was being used to execute a host check. If this option is
enabled and Centreon Engine finds that results for a particular host
check have not come back, it will log an error message and reschedule
the host check. If you start seeing host checks that never seem to get
rescheduled, enable this option and see if you notice any log messages
about orphaned hosts.

  * 0 = Don't check for orphaned host checks
  * 1 = Check for orphaned host checks (default)

=========== ==============================
**Format**  check_for_orphaned_hosts=<0/1>
**Example** check_for_orphaned_hosts=1
=========== ==============================

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

This setting determines how often (in seconds) Centreon Engine will
periodically check the "freshness" of service check results. If you have
disabled service freshness checking (with the
:ref:`check_service_freshness <main_cfg_opt_service_freshness_checking>`
option), this option has no effect. More information on freshness
checking can be found :ref:`here <freshness_checks>`.

=========== ==========================================
**Format**  service_freshness_check_interval=<seconds>
**Example** service_freshness_check_interval=60
=========== ==========================================

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

This setting determines how often (in seconds) Centreon Engine will
periodically check the "freshness" of host check results. If you have
disabled host freshness checking (with the
:ref:`check_host_freshness <main_cfg_opt_host_freshness_checking>`
option), this option has no effect. More information on freshness
checking can be found
:ref:`here <freshness_checks>`.

=========== =======================================
**Format**  host_freshness_check_interval=<seconds>
**Example** host_freshness_check_interval=60
=========== =======================================

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

.. _main_cfg_opt_date_format:

Date Format
-----------

This option allows you to specify what kind of date/time format Centreon
Engine should use in the web interface and date/time
:ref:`macros <understanding_macros>`. Possible options
(along with example output) include:

============== =================== ===================
Option         Output Format       Sample Output
============== =================== ===================
us             MM/DD/YYYY HH:MM:SS 06/30/2002 03:15:00
euro           DD/MM/YYYY HH:MM:SS 30/06/2002 03:15:00
iso8601        YYYY-MM-DD HH:MM:SS 2002-06-30 03:15:00
strict-iso8601 YYYY-MM-DDTHH:MM:SS 2002-06-30T03:15:00
============== =================== ===================

=========== ====================
**Format**  date_format=<option>
**Example** date_format=us
=========== ====================

Timezone Option
---------------

This option allows you to override the default timezone that this
instance of Centreon Engine runs in. Useful if you have multiple
instances of Centreon Engine that need to run from the same server, but
have different local times associated with them. If not specified,
Centreon Engine will use the system configured timezone.

=========== ========================
**Format**  use_timezone=<tz>
**Example** use_timezone=US/Mountain
=========== ========================

Illegal Object Name Characters
------------------------------

This option allows you to specify illegal characters that cannot be used
in host names, service descriptions, or names of other object
types. Centreon Engine will allow you to use most characters in object
definitions, but I recommend not using the characters shown in the
example above. Doing may give you problems in the web interface,
notification commands, etc.

=========== =============================================
**Format**  illegal_object_name_chars=<chars...>
**Example** illegal_object_name_chars=`~!$%^&*"\|'<>?,()=
=========== =============================================

.. _main_cfg_opt_illegal_macro_output_characters:

Illegal Macro Output Characters
-------------------------------

This option allows you to specify illegal characters that should be
stripped from :ref:`macros <understanding_macros>` before being used in
notifications, event handlers, and other commands. This DOES NOT affect
macros used in service or host check commands. You can choose to not
strip out the characters shown in the example above, but I recommend you
do not do this. Some of these characters are interpreted by the shell
(i.e. the backtick) and can lead to security problems. The following
macros are stripped of the characters you specify::

  $HOSTOUTPUT$, $HOSTPERFDATA$, $HOSTACKAUTHOR$, $HOSTACKCOMMENT$, $SERVICEOUTPUT$, $SERVICEPERFDATA$, $SERVICEACKAUTHOR$, and $SERVICEACKCOMMENT$

=========== ======================================
**Format**  illegal_macro_output_chars=<chars...>
**Example** illegal_macro_output_chars=`~$^&"\|'<>
=========== ======================================

.. _main_cfg_opt_regular_expression_matching:

Regular Expression Matching Option
----------------------------------

This option determines whether or not various directives in your
:ref:`object definitions <object_configuration_overview>` will be
processed as regular expressions. More information on how this works can
be found :ref:`here <obj_def_tricks>`.

  * 0 = Don't use regular expression matching (default)
  * 1 = Use regular expression matching

=========== =========================
**Format**  use_regexp_matching=<0/1>
**Example** use_regexp_matching=0
=========== =========================

.. _main_cfg_opt_true_regular_expression_matching:

True Regular Expression Matching Option
---------------------------------------

If you've enabled regular expression matching of various object
directives using the :ref:`use_regexp_matching <main_cfg_opt_regular_expression_matching>`
option, this option will determine when object directives are treated as
regular expressions. If this option is disabled (the default),
directives will only be treated as regular expressions if they contain
\*, ?, +, or \\.. If this option is enabled, all appropriate directives
will be treated as regular expression - be careful when enabling this!
More information on how this works can be found
:ref:`here <obj_def_tricks>`.

  * 0 = Don't use true regular expression matching (default)
  * 1 = Use true regular expression matching

=========== ==============================
**Format**  use_true_regexp_matching=<0/1>
**Example** use_true_regexp_matching=0
=========== ==============================

.. _main_cfg_opt_administrator_email_address:

Administrator Email Address
---------------------------

This is the email address for the administrator of the local machine
(i.e. the one that Centreon Engine is running on).

This value can be used in notification commands by using the
$ADMINEMAIL$ :ref:`macro <understanding_macros>`.

=========== ======================================
**Format**  admin_email=<email_address>
**Example** admin_email=root@localhost.localdomain
=========== ======================================

.. _main_cfg_opt_administrator_pager:

Administrator Pager
-------------------

This is the pager number (or pager email gateway) for the administrator
of the local machine (i.e. the one that Centreon Engine is running
on). The pager number/address can be used in notification commands by
using the $ADMINPAGER$ :ref:`macro <understanding_macros>`.

=========== =================================================
**Format**  admin_pager=<pager_number_or_pager_email_gateway>
**Example** admin_pager=pageroot@localhost.localdomain
=========== =================================================

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
  * 32 = Notification information
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
