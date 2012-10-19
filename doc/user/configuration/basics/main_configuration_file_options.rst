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
   (/etc/centreon-engine/centengine.cfg) is installed for you when you
   follow the :ref:`quickstart installation guide <get_started/quickstart_installation_guide>`.

Config File Location
====================

The main configuration file is usually named centengine.cfg and located
in the /etc/centreon-engine/ directory.

Configuration File Variables
============================

Below you will find descriptions of each main Centreon Engine
configuration file option...

Log File
--------

Format:  log_file=<file_name>
Example: log_file=/var/log/centreon-engine/centengine.log

This variable specifies where Centreon Engine should create its main log
file. This should be the first variable that you define in your
configuration file, as Centreon Engine will try to write errors that it
finds in the rest of your configuration data to this file. If you have
:ref:`log rotation <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableslogrotationmethod>`
enabled, this file will automatically be rotated every hour, day, week,
or month.

Object Configuration File
-------------------------

Format:  cfg_file=<file_name>
Example: cfg_file=/etc/centreon-engine/hosts.cfg
         cfg_file=/etc/centreon-engine/services.cfg
         cfg_file=/etc/centreon-engine/commands.cfg

This directive is used to specify an
:ref:`object configuration file <object_configuration_overview>`
containing object definitions that Centreon Engine should use for
monitoring. Object configuration files contain definitions for hosts,
host groups, contacts, contact groups, services, commands, etc. You can
seperate your configuration information into several files and specify
multiple cfg_file= statements to have each of them processed.

Object Configuration Directory
------------------------------

Format:  cfg_dir=<directory_name>
Example: cfg_dir=/etc/centreon-engine/commands
         cfg_dir=/etc/centreon-engine/services
         cfg_dir=/etc/centreon-engine/hosts

This directive is used to specify a directory which contains
:ref:`object configuration files <object_configuration_overview>`
that Centreon Engine should use for monitoring. All files in the
directory with a .cfg extension are processed as object config
files. Additionally, Centreon Engine will recursively process all config
files in subdirectories of the directory you specify here. You can
seperate your configuration files into different directories and specify
multiple cfg_dir= statements to have all config files in each directory
processed.

Object Cache File
-----------------

Format:  object_cache_file=<file_name>
Example: object_cache_file=/var/log/centreon-engine/objects.cache

This directive is used to specify a file in which a cached copy of
:ref:`object definitions <object_configuration_overview>`
should be stored. The cache file is (re)created every time Centreon
Engine is (re)started. It is intended to speed up config file caching
and allow you to edit the source
:ref:`object config files <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobjectconfigurationfile>`
while Centreon Engine is running without affecting the output displayed.

Precached Object File
---------------------

Format:  precached_object_file=<file_name>
Example: precached_object_file=/var/log/centreon-engine/objects.precache

This directive is used to specify a file in which a pre-processed,
pre-cached copy of :ref:`object definitions <object_configuration_overview>`
should be stored. This file can be used to drastically improve startup
times in large/complex Centreon Engine installations. Read more
information on how to speed up start times
:ref:`here <advanced_fast_startup_options>`.

Resource File
-------------

Format:  resource_file=<file_name>
Example: resource_file=/etc/centreon-engine/resource.cfg

This is used to specify an optional resource file that can contain
$USERn$ :ref:`macro <understanding_macros_and_how_they_work>`
definitions. $USERn$ macros are useful for storing usernames, passwords,
and items commonly used in command definitions (like directory
paths). You can include multiple resource files by adding multiple
resource_file statements to the main config file - Centreon Engine will
process them all. See the sample resource.cfg file in the sample-config/
subdirectory of the Centreon Engine distribution for an example of how
to define $USERn$ macros.

Temp File
---------

Format:  temp_file=<file_name>

This is a deprecated and ignored variable.

Status File
-----------

Format:  status_file=<file_name>
Example: status_file=/var/log/centreon-engine/status.dat

This is the file that Centreon Engine uses to store the current status,
comment, and downtime information. This file is deleted every time
Centreon Engine stops and recreated when it starts.

Status File Update Interval
---------------------------

Format:  status_update_interval=<seconds>
Example: status_update_interval=15

This setting determines how often (in seconds) that Centreon Engine will
update status data in the
:ref:`status file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstatusfile>`.
The minimum update interval is 1 second.

Notifications Option
--------------------

Format:  enable_notifications=<0/1>
Example: enable_notifications=1

This option determines whether or not Centreon Engine will send out
:ref:`notifications <notifications>` when it initially (re)starts. If
this option is disabled, Centreon Engine will not send out notifications
for any host or service.

.. note::

   If you have :ref:`state retention <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionfile>`),
   unless you disable the :ref:`use_retained_program_state
   <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   option. If you want to change this option when state retention is
   active (and the :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <advanced_external_commands>`
   or change it via the web interface. Values are as follows:

    * 0 = Disable notifications
    * 1 = Enable notifications (default)

Service Check Execution Option
------------------------------

Format:  execute_service_checks=<0/1>
Example: execute_service_checks=1

This option determines whether or not Centreon Engine will execute
service checks when it initially (re)starts. If this option is disabled,
Centreon Engine will not actively execute any service checks and will
remain in a sort of "sleep" mode (it can still accept
:ref:`passive checks <passive_checks>` unless you've
:ref:`disabled them <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablespassiveservicecheckacceptanceoption>`).
This option is most often used when configuring backup monitoring
servers, as described in the documentation on
:ref:`redundancy <advanced_redundant_and_failover_network_monitoring>`,
or when setting up a :ref:`distributed <advanced_distributed_monitoring>`
monitoring environment.

.. note::

   If you have :ref:`state retention <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionfile>`),
   unless you disable the :ref:`use_retained_program_state
   <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   option. If you want to change this option when state retention is
   active (and the :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <advanced_external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Don't execute service checks
    * 1 = Execute service checks (default)

Passive Service Check Acceptance Option
---------------------------------------

Format:  accept_passive_service_checks=<0/1>
Example: accept_passive_service_checks=1

This option determines whether or not Centreon Engine will accept
:ref:`passive service checks <passive_checks>` when it initially
(re)starts. If this option is disabled, Centreon Engine will not accept
any passive service checks.

.. note::

   If you have :ref:`state retention <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionfile>`),
   unless you disable the :ref:`use_retained_program_state
   <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   option. If you want to change this option when state retention is
   active (and the :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <advanced_external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Don't accept passive service checks
    * 1 = Accept passive service checks (default)

Host Check Execution Option
---------------------------

Format:  execute_host_checks=<0/1>
Example: execute_host_checks=1

This option determines whether or not Centreon Engine will execute
on-demand and regularly scheduled host checks when it initially
(re)starts. If this option is disabled, Centreon Engine will not
actively execute any host checks, although it can still accept
:ref:`passive host checks <passive_checks>` unless you've
:ref:`disabled them <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablespassivehostcheckacceptanceoption>`).
This option is most often used when configuring backup monitoring
servers, as described in the documentation on
:ref:`redundancy <advanced_redundant_and_failover_network_monitoring>`,
or when setting up a :ref:`distributed <advanced_distributed_monitoring>`
monitoring environment.

