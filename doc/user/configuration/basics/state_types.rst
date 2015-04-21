.. _state_types:

State Types
***********

Introduction
============

The current state of monitored services and hosts is determined by two
components:

  * The status of the service or host (i.e. OK, WARNING, UP, DOWN, etc.)
  * Tye type of state the service or host is in

There are two state types in Centreon Engine - SOFT states and HARD
states. These state types are a crucial part of the monitoring logic, as
they are used to determine when :ref:`event handlers <event_handlers>`
are executed.

This document describes the difference between SOFT and HARD states, how
they occur, and what happens when they occur.

Service and Host Check Retries
==============================

In order to prevent false alarms from transient problems, Centreon
Engine allows you to define how many times a service or host should be
(re)checked before it is considered to have a "real" problem. This is
controlled by the max_check_attempts option in the host and service
definitions. Understanding how hosts and services are (re)checked in
order to determine if a real problem exists is important in
understanding how state types work.

Soft States
===========

Soft states occur in the following situations...

  * When a service or host check results in a non-OK or non-UP state and
    the service check has not yet been (re)checked the number of times
    specified by the max_check_attempts directive in the service or host
    definition. This is called a soft error.
  * When a service or host recovers from a soft error. This is
    considered a soft recovery.

The following things occur when hosts or services experience SOFT state
changes:

  * The SOFT state is logged.
  * Event handlers are executed to handle the SOFT state.

SOFT states are only logged if you enabled the
:ref:`log_service_retries <main_cfg_opt_service_check_retry_logging>`
or :ref:`log_host_retries <main_cfg_opt_host_check_retry_logging>`
options in your main configuration file.

The only important thing that really happens during a soft state is the
execution of event handlers. Using event handlers can be particularly
useful if you want to try and proactively fix a problem before it turns
into a HARD state.

The :ref:`HOSTSTATETYPE <user_configuration_macros_host>` or
:ref:`SERVICESTATETYPE <user_configuration_macros_service>` macros will
have a value of "SOFT" when event handlers are executed, which allows
your event handler scripts to know when they should take corrective
action. More information on event handlers can be found
:ref:`here <event_handlers>`.

Hard States
===========

Hard states occur for hosts and services in the following situations:

  * When a host or service check results in a non-UP or non-OK state and
    it has been (re)checked the number of times specified by the
    max_check_attempts option in the host or service definition. This is
    a hard error state.
  * When a host or service transitions from one hard error state to
    another error state (e.g. WARNING to CRITICAL).
  * When a service check results in a non-OK state and its corresponding
    host is either DOWN or UNREACHABLE.
  * When a host or service recovers from a hard error state. This is
    considered to be a hard recovery.
  * When a :ref:`passive host check <passive_checks>` is
    received. Passive host checks are treated as SOFT just like active
    checks.

The following things occur when hosts or services experience HARD state
changes:

  * The HARD state is logged.
  * Event handlers are executed to handle the HARD state.

The :ref:`HOSTSTATETYPE <user_configuration_macros_host>` or
:ref:`SERVICESTATETYPE <user_configuration_macros_service>` macros will
have a value of "HARD" when event handlers are executed, which allows
your event handler scripts to know when they should take corrective
action. More information on event handlers can be found
:ref:`here <event_handlers>`.

Example
=======

Here's an example of how state types are determined, when state changes
occur, and when event handlers executed. The table below shows
consecutive checks of a service over time. The service has a
max_check_attempts value of 3.

==== ===== ======== ========== ============ ============================================
Time Check State    State Type State Change Notes
==== ===== ======== ========== ============ ============================================
0    1     OK       HARD       No           Initial state of the service.
1    1     CRITICAL SOFT       Yes          First detection of a non-OK state.
                                            Event handlers execute.
2    2     WARNING  SOFT       Yes          Service continues to be in a non-OK
                                            state. Event handlers execute.
3    3     CRITICAL HARD       Yes          Max check attempts has been reached,
                                            so service goes into a HARD state. Event
                                            handlers execute. Check is reset to 1
                                            immediately after this happens.
4    1     WARNING  HARD       Yes          Service changes to a HARD WARNING state.
                                            Event handlers execute.
5    1     WARNING  HARD       No           Service stabilizes in a HARD problem
                                            state.
6    1     OK       HARD       Yes          Service experiences a HARD recovery. Event
                                            handlers execute.
7    1     OK       HARD       No           Service is still OK.
8    1     UNKNOWN  OFT        Yes          Service is detected as changing to a SOFT
                                            non-OK state. Event handlers execute.
9    2     OK       SOFT       Yes          Service experiences a SOFT recovery. Event
                                            handlers execute. State type is set HARD
                                            and check is reset to 1 immediately after
                                            this happens.
10   1     OK       HARD       No           Service stabilizes in an OK state.
==== ===== ======== ========== ============ ============================================
