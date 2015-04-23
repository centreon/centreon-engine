.. _standard_macros:

Standard Macros
***************

Macro Validity
==============

Although macros can be used in all commands you define, not all macros
may be "valid" in a particular type of command. For example, some macros
may only be valid during service event handlers, whereas other may only
be valid during host check commands. There are ten types of commands
that Centreon Engine recognizes and treats differently. They are as
follows:

  * Service checks
  * Host checks
  * Service :ref:`event handlers <event_handlers>` and/or a global
    service event handler
  * Host :ref:`event handlers <event_handlers>` and/or a global host
    event handler
  * :ref:`OCSP <main_cfg_opt_obsessive_compulsive_service_processor_command>`
    command
  * :ref:`OCHP <main_cfg_opt_obsessive_compulsive_host_processor_command>`
    command

The tables below list all macros currently available in Centreon Engine,
along with a brief description of each and the types of commands in
which they are valid. If a macro is used in a command in which it is
invalid, it is replaced with an empty string. It should be noted that
macros consist of all uppercase characters and are enclosed in $
characters.

Macro Availability Chart
========================

Legend
------

======= ==========================
No      The macro is not available
**Yes** The macro is available
======= ==========================

.. _user_configuration_macros_host:

Host Macros :sup:`3`
--------------------

============================== ============== ================ =============================== ============================
Macro Name                     Service Checks Host Checks      Service Event Handlers and OCSP Host Event Handlers and OCHP
============================== ============== ================ =============================== ============================
`HOSTNAME`_                    **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTALIAS`_                   **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTADDRESS`_                 **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTSTATE`_                   **Yes**        **Yes** :sup:`1` **Yes**                         **Yes**
`HOSTSTATEID`_                 **Yes**        **Yes** :sup:`1` **Yes**                         **Yes**
`LASTHOSTSTATE`_               **Yes**        **Yes**          **Yes**                         **Yes**
`LASTHOSTSTATEID`_             **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTSTATETYPE`_               **Yes**        **Yes** :sup:`1` **Yes**                         **Yes**
`HOSTATTEMPT`_                 **Yes**        **Yes**          **Yes**                         **Yes**
`MAXHOSTATTEMPTS`_             **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTEVENTID`_                 **Yes**        **Yes**          **Yes**                         **Yes**
`LASTHOSTEVENTID`_             **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTPROBLEMID`_               **Yes**        **Yes**          **Yes**                         **Yes**
`LASTHOSTPROBLEMID`_           **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTLATENCY`_                 **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTEXECUTIONTIME`_           **Yes**        **Yes** :sup:`1` **Yes**                         **Yes**
`HOSTDURATION`_                **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTDURATIONSEC`_             **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTPERCENTCHANGE`_           **Yes**        **Yes**          **Yes**                         **Yes**
`LASTHOSTCHECK`_               **Yes**        **Yes**          **Yes**                         **Yes**
`LASTHOSTSTATECHANGE`_         **Yes**        **Yes**          **Yes**                         **Yes**
`LASTHOSTUP`_                  **Yes**        **Yes**          **Yes**                         **Yes**
`LASTHOSTDOWN`_                **Yes**        **Yes**          **Yes**                         **Yes**
`LASTHOSTUNREACHABLE`_         **Yes**        **Yes**          **Yes**                         **Yes**
`HOSTOUTPUT`_                  **Yes**        **Yes** :sup:`1` **Yes**                         **Yes**
`LONGHOSTOUTPUT`_              **Yes**        **Yes** :sup:`1` **Yes**                         **Yes**
`HOSTPERFDATA`_                **Yes**        **Yes** :sup:`1` **Yes**                         **Yes**
`HOSTCHECKCOMMAND`_            **Yes**        **Yes**          **Yes**                         **Yes**
`TOTALHOSTSERVICES`_           **Yes**        **Yes**          **Yes**                         **Yes**
`TOTALHOSTSERVICESOK`_         **Yes**        **Yes**          **Yes**                         **Yes**
`TOTALHOSTSERVICESWARNING`_    **Yes**        **Yes**          **Yes**                         **Yes**
`TOTALHOSTSERVICESUNKNOWN`_    **Yes**        **Yes**          **Yes**                         **Yes**
`TOTALHOSTSERVICESCRITICAL`_   **Yes**        **Yes**          **Yes**                         **Yes**
============================== ============== ================ =============================== ============================