.. note::

   If you have :ref:`state retention <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
    :ref:`state retention file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionfile>`),
   unless you disable the
   :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   option. If you want to change this option when state retention is
   active (and the :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <advanced_external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Don't execute host checks
    * 1 = Execute host checks (default)

Passive Host Check Acceptance Option
------------------------------------

Format: accept_passive_host_checks=<0/1>
Example: accept_passive_host_checks=1

This option determines whether or not Centreon Engine will accept
:ref:`passive host checks <passive_checks>` when it initially
(re)starts. If this option is disabled, Centreon Engine will not accept
any passive host checks.

.. note::

   If you have :ref:`state retention <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionfile>`),
   unless you disable the
   :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   option. If you want to change this option when state retention is
   active (and the
   :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <advanced_external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Don't accept passive host checks
    * 1 = Accept passive host checks (default)

Event Handler Option
--------------------

Format:  enable_event_handlers=<0/1>
Example: enable_event_handlers=1

This option determines whether or not Centreon Engine will run
:ref:`event handlers <advanced_event_handlers>` when it initially
(re)starts. If this option is disabled, Centreon Engine will not run any
host or service event handlers.

.. note::

   If you have :ref:`state retention <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionfile>`),
   unless you disable the
   :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   option. If you want to change this option when state retention is
   active (and the :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <advanced_external_commands>` or change it via
   the web interface. Values are as follows:

    * 0 = Disable event handlers
    * 1 = Enable event handlers (default)

Log Rotation Method
-------------------

Format:  log_rotation_method=<n/h/d/w/m>

This is a deprecated and ignored variable. Use logrotate daemon.

Log Archive Path
----------------

Format:  log_archive_path=<path>

This is a deprecated and ignored variable.

External Command Check Option
-----------------------------

Format:  check_external_commands=<0/1>
Example: check_external_commands=1

This option determines whether or not Centreon Engine will check the
:ref:`command file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesexternalcommandfile>`
for commands that should be executed. More information on external
commands can be found :ref:`here <advanced_external_commands>`.

  * 0 = Don't check external commands
  * 1 = Check external commands (default)

External Command Check Interval
-------------------------------

Format:  command_check_interval=<xxx>[s]
Example: command_check_interval=1

If you specify a number with an "s" appended to it (i.e. 30s), this is
the number of seconds to wait between external command checks. If you
leave off the "s", this is the number of "time units" to wait between
external command checks. Unless you've changed the
:ref:`interval_length <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablestimingintervallength>`
value (as defined below) from the default value of 60, this number will
mean minutes.

.. note::

   By setting this value to -1, Centreon Engine will check for external
   commands as often as possible. Each time Centreon Engine checks for
   external commands it will read and process all commands present in
   the :ref:`command file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesexternalcommandfile>`
   before continuing on with its other duties. More information on
   external commands can be found :ref:`here <advanced_external_commands>`.

External Command File
---------------------

Format:  command_file=<file_name>
Example: command_file=/var/log/centreon-engine/rw/centengine.cmd

This is the file that Centreon Engine will check for external commands
to process. The external command file is implemented as a named pipe
(FIFO), which is created when Centreon Engine starts and removed when it
shuts down. If the file exists when Centreon Engine starts, the Centreon
Engine process will terminate with an error message. More information on
external commands can be found :ref:`here <advanced_external_commands>`.

External Command Buffer Slots
-----------------------------

Format:  external_command_buffer_slots=<#>
Example: external_command_buffer_slots=512

.. note::

   This is an advanced feature. This option determines how many buffer
   slots Centreon Engine will reserve for caching external commands that
   have been read from the external command file by a worker thread, but
   have not yet been processed by the main thread of the Centreon Engine
   deamon. Each slot can hold one external command, so this option
   essentially determines how many commands can be buffered. For
   installations where you process a large number of passive checks
   (e.g. :ref:`distributed setups <advanced_distributed_monitoring>`),
   you may need to increase this number.

State Retention Option
----------------------

Format:  retain_state_information=<0/1>
Example: retain_state_information=1

This option determines whether or not Centreon Engine will retain state
information for hosts and services between program restarts. If you
enable this option, you should supply a value for the
:ref:`state_retention_file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionfile>`
variable. When enabled, Centreon Engine will save all state information
for hosts and service before it shuts down (or restarts) and will read
in previously saved state information when it starts up again.

  * 0 = Don't retain state information
  * 1 = Retain state information (default)

State Retention File
--------------------

Format:  state_retention_file=<file_name>
Example: state_retention_file=/var/log/centreon-engine/retention.dat

This is the file that Centreon Engine will use for storing status,
downtime, and comment information before it shuts down. When Centreon
Engine is restarted it will use the information stored in this file for
setting the initial states of services and hosts before it starts
monitoring anything. In order to make Centreon Engine retain state
information between program restarts, you must enable the
:ref:`retain_state_information <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
option.

Automatic State Retention Update Interval
-----------------------------------------

Format:  retention_update_interval=<minutes>
Example: retention_update_interval=60

This setting determines how often (in minutes) that Centreon Engine will
automatically save retention data during normal operation. If you set
this value to 0, Centreon Engine will not save retention data at regular
intervals, but it will still save retention data before shutting down or
restarting. If you have disabled state retention (with the
:ref:`retain_state_information <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
option), this option has no effect.

Use Retained Program State Option
---------------------------------

Format:  use_retained_program_state=<0/1>
Example: use_retained_program_state=1

