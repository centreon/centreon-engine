.. _obj_def:

Object Definitions
******************

Introduction
============

One of the features of Centreon Engine' object configuration format is
that you can create object definitions that inherit properties from
other object definitions. An explanation of how object inheritence works
can be found :ref:`here <object_inheritance>`. I strongly suggest that
you familiarize yourself with object inheritence once you read over the
documentation presented below, as it will make the job of creating and
maintaining object definitions much easier than it otherwise would
be. Also, read up on the :ref:`object tricks <obj_def_tricks>` that
offer shortcuts for otherwise tedious configuration tasks.

.. note::
   When creating and/or editing configuration files, keep the following in mind:

     * Lines that start with a '#' character are taken to be comments
       and are not processed
     * Directive names are case-sensitive
     * Characters that appear after a semicolon (;) in configuration
       lines are treated as comments and are not processed

.. _obj_def_retentionnotes:

Retention Notes
===============

It is important to point out that several directives in host and
service definitions may not be picked up by Centreon Engine when you
change them in your configuration files. Object directives that can
exhibit this behavior are marked with an asterisk
(:ref:`* <obj_def_retentionnotes>`).
The reason for this behavior is due to the fact that Centreon Engine
chooses to honor values stored in the
:ref:`state retention file <main_cfg_opt_state_retention_file>` over
values found in the config files, assuming you have
:ref:`state retention <main_cfg_opt_state_retention>` enabled on a
program-wide basis and the value of the directive is changed during
runtime with an
:ref:`external command <external_commands>`.

Sample Configuration Files
==========================

.. note::
   Sample object configuration files are installed in the
   ``/etc/centreon-engine/`` directory when you follow the
   :ref:`quickstart installation guide <exploit_quickstart>`.

Object Types
============

  * :ref:`Host definitions <obj_def_host>`
  * :ref:`Service definitions <obj_def_service>`
  * :ref:`Time period definitions <obj_def_timeperiod>`
  * :ref:`Command definitions <obj_def_command>`
  * :ref:`Connector definitions <obj_def_connector>`
  * :ref:`Service dependency definitions <obj_def_service_dependency>`
  * :ref:`Host dependency definitions <obj_def_host_dependency>`

.. _obj_def_host:

Host Definition
---------------

Description
^^^^^^^^^^^

A host definition is used to define a physical server, workstation,
device, etc. that resides on your network.

Definition Format
^^^^^^^^^^^^^^^^^