.. _user_configuration_macros_service:

Service Macros
--------------

================================= ============== =========== =============================== ============================
Macro Name                        Service Checks Host Checks Service Event Handlers and OCSP Host Event Handlers and OCHP
================================= ============== =========== =============================== ============================
`SERVICEDESC`_                    **Yes**        No          **Yes**                         No
`SERVICESTATE`_ :sup:`2`          **Yes**        No          **Yes**                         No
`SERVICESTATEID`_ :sup:`2`        **Yes**        No          **Yes**                         No
`LASTSERVICESTATE`_               **Yes**        No          **Yes**                         No
`LASTSERVICESTATEID`_             **Yes**        No          **Yes**                         No
`SERVICESTATETYPE`_               **Yes**        No          **Yes**                         No
`SERVICEATTEMPT`_                 **Yes**        No          **Yes**                         No
`MAXSERVICEATTEMPTS`_             **Yes**        No          **Yes**                         No
`SERVICEISVOLATILE`_              **Yes**        No          **Yes**                         No
`SERVICEEVENTID`_                 **Yes**        No          **Yes**                         No
`LASTSERVICEEVENTID`_             **Yes**        No          **Yes**                         No
`SERVICEPROBLEMID`_               **Yes**        No          **Yes**                         No
`LASTSERVICEPROBLEMID`_           **Yes**        No          **Yes**                         No
`SERVICELATENCY`_                 **Yes**        No          **Yes**                         No
`SERVICEEXECUTIONTIME`_ :sup:`2`  **Yes**        No          **Yes**                         No
`SERVICEDURATION`_                **Yes**        No          **Yes**                         No
`SERVICEDURATIONSEC`_             **Yes**        No          **Yes**                         No
`SERVICEPERCENTCHANGE`_           **Yes**        No          **Yes**                         No
`LASTSERVICECHECK`_               **Yes**        No          **Yes**                         No
`LASTSERVICESTATECHANGE`_         **Yes**        No          **Yes**                         No
`LASTSERVICEOK`_                  **Yes**        No          **Yes**                         No
`LASTSERVICEWARNING`_             **Yes**        No          **Yes**                         No
`LASTSERVICEUNKNOWN`_             **Yes**        No          **Yes**                         No
`LASTSERVICECRITICAL`_            **Yes**        No          **Yes**                         No
`SERVICEOUTPUT`_ :sup:`2`         **Yes**        No          **Yes**                         No
`LONGSERVICEOUTPUT`_ :sup:`2`     **Yes**        No          **Yes**                         No
`SERVICEPERFDATA`_ :sup:`2`       **Yes**        No          **Yes**                         No
`SERVICECHECKCOMMAND`_            **Yes**        No          **Yes**                         No
================================= ============== =========== =============================== ============================

.. _macros_summary:

Summary Macros
--------------

=========================================== ============== =========== =============================== ============================
Macro Name                                  Service Checks Host Checks Service Event Handlers and OCSP Host Event Handlers and OCHP
=========================================== ============== =========== =============================== ============================
`TOTALHOSTSUP`_ :sup:`5`                    **Yes**        **Yes**     **Yes**                         **Yes**
`TOTALHOSTSDOWN`_ :sup:`5`                  **Yes**        **Yes**     **Yes**                         **Yes**
`TOTALHOSTSUNREACHABLE`_ :sup:`5`           **Yes**        **Yes**     **Yes**                         **Yes**
`TOTALHOSTPROBLEMS`_ :sup:`5`               **Yes**        **Yes**     **Yes**                         **Yes**
`TOTALSERVICESOK`_ :sup:`5`                 **Yes**        **Yes**     **Yes**                         **Yes**
`TOTALSERVICESWARNING`_ :sup:`5`            **Yes**        **Yes**     **Yes**                         **Yes**
`TOTALSERVICESCRITICAL`_ :sup:`5`           **Yes**        **Yes**     **Yes**                         **Yes**
`TOTALSERVICESUNKNOWN`_ :sup:`5`            **Yes**        **Yes**     **Yes**                         **Yes**
`TOTALSERVICEPROBLEMS`_ :sup:`5`            **Yes**        **Yes**     **Yes**                         **Yes**
=========================================== ============== =========== =============================== ============================