This setting determines whether or not Centreon Engine will set various
program-wide state variables based on the values saved in the retention
file. Some of these program-wide state variables that are normally saved
across program restarts if state retention is enabled include the
:ref:`enable_notifications <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesnotificationsoption>`,
:ref:`enable_flap_detection <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesflapdetectionoption>`,
:ref:`enable_event_handlers <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableseventhandleroption>`,
:ref:`execute_service_checks <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesservicecheckexecutionoption>`,
and :ref:`accept_passive_service_checks <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablespassiveservicecheckacceptanceoption>`
options. If you do not have :ref:`state retention <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
enabled, this option has no effect.

  * 0 = Don't use retained program state
  * 1 = Use retained program state (default)

Use Retained Scheduling Info Option
-----------------------------------

Format:  use_retained_scheduling_info=<0/1>
Example: use_retained_scheduling_info=1

This setting determines whether or not Centreon Engine will retain
scheduling info (next check times) for hosts and services when it
restarts. If you are adding a large number (or percentage) of hosts and
services, I would recommend disabling this option when you first restart
Centreon Engine, as it can adversely skew the spread of initial
checks. Otherwise you will probably want to leave it enabled.

  * 0 = Don't use retained scheduling info
  * 1 = Use retained scheduling info (default)

Retained Host and Service Attribute Masks
-----------------------------------------

Format:  retained_host_attribute_mask=<number>
         retained_service_attribute_mask=<number>

They are a deprecated and ignered variables.

Retained Process Attribute Masks
--------------------------------

Format:  retained_process_host_attribute_mask=<number>
         retained_process_service_attribute_mask=<number>

They are a deprecated and ignered variables.

Retained Contact Attribute Masks
--------------------------------

Format:  retained_contact_host_attribute_mask=<number>
         retained_contact_service_attribute_mask=<number>
Example: retained_contact_host_attribute_mask=0
         retained_contact_service_attribute_mask=0

.. note::

   This is an advanced feature. You'll need to read the Centreon Engine
   source code to use this option effectively.

These options determine which contact attributes are NOT retained across
program restarts. There are two masks because there are often separate
host and service contact attributes that can be changed. The values for
these options are a bitwise AND of values specified by the "MODATTR_"
definitions in the include/common.h source code file. By default, all
process attributes are retained.

Syslog Logging Option
---------------------

Format:  use_syslog=<0/1>
Example: use_syslog=1

This variable determines whether messages are logged to the syslog
facility on your local host. Values are as follows:

  * 0 = Don't use syslog facility
  * 1 = Use syslog facility

Notification Logging Option
---------------------------

Format:  log_notifications=<0/1>
Example: log_notifications=1

This variable determines whether or not notification messages are
logged. If you have a lot of contacts or regular service failures your
log file will grow relatively quickly. Use this option to keep contact
notifications from being logged.

  * 0 = Don't log notifications
  * 1 = Log notifications

Service Check Retry Logging Option
----------------------------------

Format:  log_service_retries=<0/1>
Example: log_service_retries=1

This variable determines whether or not service check retries are
logged. Service check retries occur when a service check results in a
non-OK state, but you have configured Centreon Engine to retry the
service more than once before responding to the error. Services in this
situation are considered to be in "soft" states. Logging service check
retries is mostly useful when attempting to debug Centreon Engine or
test out service :ref:`event handlers <advanced_event_handlers>`.

  * 0 = Don't log service check retries
  * 1 = Log service check retries

Host Check Retry Logging Option
-------------------------------

Format:  log_host_retries=<0/1>
Example: log_host_retries=1

This variable determines whether or not host check retries are
logged. Logging host check retries is mostly useful when attempting to
debug Centreon Engine or test out host
:ref:`event handlers <advanced_event_handlers>`.

  * 0 = Don't log host check retries
  * 1 = Log host check retries

Event Handler Logging Option
----------------------------

Format:  log_event_handlers=<0/1>
Example: log_event_handlers=1

This variable determines whether or not service and host
:ref:`event handlers <advanced_event_handlers>` are logged.

Event handlers are optional commands that can be run whenever a service
or hosts changes state. Logging event handlers is most useful when
debugging Centreon Engine or first trying out your event handler
scripts.

  * 0 = Don't log event handlers
  * 1 = Log event handlers

Initial States Logging Option
-----------------------------

Format:  log_initial_states=<0/1>
Example: log_initial_states=1

This variable determines whether or not Centreon Engine will force all
initial host and service states to be logged, even if they result in an
OK state. Initial service and host states are normally only logged when
there is a problem on the first check. Enabling this option is useful if
you are using an application that scans the log file to determine
long-term state statistics for services and hosts.

  * 0 = Don't log initial states (default)
  * 1 = Log initial states

External Command Logging Option
-------------------------------

Format:  log_external_commands=<0/1>
Example: log_external_commands=1

This variable determines whether or not Centreon Engine will log
:ref:`external commands <advanced_external_commands>` that it receives
from the :ref:`external command file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesexternalcommandfile>`.

.. note::

   This option does not control whether or not
   :ref:`passive service checks <passive_checks>`
   (which are a type of external command) get logged. To enable or
   disable logging of passive checks, use the
   :ref:`log_passive_checks <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablespassivecheckloggingoption>`
   option.

    * 0 = Don't log external commands
    * 1 = Log external commands (default)

Passive Check Logging Option
----------------------------

Format:  log_passive_checks=<0/1>
Example: log_passive_checks=1

This variable determines whether or not Centreon Engine will log
:ref:`passive host and service checks <passive_checks>` that it receives
from the :ref:`external command file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesexternalcommandfile>`.
If you are setting up a
:ref:`distributed monitoring environment <advanced_distributed_monitoring>`
or plan on handling a large number of passive checks on a regular basis,
you may wish to disable this option so your log file doesn't get too
large.

  * 0 = Don't log passive checks
  * 1 = Log passive checks (default)

Global Host Event Handler Option
--------------------------------

Format:  global_host_event_handler=<command>
Example: global_host_event_handler=log-host-event-to-db

This option allows you to specify a host event handler command that is
to be run for every host state change. The global event handler is
executed immediately prior to the event handler that you have optionally
specified in each host definition. The command argument is the short
name of a command that you define in your
:ref:`object configuration file <object_configuration_overview>`.
The maximum amount of time that this command can run is controlled by
the :ref:`event_handler_timeout <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableseventhandlertimeout>`
option. More information on event handlers can be found
:ref:`here <advanced_event_handlers>`.

Global Service Event Handler Option
-----------------------------------

Format:  global_service_event_handler=<command>
Example: global_service_event_handler=log-service-event-to-db

This option allows you to specify a service event handler command that
is to be run for every service state change. The global event handler is
executed immediately prior to the event handler that you have optionally
specified in each service definition. The command argument is the short
name of a command that you define in your
:ref:`object configuration file <object_configuration_overview>`.
The maximum amount of time that this command can run is controlled by
the :ref:`event_handler_timeout <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableseventhandlertimeout>`
option. More information on event handlers can be found
:ref:`here <advanced_event_handlers>`.

Inter-Check Sleep Time
----------------------

Format:  sleep_time=<seconds>
Example: sleep_time=1

This is the number of seconds that Centreon Engine will sleep before
checking to see if the next service or host check in the scheduling
queue should be executed.

.. note::

   That Centreon Engine will only sleep after it "catches up" with queued service checks that have fallen behind.

Service Inter-Check Delay Method
--------------------------------

Format:  service_inter_check_delay_method=<n/d/s/x.xx>
Example: service_inter_check_delay_method=s

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
:ref:`here <basics_service_and_host_check_scheduling>`. Values are as
follows:

  * n = Don't use any delay - schedule all service checks to run
    immediately (i.e. at the same time!)
  * d = Use a "dumb" delay of 1 second between service checks
  * s = Use a "smart" delay calculation to spread service checks out
    evenly (default)
  * x.xx = Use a user-supplied inter-check delay of x.xx seconds

Maximum Service Check Spread
----------------------------

Format:  max_service_check_spread=<minutes>
Example: max_service_check_spread=30

This option determines the maximum number of minutes from when Centreon
Engine starts that all services (that are scheduled to be regularly
checked) are checked. This option will automatically adjust the
:ref:`service <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesserviceintercheckdelaymethod>`
inter-check delay method" (if necessary) to ensure that the initial
checks of all services occur within the timeframe you specify. In
general, this option will not have an affect on service check scheduling
if scheduling information is being retained using the
:ref:`use_retained_scheduling_info <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedschedulinginfooption>`
option. Default value is 30 (minutes).

Service Interleave Factor
-------------------------

Format:  service_interleave_factor=<s|x>
Example: service_interleave_factor=s

This variable determines how service checks are
interleaved. Interleaving allows for a more even distribution of service
checks, reduced load on remote hosts, and faster overall detection of
host problems. Setting this value to 1 is equivalent to not interleaving
the service checks (this is how versions of Centreon Engine previous to
0.0.5 worked). Set this value to s (smart) for automatic calculation of
the interleave factor unless you have a specific reason to change
it. You should see that the service check results are spread out as they
begin to appear. More information on how interleaving works can be found
:ref:`here <basics_service_and_host_check_scheduling>`.

  * x = A number greater than or equal to 1 that specifies the
    interleave factor to use. An interleave factor of 1 is equivalent to
    not interleaving the service checks.
  * s = Use a "smart" interleave factor calculation (default)

Maximum Concurrent Service Checks
---------------------------------

Format:  max_concurrent_checks=<max_checks>
Example: max_concurrent_checks=20

This option allows you to specify the maximum number of service checks
that can be run in parallel at any given time. Specifying a value of 1
for this variable essentially prevents any service checks from being run
in parallel. Specifying a value of 0 (the default) does not place any
restrictions on the number of concurrent checks. You'll have to modify
this value based on the system resources you have available on the
machine that runs Centreon Engine, as it directly affects the maximum
load that will be imposed on the system (processor utilization, memory,
etc.). More information on how to estimate how many concurrent checks
you should allow can be found
:ref:`here <basics_service_and_host_check_scheduling>`.

Check Result Reaper Frequency
-----------------------------

Format:  check_result_reaper_frequency=<frequency_in_seconds>
Example: check_result_reaper_frequency=5

This option allows you to control the frequency in seconds of check
result "reaper" events. "Reaper" events process the results from host
and service checks that have finished executing. These events consitute
the core of the monitoring logic in Centreon Engine.

Maximum Check Result Reaper Time
--------------------------------

Format:  max_check_result_reaper_time=<seconds>
Example: max_check_result_reaper_time=30

This option allows you to control the maximum amount of time in seconds
that host and service check result "reaper" events are allowed to
run. "Reaper" events process the results from host and service checks
that have finished executing. If there are a lot of results to process,
reaper events may take a long time to finish, which might delay timely
execution of new host and service checks. This variable allows you to
limit the amount of time that an individual reaper event will run before
it hands control back over to Centreon Engine for other portions of the
monitoring logic.

Host Inter-Check Delay Method
-----------------------------

Format:  host_inter_check_delay_method=<n/d/s/x.xx>
Example: host_inter_check_delay_method=s

This option allows you to control how host checks that are scheduled to
be checked on a regular basis are initially "spread out" in the event
queue. Using a "smart" delay calculation (the default) will cause
Centreon Engine to calculate an average check interval and spread
initial checks of all hosts out over that interval, thereby helping to
eliminate CPU load spikes. Using no delay is generally not
recommended. Using no delay will cause all host checks to be scheduled
for execution at the same time. More information on how to estimate how
the inter-check delay affects host check scheduling can be found
:ref:`here <advanced_service_and_host_check_scheduling>`.Values are as
follows:

  * n = Don't use any delay - schedule all host checks to run
    immediately (i.e. at the same time!)
  * d = Use a "dumb" delay of 1 second between host checks
  * s = Use a "smart" delay calculation to spread host checks out evenly
    (default)
  * x.xx = Use a user-supplied inter-check delay of x.xx seconds

Maximum Host Check Spread
-------------------------

Format:  max_host_check_spread=<minutes>
Example: max_host_check_spread=30

This option determines the maximum number of minutes from when Centreon
Engine starts that all hosts (that are scheduled to be regularly
checked) are checked. This option will automatically adjust the
:ref:`host inter-check <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostintercheckdelaymethod>`
delay method" (if necessary) to ensure that the initial checks of all
hosts occur within the timeframe you specify. In general, this option
will not have an affect on host check scheduling if scheduling
information is being retained using the
:ref:`use_retained_scheduling_info <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedschedulinginfooption>`
option. Default value is 30 (minutes).

Timing Interval Length
----------------------

Format:  interval_length=<seconds>
Example: interval_length=60

This is the number of seconds per "unit interval" used for timing in the
scheduling queue, re-notifications, etc. "Units intervals" are used in
the object configuration file to determine how often to run a service
check, how often to re-notify a contact, etc.

.. note::

   The default value for this is set to 60, which means that a "unit
   value" of 1 in the object configuration file will mean 60 seconds (1
   minute). I have not really tested other values for this variable, so
   proceed at your own risk if you decide to do so!

Auto-Rescheduling Option
------------------------

Format:  auto_reschedule_checks=<0/1>
Example: auto_reschedule_checks=1

This option determines whether or not Centreon Engine will attempt to
automatically reschedule active host and service checks to "smooth" them
out over time. This can help to balance the load on the monitoring
server, as it will attempt to keep the time between consecutive checks
consistent, at the expense of executing checks on a more rigid schedule.

.. note::

   This is an experimental feature and may be removed in future
   versions. Enabling this option can degrade performance - rather than
   increase it - if used improperly!

Auto-Rescheduling Interval
--------------------------

Format:  auto_rescheduling_interval=<seconds>
Example: auto_rescheduling_interval=30

This option determines how often (in seconds) Centreon Engine will
attempt to automatically reschedule checks. This option only has an
effect if the :ref:`auto_reschedule_checks <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesautoreschedulingoption>`
option is enabled. Default is 30 seconds.

.. note::

   This is an experimental feature and may be removed in future
   versions. Enabling the auto-rescheduling option can degrade
   performance - rather than increase it - if used improperly!

Auto-Rescheduling Window
------------------------

Format:  auto_rescheduling_window=<seconds>
Example: auto_rescheduling_window=180

This option determines the "window" of time (in seconds) that Centreon
Engine will look at when automatically rescheduling checks. Only host
and service checks that occur in the next X seconds (determined by this
variable) will be rescheduled. This option only has an effect if the
:ref:`auto_reschedule_checks <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesautoreschedulingoption>`
option is enabled. Default is 180 seconds (3 minutes).

.. note::

   This is an experimental feature and may be removed in future
   versions. Enabling the auto-rescheduling option can degrade
   performance - rather than increase it - if used improperly!

Aggressive Host Checking Option
-------------------------------

Format:  use_aggressive_host_checking=<0/1>
Example: use_aggressive_host_checking=0

Centreon Engine tries to be smart about how and when it checks the
status of hosts. In general, disabling this option will allow Centreon
Engine to make some smarter decisions and check hosts a bit
faster. Enabling this option will increase the amount of time required
to check hosts, but may improve reliability a bit. Unless you have
problems with Centreon Engine not recognizing that a host recovered, I
would suggest not enabling this option.

  * 0 = Don't use aggressive host checking (default)
  * 1 = Use aggressive host checking

Translate Passive Host Checks Option
------------------------------------

Format:  translate_passive_host_checks=<0/1>
Example: translate_passive_host_checks=1

This option determines whether or not Centreon Engine will translate
DOWN/UNREACHABLE passive host check results to their "correct" state
from the viewpoint of the local Centreon Engine instance. This can be
very useful in distributed and failover monitoring installations. More
information on passive check state translation can be found
:ref:`here <basics_passive_host_state_translation>`.

  * 0 = Disable check translation (default)
  * 1 = Enable check translation

Passive Host Checks Are SOFT Option
-----------------------------------

Format:  passive_host_checks_are_soft=<0/1>
Example: passive_host_checks_are_soft=1

This option determines whether or not Centreon Engine will treat
:ref:`passive host checks <passive_checks>` as HARD states or SOFT
states. By default, a passive host check result will put a host into a
:ref:`HARD state type <state_types>`. You can change this behavior by
enabling this option.

  * 0 = Passive host checks are HARD (default)
  * 1 = Passive host checks are SOFT

Predictive Host Dependency Checks Option
----------------------------------------

Format:  enable_predictive_host_dependency_checks=<0/1>
Example: enable_predictive_host_dependency_checks=1

This option determines whether or not Centreon Engine will execute
predictive checks of hosts that are being depended upon (as defined in
:ref:`host <basics_object_definitions#object_definitionsobjecttypeshostdependencydefinition>`
dependencies") for a particular host when it changes state. Predictive
checks help ensure that the dependency logic is as accurate as
possible. More information on how predictive checks work can be found
:ref:`here <advanced_host_and_service_dependencies>`.

  * 0 = Disable predictive checks
  * 1 = Enable predictive checks (default)