.. note::
   Optional directives are comment (line start with #).

::

  define host{
    host_name                      host_name
    alias                          alias
    address                        address
    # parents                      host_names
    # check_command                command_name
    # initial_state                [o,d,u]
    max_check_attempts             #
    # check_interval               #
    # retry_interval               #
    # active_checks_enabled        [0/1]
    check_period                   timeperiod_name
    # obsess_over_host             [0/1]
    # check_freshness              [0/1]
    # freshness_threshold          #
    # event_handler                command_name
    # event_handler_enabled        [0/1]
    # low_flap_threshold           #
    # high_flap_threshold          #
    # flap_detection_enabled       [0/1]
    # flap_detection_options       [o,d,u]
  }

Example Definition
^^^^^^^^^^^^^^^^^^

::

  define host{
    host_name                    bogus-router
    alias                        Bogus Router #1
    address                      192.168.1.254
    parents                      server-backbone
    check_command                check-host-alive
    check_interval               5
    retry_interval               1
    max_check_attempts           5
    check_period                 24x7
  }

Directive Descriptions
^^^^^^^^^^^^^^^^^^^^^^

============================ =========================================================================================================================
host_name                    This directive is used to define a short name used to identify the host. It is used in service (and other objects)
                             definitions to reference this particular host. Hosts can have multiple services (which are monitored) associated with
                             them. When used properly, the $HOSTNAME$ :ref:`macro <understanding_macros>` will contain this short name.
alias                        This directive is used to define a longer name or description used to identify the host. It is provided in order to allow
                             you to more easily identify a particular host. When used properly, the $HOSTALIAS$
                             :ref:`macro <understanding_macros>` will contain this alias/description.
address                      This directive is used to define the address of the host. Normally, this is an IP address, although it could really be
                             anything you want (so long as it can be used to check the status of the host). You can use a FQDN to identify the host
                             instead of an IP address, but if DNS services are not available this could cause problems. When used properly, the
                             $HOSTADDRESS$ :ref:`macro <understanding_macros>` will contain this address.
                             .. note::

                                If you do not specify an address directive in a host definition, the name of the host will be used as its address. A
                                word of caution about doing this, however * if DNS fails, most of your service checks will fail because the plugins
                                will be unable to resolve the host name.
parents                      This directive is used to define a comma-delimited list of short names of the "parent" hosts for this particular host.
                             Parent hosts are typically routers, switches, firewalls, etc. that lie between the monitoring host and a remote hosts. A
                             router, switch, etc. which is closest to the remote host is considered to be that host's "parent". Read the "Determining
                             Status and Reachability of Network Hosts" document located
                             :ref:`here <status_and_reachability_network>` for more information. If this host is on the
                             same network segment as the host doing the monitoring (without any intermediate routers, etc.) the host is considered to
                             be on the local network and will not have a parent host. Leave this value blank if the host does not have a parent host
                             (i.e. it is on the same segment as the Centreon Engine host). The order in which you specify parent hosts has no effect
                             on how things are monitored.
check_command                This directive is used to specify the short name of the :ref:`command <obj_def_command>`
                             that should be used to check if the host is up or down. Typically, this command would try and ping the host to see if it
                             is "alive". The command must return a status of OK (0) or Centreon Engine will assume the host is down. If you leave this
                             argument blank, the host will not be actively checked. Thus, Centreon Engine will likely always assume the host is up (it
                             may show up as being in a "PENDING" state in the web interface). This is useful if you are monitoring printers or other
                             devices that are frequently turned off. The maximum amount of time that the check command can run is controlled by
                             either the host's check_timeout option or the global :ref:`host_check_timeout <main_cfg_opt_host_check_timeout>`
                             option.
check_timeout                This is the maximum number of seconds that Centreon Engine will allow host checks to run. If checks exceed this limit,
                             they are killed and a DOWN state is returned. A timeout error will also be logged. There is often widespread confusion as
                             to what this option really does. It is meant to be used as a last ditch mechanism to kill off plugins which are
                             misbehaving and not exiting in a timely manner. It should be set to something reasonable (like 10 seconds), so that each
                             host check normally finishes executing within this time limit. If a host check runs longer than this limit, Centreon
                             Engine will kill it off thinking it is a runaway processes.
initial_state                By default Centreon Engine will assume that all hosts are in UP states when it starts. You can override the initial state
                             for a host by using this directive. Valid options are: o = UP, d = DOWN, and u = UNREACHABLE.
max_check_attempts           This directive is used to define the number of times that Centreon Engine will retry the host check command if it returns
                             any state other than an OK state. Setting this value to 1 will cause Centreon Engine to generate an alert without
                             retrying the host check.
                             .. note::

                                If you do not want to check the status of the host, you must still set this to a minimum value of 1. To bypass the
                                host check, just leave the check_command option blank.
check_interval               This directive is used to define the number of "time units" between regularly scheduled checks of the host. Unless you've
                             changed the :ref:`interval_length <main_cfg_opt_timing_interval_length>`
                             directive from the default value of 60, this number will mean minutes. More information on this value can be found in the
                             :ref:`check scheduling <scheduling_service_and_host>` documentation.
retry_interval               This directive is used to define the number of "time units" to wait before scheduling a re-check of the hosts. Hosts are
                             rescheduled at the retry interval when they have changed to a non-UP state. Once the host has been retried
                             max_check_attempts times without a change in its status, it will revert to being scheduled at its "normal" rate as
                             defined by the check_interval value. Unless you've changed the
                             :ref:`interval_length <main_cfg_opt_timing_interval_length>`
                             directive from the default value of 60, this number will mean minutes. More information on this value can be found in the
                             :ref:`check scheduling <scheduling_service_and_host>` documentation.
active_checks_enabled        :ref:`* <obj_def_retentionnotes>` This directive is used to determine whether or not active
                             checks (either regularly scheduled or on-demand) of this host are enabled. Values: 0 = disable active host checks,
                             1 = enable active host checks (default).
check_period                 This directive is used to specify the short name of the
                             :ref:`time period <obj_def_timeperiod>` during which active checks of this host can be made.
obsess_over_host             :ref:`* <obj_def_retentionnotes>` This directive determines whether or not checks for the
                             host will be "obsessed" over using the
                             :ref:`ochp_command <main_cfg_opt_obsessive_compulsive_host_processor_command>`.
check_freshness              :ref:`* <obj_def_retentionnotes>` This directive is used to determine whether or not
                             :ref:`freshness checks <freshness_checks>` are enabled for this host. Values: 0 = disable
                             freshness checks, 1 = enable freshness checks (default).
freshness_threshold          This directive is used to specify the freshness threshold (in seconds) for this host. If you set this directive to a
                             value of 0, Centreon Engine will determine a freshness threshold to use automatically.
event_handler                This directive is used to specify the short name of the :ref:`command <obj_def_command>`
                             that should be run whenever a change in the state of the host is detected (i.e. whenever it goes down or recovers). Read
                             the documentation on :ref:`event handlers <event_handlers>` for a more detailed explanation of how to write
                             scripts for handling events. The maximum amount of time that the event handler command can run is controlled by the
                             :ref:`event_handler_timeout <main_cfg_opt_event_handler_timeout>`
                             option.
event_handler_enabled        :ref:`* <obj_def_retentionnotes>` This directive is used to determine whether or not the
                             event handler for this host is enabled. Values: 0 = disable host event handler, 1 = enable host event handler.
low_flap_threshold           This directive is used to specify the low state change threshold used in flap detection for this host. More information
                             on flap detection can be found :ref:`here <flapping_detection>`. If you set this directive
                             to a value of 0, the program-wide value specified by the
                             :ref:`low_host_flap_threshold <main_cfg_opt_low_host_flap_threshold>`
                             directive will be used.
high_flap_threshold          This directive is used to specify the high state change threshold used in flap detection for this host. More information
                             on flap detection can be found :ref:`here <flapping_detection>`. If you set this directive
                             to a value of 0, the program-wide value specified by the
                             :ref:`high_host_flap_threshold <main_cfg_opt_high_host_flap_threshold>`
                             directive will be used.
flap_detection_enabled       :ref:`* <obj_def_retentionnotes>` This directive is used to determine whether or not flap
                             detection is enabled for this host. More information on flap detection can be found
                             :ref:`here <flapping_detection>`. Values: 0 = disable host flap detection, 1 = enable host
                             flap detection.
flap_detection_options       This directive is used to determine what host states the
                             :ref:`flap detection logic <flapping_detection>` will use for this host. Valid options are
                             a combination of one or more of the following: o = UP states, d = DOWN states, u = UNREACHABLE states.
timezone                     Time zone of this host. All times applied to this host (check period, ...) will be affected by this option.
============================ =========================================================================================================================

.. _obj_def_service:

Service Definition
------------------

Description
^^^^^^^^^^^

A service definition is used to identify a "service" that runs on a
host. The term "service" is used very loosely. It can mean an actual
service that runs on the host (POP, SMTP, HTTP, etc.) or some other type
of metric associated with the host (response to a ping, number of logged
in users, free disk space, etc.). The different arguments to a service
definition are outlined below.

Definition Format
^^^^^^^^^^^^^^^^^

.. note::
   Optional directives are comment (line start with #).

::

  define service{
    host_name                      host_name
    service_description            service_description
    # is_volatile                  [0/1]
    check_command                  command_name
    # initial_state                [o,w,u,c]
    max_check_attempts             #
    check_interval                 #
    retry_interval                 #
    # active_checks_enabled        [0/1]
    check_period                   timeperiod_name
    # obsess_over_service          [0/1]
    # check_freshness              [0/1]
    # freshness_threshold          #
    # event_handler                command_name
    # event_handler_enabled        [0/1]
    # low_flap_threshold           #
    # high_flap_threshold          #
    # flap_detection_enabled       [0/1]
    # flap_detection_options       [o,w,c,u]
  }

Example Definition
^^^^^^^^^^^^^^^^^^

::

  define service{
    host_name             linux-server
    service_description   check-disk-sda1
    check_command         check-disk!/dev/sda1
    max_check_attempts    5
    check_interval        5
    retry_interval        3
    check_period          24x7
  }



Directive Descriptions
^^^^^^^^^^^^^^^^^^^^^^

============================ =========================================================================================================================
host_name                    This directive is used to specify the short name(s) of the :ref:`host(s) <obj_def_host>` that the service "runs" on or is
                             associated with. Multiple hosts should be separated by commas.
service_description;         This directive is used to define the description of the service, which may contain spaces, dashes, and colons
                             (semicolons, apostrophes, and quotation marks should be avoided). No two services associated with the same host can have
                             the same description. Services are uniquely identified with their host_name and service_description directives.
is_volatile                  This directive is used to denote whether the service is "volatile". Services are normally not volatile. More information
                             on volatile service and how they differ from normal services can be found :ref:`here <volatile_services>`.
                             Value: 0 = service is not volatile, 1 = service is volatile.
check_command                This directive is used to specify the short name of the :ref:`command <obj_def_command>` that Centreon Engine will run in
                             order to check the status of the service. The maximum amount of time that the service check command can run is controlled
                             by either the service's check_timeout option or the global :ref:`service_check_timeout <main_cfg_opt_service_check_timeout>`
                             option.
check_timeout                This is the maximum number of seconds that Centreon Engine will allow service checks to run. If checks exceed this limit,
                             they are killed and a CRITICAL state is returned. A timeout error will also be logged. There is often widespread confusion
                             as to what this option really does. It is meant to be used as a last ditch mechanism to kill off plugins which are
                             misbehaving and not exiting in a timely manner. It should be set to something reasonably (like 10 seconds), so that each
                             service check normally finishes executing within this time limit. If a service check runs longer than this limit, Centreon
                             Engine will kill it off thinking it is a runaway processes.
initial_state                By default Centreon Engine will assume that all services are in OK states when it starts. You can override the initial
                             state for a service by using this directive. Valid options are: o = OK, w = WARNING, u = UNKNOWN, and c = CRITICAL.
max_check_attempts           This directive is used to define the number of times that Centreon Engine will retry the service check command if it
                             returns any state other than an OK state. Setting this value to 1 will cause Centreon Engine to generate an alert without
                             retrying the service check again.
check_interval               This directive is used to define the number of "time units" to wait before scheduling the next "regular" check of the
                             service. "Regular" checks are those that occur when the service is in an OK state or when the service is in a non-OK
                             state, but has already been rechecked max_check_attempts number of times. Unless you've changed the
                             :ref:`interval_length <main_cfg_opt_timing_interval_length>` directive from the default value of 60, this number will
                             mean minutes. More information on this value can be found in the :ref:`check scheduling <scheduling_service_and_host>`
                             documentation.
retry_interval               This directive is used to define the number of "time units" to wait before scheduling a re-check of the service. Services
                             are rescheduled at the retry interval when they have changed to a non-OK state. Once the service has been retried
                             max_check_attempts times without a change in its status, it will revert to being scheduled at its "normal" rate as
                             defined by the check_interval value. Unless you've changed the
                             :ref:`interval_length <main_cfg_opt_timing_interval_length>` directive from the default value of 60, this number will
                             mean minutes. More information on this value can be found in the :ref:`check scheduling <scheduling_service_and_host>`
                             documentation.
active_checks_enabled        :ref:`* <obj_def_retentionnotes>` This directive is used to determine whether or not active checks of this service are
                             enabled. Values: 0 = disable active service checks, 1 = enable active service checks (default).
check_period                 This directive is used to specify the short name of the :ref:`time period <obj_def_timeperiod>` during which active
                             checks of this service can be made.
obsess_over_service          :ref:`* <obj_def_retentionnotes>` This directive determines whether or not checks for the service will be "obsessed"
                             over using the :ref:`ocsp_command <main_cfg_opt_obsessive_compulsive_service_processor_command>`.
check_freshness              :ref:`* <obj_def_retentionnotes>` This directive is used to determine whether or not
                             :ref:`freshness checks <freshness_checks>` are enabled for this service. Values: 0 = disable freshness checks, 1 = enable
                             freshness checks (default).
freshness_threshold          This directive is used to specify the freshness threshold (in seconds) for this service. If you set this directive to a
                             value of 0, Centreon Engine will determine a freshness threshold to use automatically.
event_handler                This directive is used to specify the short name of the :ref:`command <obj_def_command>`
                             that should be run whenever a change in the state of the service is detected (i.e. whenever it goes down or recovers).
                             Read the documentation on:ref:`event handlers <event_handlers>` for a more detailed explanation of how to write
                             scripts for handling events. The maximum amount of time that the event handler command can run is controlled by the
                             :ref:`event_handler_timeout <main_cfg_opt_event_handler_timeout>`
                             option.
event_handler_enabled        This directive is used to determine whether or not the event handler for this service is enabled. Values: 0 = disable
                             service event handler, 1 = enable service event handler.
low_flap_threshold           This directive is used to specify the low state change threshold used in flap detection for this service. More
                             information on flap detection can be found :ref:`here <flapping_detection>`. If you set
                             this directive to a value of 0, the program-wide value specified by the
                             :ref:`low_service_flap_threshold <main_cfg_opt_low_service_flap_threshold>`
                             directive will be used.
high_flap_threshold          This directive is used to specify the high state change threshold used in flap detection for this service. More
                             information on flap detection can be found :ref:`here <flapping_detection>`. If you set
                             this directive to a value of 0, the program-wide value specified by the
                             :ref:`high_service_flap_threshold <main_cfg_opt_high_service_flap_threshold>`
                             directive will be used.
flap_detection_enabled       :ref:`* <obj_def_retentionnotes>` This directive is used to determine whether or not flap
                             detection is enabled for this service. More information on flap detection can be found
                             :ref:`here <flapping_detection>`. Values: 0 = disable service flap detection, 1 = enable
                             service flap detection.
flap_detection_options       This directive is used to determine what service states the
                             :ref:`flap detection logic <flapping_detection>` will use for this service. Valid options
                             are a combination of one or more of the following: o = OK states, w = WARNING states, c = CRITICAL states,
                             u = UNKNOWN states.
timezone                     Time zone of this service. All times applied to this service (check period, ...) will be affected by this option.
============================ =========================================================================================================================

.. _obj_def_timeperiod:

Time Period Definition
----------------------

Description
^^^^^^^^^^^

A time period is a list of times during various days that are considered
to be "valid" times for service checks. It consists of time ranges for
each day of the week that "rotate" once the week has come to an end.
Different types of exceptions to the normal weekly time are supported,
including: specific weekdays, days of generic months, days of specific
months, and calendar dates.

Definition Format
^^^^^^^^^^^^^^^^^

.. note::
   Optional directives are comment (line start with #).

::

  define timeperiod{
    timeperiod_name      timeperiod_name
    alias                alias
    # [weekday]          timeranges
    # [exception]        timeranges
    # exclude            [timeperiod1,timeperiod2,...,timeperiodn]
  }

Example Definitions
^^^^^^^^^^^^^^^^^^^

::

  define timeperiod{
    timeperiod_name nonworkhours
    alias           Non-Work Hours
    sunday          00:00-24:00             ; Every Sunday of every week
    monday          00:00-09:00,17:00-24:00 ; Every Monday of every week
    tuesday         00:00-09:00,17:00-24:00 ; Every Tuesday of every week
    wednesday       00:00-09:00,17:00-24:00 ; Every Wednesday of every week
    thursday        00:00-09:00,17:00-24:00 ; Every Thursday of every week
    friday          00:00-09:00,17:00-24:00 ; Every Friday of every week
    saturday        00:00-24:00             ; Every Saturday of every week
  }

  define timeperiod{
    timeperiod_name      misc-single-days
    alias                Misc Single Days
    1999-01-28           00:00-24:00 ; January 28th, 1999
    monday 3             00:00-24:00 ; 3rd Monday of every month
    day 2                00:00-24:00 ; 2nd day of every month
    february 10          00:00-24:00 ; February 10th of every year
    february -1          00:00-24:00 ; Last day in February of every year
    friday -2            00:00-24:00 ; 2nd to last Friday of every month
    thursday -1 november 00:00-24:00 ; Last Thursday in November of every year
  }

  define timeperiod{
    timeperiod_name                misc-date-ranges
    alias                          Misc Date Ranges
    2007-01-01 - 2008-02-01        00:00-24:00 ; January 1st, 2007 to February 1st, 2008
    monday 3 - thursday 4          00:00-24:00 ; 3rd Monday to 4th Thursday of every month
    day 1 - 15                     00:00-24:00 ; 1st to 15th day of every month
    day 20 - -1                    00:00-24:00 ; 20th to the last day of every month
    july 10 - 15                   00:00-24:00 ; July 10th to July 15th of every year
    april 10 - may 15              00:00-24:00 ; April 10th to May 15th of every year
    tuesday 1 april - friday 2 may 00:00-24:00 ; 1st Tuesday in April to 2nd Friday in May of every year
  }

  define timeperiod{
    timeperiod_name                    misc-skip-ranges
    alias                              Misc Skip Ranges
    2007-01-01 - 2008-02-01 / 3        00:00-24:00 ; Every 3 days from January 1st, 2007 to February 1st, 2008
    2008-04-01 / 7                     00:00-24:00 ; Every 7 days from April 1st, 2008 (continuing forever)
    monday 3 - thursday 4 / 2          00:00-24:00 ; Every other day from 3rd Monday to 4th Thursday of every month
    day 1 - 15 / 5                     00:00-24:00 ; Every 5 days from the 1st to the 15th day of every month
    july 10 - 15 / 2                   00:00-24:00 ; Every other day from July 10th to July 15th of every year
    tuesday 1 april - friday 2 may / 6 00:00-24:00 ; Every 6 days from the 1st Tuesday in April to the 2nd Friday in May of every year
  }

Directive Descriptions
^^^^^^^^^^^^^^^^^^^^^^

=============== ======================================================================================================================================
timeperiod_name This directives is the short name used to identify the time period.
alias           This directive is a longer name or description used to identify the time period.
[weekday]       The weekday directives ("sunday" through "saturday")are comma-delimited lists of time ranges that are "valid" times for a particular
                day of the week. Notice that there are seven different days for which you can define time ranges (Sunday through Saturday). Each time
                range is in the form of HH:MM-HH:MM, where hours are specified on a 24 hour clock. For example, 00:15-24:00 means 12:15am in the
                morning for this day until 12:00am midnight (a 23 hour, 45 minute total time range). If you wish to exclude an entire day from the
                timeperiod, simply do not include it in the timeperiod definition.
[exception]     You can specify several different types of exceptions to the standard rotating weekday schedule. Exceptions can take a number of
                different forms including single days of a specific or generic month, single weekdays in a month, or single calendar dates. You can
                also specify a range of days/dates and even specify skip intervals to obtain functionality described by "every 3 days between these
                dates". Rather than list all the possible formats for exception strings, I'll let you look at the example timeperiod definitions
                above to see what's possible. :-) Weekdays and different types of exceptions all have different levels of precedence, so its
                important to understand how they can affect each other. More information on this can be found in the documentation on
                :ref:`timeperiods <timeperiods>`.
exclude         This directive is used to specify the short names of other timeperiod definitions whose time ranges should be excluded from this
                timeperiod. Multiple timeperiod names should be separated with a comma.
=============== ======================================================================================================================================

.. _obj_def_command:

Command Definition
------------------

Description
^^^^^^^^^^^

A command definition is just that. It defines a command. Commands that
can be defined include service checks, service event handlers, host
checks, and host event handlers. Command definitions can contain
:ref:`macros <understanding_macros>`, but you must make sure that you
include only those macros that are "valid" for the circumstances when
the command will be used. More information on what macros are available
and when they are "valid" can be found
:ref:`here <understanding_macros>`. The different arguments to a command
definition are outlined below.

Definition Format
^^^^^^^^^^^^^^^^^

.. note::
   Optional directives are comment (line start with #).

::

  define command{
    command_name   command_name
    command_line   command_line
    # connector    connector_name
  }

Example Definition
^^^^^^^^^^^^^^^^^^

::

  define command{
    command_name check_pop
    command_line /usr/lib/nagios/plugins/check_pop -H $HOSTADDRESS$
  }

Directive Descriptions
^^^^^^^^^^^^^^^^^^^^^^

============ =========================================================================================================================================
command_name This directive is the short name used to identify the command. It is referenced in :ref:`host <obj_def_host>`, and
             :ref:`service <obj_def_service>` definitions (in check, and event handler directives), among other places.
command_line This directive is used to define what is actually executed by Centreon Engine when the command is used for service or host checks,
             or :ref:`event handlers <event_handlers>`. Before the command line is executed, all valid
             :ref:`macros <understanding_macros>` are replaced with their respective values. See the documentation on macros for
             determining when you can use different macros. Note that the command line is not surrounded in quotes. Also, if you want to pass a dollar
             sign ($)on the command line, you have to escape it with another dollar sign.
             .. note::

                You may not include a semicolon (;) in the command_line directive, because everything after it will be ignored as a config file
                comment. You can work around this limitation by setting one of the :ref:`$USER$ <user_configuration_macros_misc>` macros in your
                :ref:`resource file <main_cfg_opt_resource_file>` to a semicolon and then referencing the appropriate $USER$ macro in the
                command_line directive in place of the semicolon.If you want to pass arguments to commands during runtime, you can use
                :ref:`$ARGn$ macros <user_configuration_macros_misc>` in the command_line directive of the command definition and then separate
                individual arguments from the command name (and from each other) using bang (!) characters in the object definition directive
                (host check command, service event handler command, etc) that references the command. More information on how arguments in command
                definitions are processed during runtime can be found in the documentation on :ref:`macros <understanding_macros>`.

             .. note::

                Centreon-Engine does not support the shell commands in command_line. You need to define a command without shell features.
connector    his directive is used for link a command with a connector. When this directive is not empty, the command is replace by the connector.
             When the connector is call the command_line argument is use.
============ =========================================================================================================================================

.. _obj_def_connector:

Connector Definition
--------------------

Description
^^^^^^^^^^^

A connector is just like a command with better performences. A connector
run on background and it is never close. A connector is define by a name
and a command line.

Definition Format
^^^^^^^^^^^^^^^^^

.. note::
   Optional directives are comment (line start with #).

::

  define connector{
    connector_name connector_name
    connector_line connector_line
  }

Example Definition
^^^^^^^^^^^^^^^^^^

::

  define connector{
    connector_name connector_icmp
    connector_line /usr/lib/nagios/plugins/connector_icmp
  }

Directive Descriptions
^^^^^^^^^^^^^^^^^^^^^^

============== =======================================================================================================================================
connector_name This directive is the short name used to identify the connector. It is referenced in :ref:`command <obj_def_connector>` definitions.
connector_line This directive is used to define the path of the binary connector and the optional argument. It is possible to use the Centreon-Engine
               macros.
============== =======================================================================================================================================

.. _obj_def_service_dependency:

Service Dependency Definition
-----------------------------

Description
^^^^^^^^^^^

Service dependencies are an advanced feature of Centreon Engine that
allow you to suppress active checks of services based on the status of
one or more other services. Service dependencies are optional and are
mainly targeted at advanced users who have complicated monitoring
setups. More information on how service dependencies work (read this!)
can be found :ref:`here <host_and_service_dependencies>`.

Definition Format
^^^^^^^^^^^^^^^^^

.. note::
   Optional directives are comment (line start with #). However, you
   must supply at least one type of criteria for the definition to be of
   much use.

::

  define servicedependency{
    dependent_host_name                host_name
    dependent_service_description      service_description
    host_name                          host_name
    service_description                service_description
    # inherits_parent                  [0/1]
    # failure_criteria                 [o,w,u,c,p,n]
    # dependency_period                timeperiod_name
  }

Example Definition
^^^^^^^^^^^^^^^^^^

::

  define servicedependency{
    host_name                     WWW1
    service_description           Apache Web Server
    dependent_host_name           WWW1
    dependent_service_description Main Web Site
    failure_criteria              w,u,c
  }

Directive Descriptions
^^^^^^^^^^^^^^^^^^^^^^

============================= ========================================================================================================================
dependent_host_name           This directive is used to identify the short name(s) of the :ref:`host(s) <obj_def_host>`
                              that the dependent service "runs" on or is associated with. Multiple hosts should be separated by commas.
dependent_service_description This directive is used to identify the description of the dependent :ref:`service <obj_def_service>`.
host_name                     This directive is used to identify the short name(s) of the
                              :ref:`host(s) <obj_def_host>` that the service that is being depended upon (also referred
                              to as the master service) "runs" on or is associated with. Multiple hosts should be separated by commas.
service_description           This directive is used to identify the description of the :ref:`service <obj_def_service>`
                              that is being depended upon (also referred to as the master service).
inherits_parent               This directive indicates whether or not the dependency inherits dependencies of the service that is being depended upon
                              (also referred to as the master service). In other words, if the master service is dependent upon other services and any
                              one of those dependencies fail, this dependency will also fail.
failure_criteria              This directive is used to specify the criteria that determine when the dependent service should not be actively checked.
                              If the master service is in one of the failure states we specify, the dependent service will not be actively checked.
                              Valid options are a combination of one or more of the following (multiple options are separated with commas): o = fail
                              on an OK state, w = fail on a WARNING state, u = fail on an UNKNOWN state, c = fail on a CRITICAL state, and p = fail on
                              a pending state (e.g. the service has not yet been checked). If you specify n (none) as an option, the execution
                              dependency will never fail and checks of the dependent service will always be actively checked (if other conditions
                              allow for it to be). Example: If you specify o,c,u in this field, the dependent service will not be actively checked if
                              the master service is in either an OK, a CRITICAL, or an UNKNOWN state.
dependency_period             This directive is used to specify the short name of the
                              :ref:`time period <obj_def_timeperiod>` during which this dependency is valid. If this
                              directive is not specified, the dependency is considered to be valid during all times.
============================= ========================================================================================================================

.. _obj_def_host_dependency:

Host Dependency Definition
--------------------------

Description
^^^^^^^^^^^

Host dependencies are an advanced feature of Centreon Engine that allow
you to suppress active checks of hosts based on the status of one or
more other hosts. Host dependencies are optional and are mainly targeted
at advanced users who have complicated monitoring setups. More
information on how host dependencies work (read this!) can be found
:ref:`here <host_and_service_dependencies>`.

Definition Format
^^^^^^^^^^^^^^^^^

.. note::
   Optional directives are comment (line start with #).

::

  define hostdependency{
    dependent_host_name             host_name
    host_name                       host_name
    # inherits_parent               [0/1]
    # failure_criteria              [o,d,u,p,n]
    # dependency_period             timeperiod_name
  }

Example Definition
^^^^^^^^^^^^^^^^^^

::

  define hostdependency{
    host_name                     WWW1
    dependent_host_name           DBASE1
    failure_criteria              d,u
  }

Directive Descriptions
^^^^^^^^^^^^^^^^^^^^^^

============================= ========================================================================================================================
dependent_host_name           This directive is used to identify the short name(s) of the dependent
                              :ref:`host(s) <obj_def_host>`. Multiple hosts should be separated by commas.
host_name                     This directive is used to identify the short name(s) of the :ref:`host(s) <obj_def_host>`
                              that is being depended upon (also referred to as the master host). Multiple hosts should be separated by commas.
inherits_parent               This directive indicates whether or not the dependency inherits dependencies of the host that is being depended upon
                              (also referred to as the master host). In other words, if the master host is dependent upon other hosts and any one of
                              those dependencies fail, this dependency will also fail.
failure_criteria              This directive is used to specify the criteria that determine when the dependent host should not be actively checked. If
                              the master host is in one of the failure states we specify, the dependent host will not be actively checked. Valid
                              options are a combination of one or more of the following (multiple options are separated with commas): o = fail on an
                              UP state, d = fail on a DOWN state, u = fail on an UNREACHABLE state, and p = fail on a pending state (e.g. the host has
                              not yet been checked). If you specify n (none) as an option, the execution dependency will never fail and the dependent
                              host will always be actively checked (if other conditions allow for it to be). Example: If you specify u,d in this
                              field, the dependent host will not be actively checked if the master host is in either an UNREACHABLE or DOWN state.
dependency_period             This directive is used to specify the short name of the
                              :ref:`time period <obj_def_timeperiod>` during which this dependency is valid. If this
                              directive is not specified, the dependency is considered to be valid during all times.
============================= ========================================================================================================================