Date/Time Macros
----------------

========================= ============== =========== =============================== ============================
Macro Name                Service Checks Host Checks Service Event Handlers and OCSP Host Event Handlers and OCHP
========================= ============== =========== =============================== ============================
`TIMET`_                  **Yes**        **Yes**     **Yes**                         **Yes**
`ISVALIDTIME`_ :sup:`4`   **Yes**        **Yes**     **Yes**                         **Yes**
`NEXTVALIDTIME`_ :sup:`4` **Yes**        **Yes**     **Yes**                         **Yes**
========================= ============== =========== =============================== ============================

File Macros
-----------

====================== ============== =========== =============================== ============================
Macro Name             Service Checks Host Checks Service Event Handlers and OCSP Host Event Handlers and OCHP
====================== ============== =========== =============================== ============================
`MAINCONFIGFILE`_      **Yes**        **Yes**     **Yes**                         **Yes**
`STATUSDATAFILE`_      **Yes**        **Yes**     **Yes**                         **Yes**
`RETENTIONDATAFILE`_   **Yes**        **Yes**     **Yes**                         **Yes**
`LOGFILE`_             **Yes**        **Yes**     **Yes**                         **Yes**
`COMMANDFILE`_         **Yes**        **Yes**     **Yes**                         **Yes**
====================== ============== =========== =============================== ============================

.. _user_configuration_macros_misc:

Misc Macros
-----------

=================== ============== =========== =============================== ============================
Macro Name          Service Checks Host Checks Service Event Handlers and OCSP Host Event Handlers and OCHP
=================== ============== =========== =============================== ============================
`PROCESSSTARTTIME`_ **Yes**        **Yes**     **Yes**                         **Yes**
`EVENTSTARTTIME`_   **Yes**        **Yes**     **Yes**                         **Yes**
`ARGn`_             **Yes**        **Yes**     **Yes**                         **Yes**
`USERn`_            **Yes**        **Yes**     **Yes**                         **Yes**
=================== ============== =========== =============================== ============================

Macro Descriptions
==================

Host Macros :sup:`3`
--------------------

============================== =========================================================================================================================
_`HOSTNAME`                    Short name for the host (i.e. "biglinuxbox"). This value is taken from the host_name directive in the
                               :ref:`host definition <obj_def_host>`.
_`HOSTALIAS`                   Long name/description for the host. This value is taken from the alias directive in the
                               :ref:`host definition <obj_def_host>`.
_`HOSTADDRESS`                 Address of the host. This value is taken from the address directive in the
                               :ref:`host definition <obj_def_host>`.
_`HOSTSTATE`                   A string indicating the current state of the host ("UP", "DOWN", or "UNREACHABLE").
_`HOSTSTATEID`                 A number that corresponds to the current state of the host: 0=UP, 1=DOWN, 2=UNREACHABLE.
_`LASTHOSTSTATE`               A string indicating the last state of the host ("UP", "DOWN", or "UNREACHABLE").
_`LASTHOSTSTATEID`             A number that corresponds to the last state of the host: 0=UP, 1=DOWN, 2=UNREACHABLE.
_`HOSTSTATETYPE`               A string indicating the :ref:`state type <state_types>` for the current host check ("HARD" or "SOFT"). Soft states occur
                               when host checks return a non-OK (non-UP) state and are in the process of being retried. Hard states result when host
                               checks have been checked a specified maximum number of times.
_`HOSTATTEMPT`                 The number of the current host check retry. For instance, if this is the second time that the host is being rechecked,
                               this will be the number two. Current attempt number is really only useful when writing host event handlers for "soft"
                               states that take a specific action based on the host retry number.
_`MAXHOSTATTEMPTS`             The max check attempts as defined for the current host. Useful when writing host event handlers for "soft" states that
                               take a specific action based on the host retry number.
_`HOSTEVENTID`                 A globally unique number associated with the host's current state. Every time a host (or service) experiences a state
                               change, a global event ID number is incremented by one (1). If a host has experienced no state changes, this macro will
                               be set to zero (0).