Predictive Service Dependency Checks Option
-------------------------------------------

Format:  enable_predictive_service_dependency_checks=<0/1>
Example: enable_predictive_service_dependency_checks=1

This option determines whether or not Centreon Engine will execute
predictive checks of services that are being depended upon (as defined
in :ref:`service dependencies <basics_object_definitions#object_definitionsobjecttypesservicedependencydefinition>`)
for a particular service when it changes state. Predictive checks help
ensure that the dependency logic is as accurate as possible. More
information on how predictive checks work can be found
:ref:`here <advanced_host_and_service_dependencies>`.

  * 0 = Disable predictive checks
  * 1 = Enable predictive checks (default)

Cached Host Check Horizon
-------------------------

Format:  cached_host_check_horizon=<seconds>
Example: cached_host_check_horizon=15

This option determines the maximum amount of time (in seconds) that the
state of a previous host check is considered current. Cached host states
(from host checks that were performed more recently than the time
specified by this value) can improve host check performance
immensely. Too high of a value for this option may result in
(temporarily) inaccurate host states, while a low value may result in a
performance hit for host checks. Use a value of 0 if you want to disable
host check caching. More information on cached checks can be found
:ref:`here <advanced_cached_checks>`.

Cached Service Check Horizon
----------------------------

Format:  cached_service_check_horizon=<seconds>
Example: cached_service_check_horizon=15

This option determines the maximum amount of time (in seconds) that the
state of a previous service check is considered current. Cached service
states (from service checks that were performed more recently than the
time specified by this value) can improve service check performance when
a lot of :ref:`service dependencies <basics_object_definitions#object_definitionsobjecttypesservicedependencydefinition>`
are used. Too high of a value for this option may result in inaccuracies
in the service dependency logic. Use a value of 0 if you want to disable
service check caching. More information on cached checks can be found
:ref:`here <advanced_cached_checks>`.

Large Installation Tweaks Option
--------------------------------

Format:  use_large_installation_tweaks=<0/1>
Example: use_large_installation_tweaks=0

This option determines whether or not the Centreon Engine daemon will
take several shortcuts to improve performance. These shortcuts result in
the loss of a few features, but larger installations will likely see a
lot of benefit from doing so. More information on what optimizations are
taken when you enable this option can be found
:ref:`here <advanced_large_installation_tweaks>`.

  * 0 = Don't use tweaks (default)
  * 1 = Use tweaks

Child Process Memory Option
---------------------------

Format:  free_child_process_memory=<0/1>
Example: free_child_process_memory=0

This option determines whether or not Centreon Engine will free memory
in child processes when they are fork()ed off from the main process. By
default, Centreon Engine frees memory. However, if the
:ref:`use_large_installation_tweaks <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableslarge_installation_tweaksoption>`
option is enabled, it will not. By defining this option in your
configuration file, you are able to override things to get the behavior
you want.

  * 0 = Don't free memory
  * 1 = Free memory

Child Processes Fork Twice
--------------------------

Format:  child_processes_fork_twice=<0/1>

This is a deprecated and ignored variable.

Environment Macros Option
-------------------------

Format:  enable_environment_macros=<0/1>
Example: enable_environment_macros=0

This option determines whether or not the Centreon Engine daemon will
make all standard :ref:`macros <standard_macros>` available as
environment variables to your check, notification, event hander,
etc. commands. In large Centreon Engine installations this can be
problematic because it takes additional memory and (more importantly)
CPU to compute the values of all macros and make them available to the
environment.

  * 0 = Don't make macros available as environment variables
  * 1 = Make macros available as environment variables (default)

Flap Detection Option
---------------------

Format:  enable_flap_detection=<0/1>
Example: enable_flap_detection=0

This option determines whether or not Centreon Engine will try and
detect hosts and services that are "flapping". Flapping occurs when a
host or service changes between states too frequently, resulting in a
barrage of notifications being sent out. When Centreon Engine detects
that a host or service is flapping, it will temporarily suppress
notifications for that host/service until it stops flapping. Flap
detection is very experimental at this point, so use this feature with
caution! More information on how flap detection and handling works can
be found :ref:`here <advanced_detection_and_handling_of_state_flapping>`.

.. note::

   If you have :ref:`state retention <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionoption>`
   enabled, Centreon Engine will ignore this setting when it (re)starts
   and use the last known setting for this option (as stored in the
   :ref:`state retention file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionfile>`),
   unless you disable the
   :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   option. If you want to change this option when state retention is
   active (and the
   :ref:`use_retained_program_state <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesuseretainedprogramstateoption>`
   is enabled), you'll have to use the appropriate
   :ref:`external command <advanced_external_commands>` or change it via
   the web interface.

    * 0 = Don't enable flap detection (default)
    * 1 = Enable flap detection

Low Service Flap Threshold
--------------------------

Format:  low_service_flap_threshold=<percent>
Example: low_service_flap_threshold=25.0

This option is used to set the low threshold for detection of service
flapping. For more information on how flap detection and handling works
(and how this option affects things) read
:ref:`this <advanced_detection_and_handling_of_state_flapping>`.

High Service Flap Threshold
---------------------------

Format:  high_service_flap_threshold=<percent>
Example: high_service_flap_threshold=50.0

This option is used to set the high threshold for detection of service
flapping. For more information on how flap detection and handling works
(and how this option affects things) read
:ref:`this <advanced_detection_and_handling_of_state_flapping>`.

Low Host Flap Threshold
-----------------------

Format:  low_host_flap_threshold=<percent>
Example: low_host_flap_threshold=25.0

This option is used to set the low threshold for detection of host
flapping. For more information on how flap detection and handling works
(and how this option affects things) read
:ref:`this <advanced_detection_and_handling_of_state_flapping>`.

High Host Flap Threshold
------------------------

Format:  high_host_flap_threshold=<percent>
Example: high_host_flap_threshold=50.0

This option is used to set the high threshold for detection of host
flapping. For more information on how flap detection and handling works
(and how this option affects things) read
:ref:`this <advanced_detection_and_handling_of_state_flapping>`.

Soft State Dependencies Option
------------------------------

Format:  soft_state_dependencies=<0/1>
Example: soft_state_dependencies=0

This option determines whether or not Centreon Engine will use soft
state information when checking
:ref:`host and service dependencies <advanced_host_and_service_dependencies>`.
Normally Centreon Engine will only use the latest hard host or service
state when checking dependencies. If you want it to use the latest state
(regardless of whether its a soft or hard
:ref:`state type <state_types>`), enable this option.

  * 0 = Don't use soft state dependencies (default)
  * 1 = Use soft state dependencies

Service Check Timeout
---------------------

Format:  service_check_timeout=<seconds>
Example: service_check_timeout=60

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

Host Check Timeout
------------------

Format:  host_check_timeout=<seconds>
Example: host_check_timeout=60

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

Event Handler Timeout
---------------------