_`LASTHOSTEVENTID`             The previous (globally unique) event number that was given to the host.
_`HOSTPROBLEMID`               A globally unique number associated with the host's current problem state. Every time a host (or service) transitions
                               from an UP or OK state to a problem state, a global problem ID number is incremented by one (1). This macro will be
                               non-zero if the host is currently a non-UP state. State transitions between non-UP states (e.g. DOWN to UNREACHABLE) do
                               not cause this problem id to increase. If the host is currently in an UP state, this macro will be set to zero (0).
                               Combined with event handlers, this macro could be used to automatically open trouble tickets when hosts first enter a
                               problem state.
_`LASTHOSTPROBLEMID`           The previous (globally unique) problem number that was given to the host. Combined with event handlers, this macro could
                               be used for automatically closing trouble tickets, etc. when a host recovers to an UP state.
_`HOSTLATENCY`                 A (floating point) number indicating the number of seconds that a scheduled host check lagged behind its scheduled check
                               time. For instance, if a check was scheduled for 03:14:15 and it didn't get executed until 03:14:17, there would be a
                               check latency of 2.0 seconds. On-demand host checks have a latency of zero seconds.
_`HOSTEXECUTIONTIME`           A (floating point) number indicating the number of seconds that the host check took to execute (i.e. the amount of time
                               the check was executing).
_`HOSTDURATION`                A string indicating the amount of time that the host has spent in its current state. Format is "XXh YYm ZZs", indicating
                               hours, minutes and seconds.
_`HOSTDURATIONSEC`             A number indicating the number of seconds that the host has spent in its current state.
_`HOSTPERCENTCHANGE`           A (floating point) number indicating the percent state change the host has undergone. Percent state change is used by the
                               :ref:`flap detection <flapping_detection>` algorithm.
_`LASTHOSTCHECK`               This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which a check of the host was
                               last performed.
_`LASTHOSTSTATECHANGE`         This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time the host last changed state.
_`LASTHOSTUP`                  This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the host was last
                               detected as being in an UP state.
_`LASTHOSTDOWN`                This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the host was last
                               detected as being in a DOWN state.
_`LASTHOSTUNREACHABLE`         This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the host was last
                               detected as being in an UNREACHABLE state.
_`HOSTOUTPUT`                  The first line of text output from the last host check (i.e. "Ping OK").
_`LONGHOSTOUTPUT`              The full text output (aside from the first line) from the last host check.
_`HOSTPERFDATA`                This macro contains any :ref:`performance data <performance_data>` that may have been returned by the last host
                               check.
_`HOSTCHECKCOMMAND`            This macro contains the name of the command (along with any arguments passed to it) used to perform the host check.
_`TOTALHOSTSERVICES`           The total number of services associated with the host.
_`TOTALHOSTSERVICESOK`         The total number of services associated with the host that are in an OK state.
_`TOTALHOSTSERVICESWARNING`    The total number of services associated with the host that are in a WARNING state.
_`TOTALHOSTSERVICESUNKNOWN`    The total number of services associated with the host that are in an UNKNOWN state.
_`TOTALHOSTSERVICESCRITICAL`   The total number of services associated with the host that are in a CRITICAL state.
============================== =========================================================================================================================

Service Macros
--------------

================================= ======================================================================================================================
_`SERVICEDESC`                    The long name/description of the service (i.e. "Main Website"). This value is taken from the service_description
                                  directive of the :ref:`service definition <obj_def_service>`.
_`SERVICESTATE`                   A string indicating the current state of the service ("OK", "WARNING", "UNKNOWN", or "CRITICAL").
_`SERVICESTATEID`                 A number that corresponds to the current state of the service: 0=OK, 1=WARNING, 2=CRITICAL, 3=UNKNOWN.
_`LASTSERVICESTATE`               A string indicating the last state of the service ("OK", "WARNING", "UNKNOWN", or "CRITICAL").
_`LASTSERVICESTATEID`             A number that corresponds to the last state of the service: 0=OK, 1=WARNING, 2=CRITICAL, 3=UNKNOWN.
_`SERVICESTATETYPE`               A string indicating the :ref:`state type <state_types>` for the current service check ("HARD" or "SOFT"). Soft states
                                  occur when service checks return a non-OK state and are in the process of being retried. Hard states result when
                                  service checks have been checked a specified maximum number of times.