Format:  event_handler_timeout=<seconds>
Example: event_handler_timeout=60

This is the maximum number of seconds that Centreon Engine will allow
:ref:`event handlers <advanced_event_handlers>` to be run. If an event
handler exceeds this time limit it will be killed and a warning will be
logged.

There is often widespread confusion as to what this option really
does. It is meant to be used as a last ditch mechanism to kill off
commands which are misbehaving and not exiting in a timely manner. It
should be set to something high (like 60 seconds or more), so that each
event handler command normally finishes executing within this time
limit. If an event handler runs longer than this limit, Centreon Engine
will kill it off thinking it is a runaway processes.

Notification Timeout
--------------------

Format:  notification_timeout=<seconds>
Example: notification_timeout=60

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

Obsessive Compulsive Service Processor Timeout
----------------------------------------------

Format:  ocsp_timeout=<seconds>
Example: ocsp_timeout=5

This is the maximum number of seconds that Centreon Engine will allow an
:ref:`obsessive compulsive service processor <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsiveserviceprocessorcommand>`
command" to be run. If a command exceeds this time limit it will be
killed and a warning will be logged.

Obsessive Compulsive Host Processor Timeout
-------------------------------------------

Format:  ochp_timeout=<seconds>
Example: ochp_timeout=5

This is the maximum number of seconds that Centreon Engine will allow an
:ref:`obsessive compulsive host processor <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsivehostprocessorcommand>`
command" to be run. If a command exceeds this time limit it will be
killed and a warning will be logged.

Performance Data Processor Command Timeout
------------------------------------------

Format:  perfdata_timeout=<seconds>
Example: perfdata_timeout=5

This is the maximum number of seconds that Centreon Engine will allow a
:ref:`host performance data <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostperformance_dataprocessingcommand>`
processor command" or
:ref:`service performance data processor command <main_configuration_file_optionsconfigurationfilevariablesserviceperformance_dataprocessingcommand>`
to be run. If a command exceeds this time limit it will be killed and a
warning will be logged.

Obsess Over Services Option
---------------------------

Format:  obsess_over_services=<0/1>
Example: obsess_over_services=1

This value determines whether or not Centreon Engine will "obsess" over
service checks results and run the
:ref:`obsessive compulsive service processor command <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsiveserviceprocessorcommand>`
you define. I know - funny name, but it was all I could think of. This
option is useful for performing
:ref:`distributed monitoring <advanced_distributed_monitoring>`.
If you're not doing distributed monitoring, don't enable this option.

  * 0 = Don't obsess over services (default)
  * 1 = Obsess over services

Obsessive Compulsive Service Processor Command
----------------------------------------------

Format:  ocsp_command=<command>
Example: ocsp_command=obsessive_service_handler

This option allows you to specify a command to be run after every
service check, which can be useful in
:ref:`distributed monitoring <advanced_distributed_monitoring>`. This
command is executed after any :ref:`event handler <advanced_event_handlers>`
or :ref:`notification <notifications>` commands. The command argument is
the short name of a :ref:`command definition <basics_object_definitions#object_definitionsobjecttypescommanddefinition>`
that you define in your object configuration file. The maximum amount of
time that this command can run is controlled by the
:ref:`ocsp_timeout <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsiveserviceprocessortimeout>`
option. More information on distributed monitoring can be found
:ref:`here <advanced_distributed_monitoring>`. This command is only
executed if the :ref:`obsess_over_services <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessoverservicesoption>`
option is enabled globally and if the obsess_over_service directive in
the :ref:`service definition <basics_object_definitions#object_definitionsobjecttypesservicedefinitionservice>`
is enabled.

Obsess Over Hosts Option
------------------------

Format:  obsess_over_hosts=<0/1>
Example: obsess_over_hosts=1

This value determines whether or not Centreon Engine will "obsess" over
host checks results and run the
:ref:`obsessive compulsive host processor command <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsivehostprocessorcommand>`
you define. I know - funny name, but it was all I could think of. This
option is useful for performing
:ref:`distributed monitoring <advanced_distributed_monitoring>`. If
you're not doing distributed monitoring, don't enable this option.

  * 0 = Don't obsess over hosts (default)
  * 1 = Obsess over hosts

Obsessive Compulsive Host Processor Command
-------------------------------------------

Format:  ochp_command=<command>
Example: ochp_command=obsessive_host_handler

This option allows you to specify a command to be run after every host
check, which can be useful in :ref:`distributed monitoring <advanced_distributed_monitoring>`.
This command is executed after any :ref:`event handler <advanced_event_handlers>`
or :ref:`notification <notifications>` commands. The command argument is
the short name of a :ref:`command definition <basics_object_definitions#object_definitionsobjecttypescommanddefinition>`
that you define in your object configuration file. The maximum amount of
time that this command can run is controlled by the
:ref:`ochp_timeout <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsivehostprocessortimeout>`
option. More information on distributed monitoring can be found
:ref:`here <advanced_distributed_monitoring>`. This command is only
executed if the :ref:`obsess_over_hosts <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessoverhostsoption>`
option is enabled globally and if the obsess_over_host directive in the
:ref:`host definition <basics_object_definitions#object_definitionsobjecttypeshostdefinition>`
is enabled.

Performance Data Processing Option
----------------------------------

Format:  process_performance_data=<0/1>
Example: process_performance_data=1

This value determines whether or not Centreon Engine will process host
and service check :ref:`performance data <advanced_performance_data>`.

  * 0 = Don't process performance data (default)
  * 1 = Process performance data

Host Performance Data Processing Command
----------------------------------------

Format:  host_perfdata_command=<command>
Example: host_perfdata_command=process-host-perfdata

This option allows you to specify a command to be run after every host
check to process host :ref:`performance data <advanced_performance_data>`
that may be returned from the check. The command argument is the short
name of a :ref:`command <basics_object_definitions#object_definitionsobjecttypescommanddefinition>`
definition" that you define in your object configuration file. This
command is only executed if the
:ref:`process_performance_data <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesperformance_dataprocessingoption>`
option is enabled globally and if the process_perf_data directive in the
:ref:`host definition <basics_object_definitions#object_definitionsobjecttypeshostdefinition>`
is enabled.

Service Performance Data Processing Command
-------------------------------------------

Format:  service_perfdata_command=<command>
Example: service_perfdata_command=process-service-perfdata

This option allows you to specify a command to be run after every
service check to process service :ref:`performance data <advanced_performance_data>`
that may be returned from the check. The command argument is the short
name of a :ref:`command definition <basics_object_definitions#object_definitionsobjecttypescommanddefinition>`
that you define in your object configuration file. This command is only
executed if the :ref:`process_performance_data <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesperformance_dataprocessingoption>`
option is enabled globally and if the process_perf_data directive in the
:ref:`service definition <basics_object_definitions#object_definitionsobjecttypesservicedefinitionservice>`
is enabled.

Host Performance Data File
--------------------------

Format:  host_perfdata_file=<file_name>
Example: host_perfdata_file=/var/log/centreon-engine/host-perfdata.dat

This option allows you to specify a file to which host
:ref:`performance data <advanced_performance_data>` will be written
after every host check. Data will be written to the performance file as
specified by the :ref:`host_perfdata_file_template <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostperformance_datafiletemplate>`
option. Performance data is only written to this file if the
:ref:`process_performance_data <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesperformance_dataprocessingoption>`
option is enabled globally and if the process_perf_data directive in the
:ref:`host definition <basics_object_definitions#object_definitionsobjecttypeshostdefinition>`
is enabled.

Service Performance Data File
-----------------------------

Format:  service_perfdata_file=<file_name>
Example: service_perfdata_file=/var/log/centreon-engine/service-perfdata.dat

This option allows you to specify a file to which service
:ref:`performance data <advanced_performance_data>` will be written
after every service check. Data will be written to the performance file
as specified by the :ref:`service_perfdata_file_template <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesserviceperformance_datafiletemplate>`
option. Performance data is only written to this file if the
:ref:`process_performance_data <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesperformance_dataprocessingoption>`
option is enabled globally and if the process_perf_data directive in the
:ref:`service definition <basics_object_definitions#object_definitionsobjecttypesservicedefinitionservice>`
is enabled.

Host Performance Data File Template
-----------------------------------

Format:  host_perfdata_file_template=<template>
Example: host_perfdata_file_template=[HOSTPERFDATA]\\t$TIMET$\\t$HOSTNAME$\\t$HOSTEXECUTIONTIME$\\t$HOSTOUTPUT$\\t$HOSTPERFDATA$

This option determines what (and how) data is written to the
:ref:`host performance data file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostperformance_datafile>`.
The template may contain :ref:`macros <understanding_macros_and_how_they_work>`,
special characters (\t for tab, \r for carriage return, \n for newline)
and plain text. A newline is automatically added after each write to the
performance data file.

Service Performance Data File Template
--------------------------------------

Format:  service_perfdata_file_template=<template>
Example: service_perfdata_file_template=[SERVICEPERFDATA]\\t$TIMET$\\t$HOSTNAME$\\t$SERVICEDESC$\\t$SERVICEEXECUTIONTIME$\\t$SERVICELATENCY$\\t$SERVICEOUTPUT$\\t$SERVICEPERFDATA$

This option determines what (and how) data is written to the
:ref:`service performance data file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesserviceperformance_datafile>`.
The template may contain :ref:`macros <understanding_macros_and_how_they_work>`,
special characters (\t for tab, \r for carriage return, \n for newline)
and plain text. A newline is automatically added after each write to the
performance data file.

Host Performance Data File Mode
-------------------------------

Format:  host_perfdata_file_mode=<mode>
Example: host_perfdata_file_mode=a

This option determines how the :ref:`host <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostperformance_datafile>`
performance data file" is opened. Unless the file is a named pipe you'll
probably want to use the default mode of append.

  * a = Open file in append mode (default)
  * w = Open file in write mode
  * p = Open in non-blocking read/write mode (useful when writing to
    pipes)

Service Performance Data File Mode
----------------------------------

Format:  service_perfdata_file_mode=<mode>
Example: service_perfdata_file_mode=a

This option determines how the :ref:`service <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesserviceperformance_datafile>`
performance data file" is opened. Unless the file is a named pipe you'll
probably want to use the default mode of append.

  * a = Open file in append mode (default)
  * w = Open file in write mode
  * p = Open in non-blocking read/write mode (useful when writing to
    pipes)

Host Performance Data File Processing Interval
----------------------------------------------

Format:  host_perfdata_file_processing_interval=<seconds>
Example: host_perfdata_file_processing_interval=0

This option allows you to specify the interval (in seconds) at which the
:ref:`host performance data file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostperformance_datafile>`
is processed using the :ref:`host performance data file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostperformance_datafileprocessingcommand>`
processing command". A value of 0 indicates that the performance data
file should not be processed at regular intervals.

Service Performance Data File Processing Interval
-------------------------------------------------

Format:  service_perfdata_file_processing_interval=<seconds>
Example: service_perfdata_file_processing_interval=0

This option allows you to specify the interval (in seconds) at which the
:ref:`service performance data <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesserviceperformance_datafile>`
file" is processed using the
:ref:`service performance data file processing command <main_configuration_file_optionsconfigurationfilevariablesserviceperformance_datafileprocessingcommand>`.
A value of 0 indicates that the performance data file should not be
processed at regular intervals.

Host Performance Data File Processing Command
---------------------------------------------

Format:  host_perfdata_file_processing_command=<command>
Example: host_perfdata_file_processing_command=process-host-perfdata-file

This option allows you to specify the command that should be executed to
process the :ref:`host performance <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostperformance_datafile>`
data file". The command argument is the short name of a
:ref:`command definition <object_definitionsobjecttypescommanddefinition>`
that you define in your object configuration file. The interval at
which this command is executed is determined by the
:ref:`host_perfdata_file_processing_interval <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostperformance_datafileprocessinginterval>`
directive.

Service Performance Data File Processing Command
------------------------------------------------

Format:  service_perfdata_file_processing_command=<command>
Example: service_perfdata_file_processing_command=process-service-perfdata-file

This option allows you to specify the command that should be executed to
process the :ref:`service <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesserviceperformance_datafile>`
performance data file". The command argument is the short name of a
:ref:`command definition <object_definitionsobjecttypescommanddefinition>`
that you define in your object configuration file. The interval at which
this command is executed is determined by the
:ref:`service_perfdata_file_processing_interval <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesserviceperformance_datafileprocessinginterval>`
directive.

Orphaned Service Check Option
-----------------------------

Format:  check_for_orphaned_services=<0/1>
Example: check_for_orphaned_services=1

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

Orphaned Host Check Option
--------------------------

Format:  check_for_orphaned_hosts=<0/1>
Example: check_for_orphaned_hosts=1

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

Service Freshness Checking Option
---------------------------------

Format:  check_service_freshness=<0/1>
Example: check_service_freshness=0

This option determines whether or not Centreon Engine will periodically
check the "freshness" of service checks. Enabling this option is useful
for helping to ensure that :ref:`passive service checks <passive_checks>`
are received in a timely manner. More information on freshness checking
can be found :ref:`here <advanced_service_and_host_freshness_checks>`.

  * 0 = Don't check service freshness
  * 1 = Check service freshness (default)

Service Freshness Check Interval
--------------------------------

Format:  service_freshness_check_interval=<seconds>
Example: service_freshness_check_interval=60

This setting determines how often (in seconds) Centreon Engine will
periodically check the "freshness" of service check results. If you have
disabled service freshness checking (with the
:ref:`check_service_freshness <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesservicefreshnesscheckingoption>`
option), this option has no effect. More information on freshness
checking can be found :ref:`here <advanced_service_and_host_freshness_checks>`.

Host Freshness Checking Option
------------------------------

Format:  check_host_freshness=<0/1>
Example: check_host_freshness=0

This option determines whether or not Centreon Engine will periodically
check the "freshness" of host checks. Enabling this option is useful for
helping to ensure that :ref:`passive host checks <passive_checks>` are
received in a timely manner. More information on freshness checking can
be found :ref:`here <advanced_service_and_host_freshness_checks>`.

  * 0 = Don't check host freshness
  * 1 = Check host freshness (default)

Host Freshness Check Interval
-----------------------------

Format:  host_freshness_check_interval=<seconds>
Example: host_freshness_check_interval=60

This setting determines how often (in seconds) Centreon Engine will
periodically check the "freshness" of host check results. If you have
disabled host freshness checking (with the
:ref:`check_host_freshness <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableshostfreshnesscheckingoption>`
option), this option has no effect. More information on freshness
checking can be found
:ref:`here <advanced_service_and_host_freshness_checks>`.