_`SERVICEATTEMPT`                 The number of the current service check retry. For instance, if this is the second time that the service is being
                                  rechecked, this will be the number two. Current attempt number is really only useful when writing service event
                                  handlers for "soft" states that take a specific action based on the service retry number.
_`MAXSERVICEATTEMPTS`             The max check attempts as defined for the current service. Useful when writing host event handlers for "soft" states
                                  that take a specific action based on the service retry number.
_`SERVICEISVOLATILE`              Indicates whether the service is marked as being volatile or not: 0 = not volatile, 1 = volatile.
_`SERVICEEVENTID`                 A globally unique number associated with the service's current state. Every time a a service (or host) experiences a
                                  state change, a global event ID number is incremented by one (1). If a service has experienced no state changes, this
                                  macro will be set to zero (0).
_`LASTSERVICEEVENTID`             The previous (globally unique) event number that given to the service.
_`SERVICEPROBLEMID`               A globally unique number associated with the service's current problem state. Every time a service (or host)
                                  transitions from an OK or UP state to a problem state, a global problem ID number is incremented by one (1). This
                                  macro will be non-zero if the service is currently a non-OK state. State transitions between non-OK states (e.g.
                                  WARNING to CRITICAL) do not cause this problem id to increase. If the service is currently in an OK state, this macro
                                  will be set to zero (0). Combined with event handlers, this macro could be used to automatically open trouble tickets
                                  when services first enter a problem state.
_`LASTSERVICEPROBLEMID`           The previous (globally unique) problem number that was given to the service. Combined with event handlers, this macro
                                  could be used for automatically closing trouble tickets, etc. when a service recovers to an OK state.
_`SERVICELATENCY`                 A (floating point) number indicating the number of seconds that a scheduled service check lagged behind its scheduled
                                  check time. For instance, if a check was scheduled for 03:14:15 and it didn't get executed until 03:14:17, there would
                                  be a check latency of 2.0 seconds.
_`SERVICEEXECUTIONTIME`           A (floating point) number indicating the number of seconds that the service check took to execute (i.e. the amount of
                                  time the check was executing).
_`SERVICEDURATION`                A string indicating the amount of time that the service has spent in its current state. Format is "XXh YYm ZZs",
                                  indicating hours, minutes and seconds.
_`SERVICEDURATIONSEC`             A number indicating the number of seconds that the service has spent in its current state.
_`SERVICEPERCENTCHANGE`           A (floating point) number indicating the percent state change the service has undergone. Percent state change is used
                                  by the :ref:`flap detection <flapping_detection>` algorithm.
_`LASTSERVICECHECK`               This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which a check of the
                                  service was last performed.
_`LASTSERVICESTATECHANGE`         This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time the service last changed
                                  state.
_`LASTSERVICEOK`                  This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                  detected as being in an OK state.
_`LASTSERVICEWARNING`             This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                  detected as being in a WARNING state.
_`LASTSERVICEUNKNOWN`             This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                  detected as being in an UNKNOWN state.
_`LASTSERVICECRITICAL`            This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                  detected as being in a CRITICAL state.
_`SERVICEOUTPUT`                  The first line of text output from the last service check (i.e. "Ping OK").
_`LONGSERVICEOUTPUT`              The full text output (aside from the first line) from the last service check.
_`SERVICEPERFDATA`                This macro contains any :ref:`performance data <performance_data>` that may have been returned by the last
                                  service check.
_`SERVICECHECKCOMMAND`            This macro contains the name of the command (along with any arguments passed to it) used to perform the service check.
================================= ======================================================================================================================

Summary Macros
--------------

================================= =======================================================================================================================
_`TOTALHOSTSUP`                   This macro reflects the total number of hosts that are currently in an UP state.
_`TOTALHOSTSDOWN`                 This macro reflects the total number of hosts that are currently in a DOWN state.
_`TOTALHOSTSUNREACHABLE`          This macro reflects the total number of hosts that are currently in an UNREACHABLE state.
_`TOTALHOSTPROBLEMS`              This macro reflects the total number of hosts that are currently either in a DOWN or an UNREACHABLE state.
_`TOTALSERVICESOK`                This macro reflects the total number of services that are currently in an OK state.
_`TOTALSERVICESWARNING`           This macro reflects the total number of services that are currently in a WARNING state.
_`TOTALSERVICESCRITICAL`          This macro reflects the total number of services that are currently in a CRITICAL state.
_`TOTALSERVICESUNKNOWN`           This macro reflects the total number of services that are currently in an UNKNOWN state.
_`TOTALSERVICEPROBLEMS`           This macro reflects the total number of services that are currently either in a WARNING, CRITICAL, or UNKNOWN state.
================================= =======================================================================================================================