Additional Freshness Threshold Latency Option
---------------------------------------------

Format:  additional_freshness_latency=<#>
Example: additional_freshness_latency=15

This option determines the number of seconds Centreon Engine will add to
any host or services freshness threshold it automatically calculates
(e.g. those not specified explicity by the user). More information on
freshness checking can be found
:ref:`here <advanced_service_and_host_freshness_checks>`.

Date Format
-----------

Format:  date_format=<option>
Example: date_format=us

This option allows you to specify what kind of date/time format Centreon
Engine should use in the web interface and date/time
:ref:`macros <understanding_macros_and_how_they_work>`. Possible options
(along with example output) include:

============== =================== ===================
Option         Output Format       Sample Output
============== =================== ===================
us             MM/DD/YYYY HH:MM:SS 06/30/2002 03:15:00
euro           DD/MM/YYYY HH:MM:SS 30/06/2002 03:15:00
iso8601        YYYY-MM-DD HH:MM:SS 2002-06-30 03:15:00
strict-iso8601 YYYY-MM-DDTHH:MM:SS 2002-06-30T03:15:00
============== =================== ===================

Timezone Option
---------------

Format:  use_timezone=<tz>
Example: use_timezone=US/Mountain

This option allows you to override the default timezone that this
instance of Centreon Engine runs in. Useful if you have multiple
instances of Centreon Engine that need to run from the same server, but
have different local times associated with them. If not specified,
Centreon Engine will use the system configured timezone.

Illegal Object Name Characters
------------------------------

Format:  illegal_object_name_chars=<chars...>
Example: illegal_object_name_chars=`~!$%^&*"|'<>?,()=

This option allows you to specify illegal characters that cannot be used
in host names, service descriptions, or names of other object
types. Centreon Engine will allow you to use most characters in object
definitions, but I recommend not using the characters shown in the
example above. Doing may give you problems in the web interface,
notification commands, etc.

Illegal Macro Output Characters
-------------------------------

Format:  illegal_macro_output_chars=<chars...>
Example: illegal_macro_output_chars=`~$^&"|'<>

This option allows you to specify illegal characters that should be
stripped from :ref:`macros <understanding_macros_and_how_they_work>`
before being used in notifications, event handlers, and other
commands. This DOES NOT affect macros used in service or host check
commands. You can choose to not strip out the characters shown in the
example above, but I recommend you do not do this. Some of these
characters are interpreted by the shell (i.e. the backtick) and can lead
to security problems. The following macros are stripped of the
characters you specify::

  $HOSTOUTPUT$, $HOSTPERFDATA$, $HOSTACKAUTHOR$, $HOSTACKCOMMENT$, $SERVICEOUTPUT$, $SERVICEPERFDATA$, $SERVICEACKAUTHOR$, and $SERVICEACKCOMMENT$

Regular Expression Matching Option
----------------------------------

Format:  use_regexp_matching=<0/1>
Example: use_regexp_matching=0

This option determines whether or not various directives in your
:ref:`object definitions <object_configuration_overview>` will be
processed as regular expressions. More information on how this works can
be found :ref:`here <advanced_time_saving_tricks_for_object_definitions>`.

  * 0 = Don't use regular expression matching (default)
  * 1 = Use regular expression matching

True Regular Expression Matching Option
---------------------------------------

Format:  use_true_regexp_matching=<0/1>
Example: use_true_regexp_matching=0

If you've enabled regular expression matching of various object
directives using the :ref:`use_regexp_matching <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesregularexpressionmatchingoption>`
option, this option will determine when object directives are treated as
regular expressions. If this option is disabled (the default),
directives will only be treated as regular expressions if they contain
*, ?, +, or \.. If this option is enabled, all appropriate directives
will be treated as regular expression - be careful when enabling this!
More information on how this works can be found
:ref:`here <advanced_time_saving_tricks_for_object_definitions>`.

  * 0 = Don't use true regular expression matching (default)
  * 1 = Use true regular expression matching

Administrator Email Address
---------------------------

Format:  admin_email=<email_address>
Example: admin_email=root@localhost.localdomain

This is the email address for the administrator of the local machine
(i.e. the one that Centreon Engine is running on).

This value can be used in notification commands by using the
$ADMINEMAIL$ :ref:`macro <understanding_macros_and_how_they_work>`.

Administrator Pager
-------------------

Format:  admin_pager=<pager_number_or_pager_email_gateway>
Example: admin_pager=pageroot@localhost.localdomain

This is the pager number (or pager email gateway) for the administrator
of the local machine (i.e. the one that Centreon Engine is running
on). The pager number/address can be used in notification commands by
using the $ADMINPAGER$ :ref:`macro <understanding_macros_and_how_they_work>`.

Event Broker Options
--------------------

Format:  event_broker_options=<#>
Example: event_broker_options=-1

This option controls what (if any) data gets sent to the event broker
and, in turn, to any loaded event broker modules. This is an advanced
option. When in doubt, either broker nothing (if not using event broker
modules) or broker everything (if using event broker modules). Possible
values are shown below.

  * 0 = Broker nothing
  * -1 = Broker everything
  * # = See BROKER_* definitions in source code (include/broker.h) for
      other values that can be OR'ed together

Event Broker Modules
--------------------

Format:  broker_module=<modulepath> [moduleargs]
Example: broker_module=/usr/local/centengine/bin/ndomod.o
         cfg_file=/etc/centreon-engine/ndomod.cfg

This directive is used to specify an event broker module that should by
loaded by Centreon Engine at startup. Use multiple directives if you
want to load more than one module. Arguments that should be passed to
the module at startup are seperated from the module path by a space.

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

Configuration File Variables
----------------------------

Format:  debug_file=<file_name>
Example: debug_file=/var/log/centreon-engine/centengine.debug

This option determines where Centreon Engine should write debugging
information. What (if any) information is written is determined by the
:ref:`debug_level <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariables>`
and :ref:`debug_verbosity <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariables>`
options. You can have Centreon Engine automaticaly rotate the debug file
when it reaches a certain size by using the
:ref:`max_debug_file_size <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariables>`
option.

Configuration File Variables
----------------------------

Format:  debug_level=<#>
Example: debug_level=24

This option determines what type of information Centreon Engine should
write to the :ref:`debug_file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariables>`.
This value is a logical OR of the values below.

  * -1 = Log everything
  * 0 = Log nothing (default)
  * 1 = Function enter/exit information
  * 2 = Config information
  * 4 = Process information
  * 8 = Scheduled event information
  * 16 = Host/service check information
  * 32 = Notification information
  * 64 = Event broker information

Configuration File Variables
----------------------------

Format:  debug_verbosity=<#>
Example: debug_verbosity=1

This option determines how much debugging information Centreon Engine
should write to the :ref:`debug_file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariables>`.

  * 0 = Basic information
  * 1 = More detailed information (default)
  * 2 = Highly detailed information

Configuration File Variables
----------------------------

Format:  max_debug_file_size=<#>
Example: max_debug_file_size=1000000

This option determines the maximum size (in bytes) of the
:ref:`debug file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariables>`.
If the file grows larger than this size, it will be renamed with a .old
extension. If a file already exists with a .old extension it will
automatically be deleted. This helps ensure your disk space usage
doesn't get out of control when debugging Centreon Engine.