Date/Time Macros
----------------

========================= ===============================================================================================================================
_`TIME`                   Current time stamp (i.e. 00:30:28).
_`TIMET`                  Current time stamp in time_t format (seconds since the UNIX epoch).
_`ISVALIDTIME` :sup:`4`   This is a special on-demand macro that returns a 1 or 0 depending on whether or not a particular time is valid within a
                          specified timeperiod. There are two ways of using this macro:

                            * $ISVALIDTIME:24x7$ will be set to "1" if the current time is valid within the "24x7" timeperiod. If not, it will be set to
                              "0".
                            * $ISVALIDTIME:24x7:timestamp$ will be set to "1" if the time specified by the "timestamp" argument (which must be in time_t
                              format) is valid within the "24x7" timeperiod. If not, it will be set to "0".
_`NEXTVALIDTIME` :sup:`4` This is a special on-demand macro that returns the next valid time (in time_t format) for a specified timeperiod. There are two
                          ways of using this macro:

                            * $NEXTVALIDTIME:24x7$ will return the next valid time from and including the current time in the "24x7" timeperiod.
                            * $NEXTVALIDTIME:24x7:timestamp$ will return the next valid time from and including the time specified by the "timestamp"
                              argument (which must be specified in time_t format) in the "24x7" timeperiod.If a next valid time cannot be found in the
                              specified timeperiod, the macro will be set to "0".
========================= ===============================================================================================================================

File Macros
-----------

====================== ==================================================================================================================================
_`MAINCONFIGFILE`      The location of the :ref:`main config file <main_cfg_opt>`.
_`STATUSDATAFILE`      The location of the :ref:`status data file <main_cfg_opt_status_file>`.
_`RETENTIONDATAFILE`   The location of the :ref:`retention data file <main_cfg_opt_state_retention_file>`.
_`LOGFILE`             The location of the :ref:`log file <main_cfg_opt_log_file>`.
_`COMMANDFILE`         The location of the :ref:`command file <main_cfg_opt_external_command_file>`.
====================== ==================================================================================================================================

Misc Macros
-----------

=================== =====================================================================================================================================
_`PROCESSSTARTTIME` Time stamp in time_t format (seconds since the UNIX epoch) indicating when the Centreon Engine process was last (re)started. You can
                    determine the number of seconds that Centreon Engine has been running (since it was last restarted) by subtracting $PROCESSSTARTTIME$
                    from `TIMET`_.
_`EVENTSTARTTIME`   Time stamp in time_t format (seconds since the UNIX epoch) indicating when the Centreon Engine process starting process events
                    (checks, etc.). You can determine the number of seconds that it took for Centreon Engine to startup by subtracting $PROCESSSTARTTIME$
                    from $EVENTSTARTTIME$.
_`ARGn`             The nth argument passed to the command (event handler, service check, etc.). Centreon Engine supports up to 32 argument macros
                    ($ARG1$ through $ARG32$).
_`USERn`            The nth user-definable macro.
=================== =====================================================================================================================================

Notes
=====

  * :sup:`1` These macros are not valid for the host they are
    associated with when that host is being checked (i.e. they make no
    sense, as they haven't been determined yet).
  * :sup:`2` These macros are not valid for the service they are
    associated with when that service is being checked (i.e. they make
    no sense, as they haven't been determined yet).
  * :sup:`3` When host macros are used in service-related commands
    (i.e. service event handlers, etc) they refer to they host that
    they service is associated with.
  * :sup:`4` These macro are only available as on-demand macros -
    e.g. you must supply an additional argument with them in order to
    use them. These macros are not available as environment variables.
  * :sup:`5` Summary macros are quite CPU-intensive to calculate.
