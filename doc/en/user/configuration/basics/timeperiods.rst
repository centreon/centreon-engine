.. _timeperiods:

Time Periods
************

Introduction
============

:ref:`Timeperiod <obj_def_timeperiod>` definitions allow you to control
when various aspects of the monitoring and alerting logic can
operate. For instance, you can restrict:

  * When regularly scheduled host and service checks can be performed
  * When notifications can be sent out
  * When notification escalations can be used
  * When dependencies are valid

Precedence in Time Periods
==========================

Timeperod :ref:`definitions <obj_def_timeperiod>` may contain multiple
types of directives, including weekdays, days of the month, and calendar
dates. Different types of directives have different precendence levels
and may override other directives in your timeperiod definitions. The
order of precedence for different types of directives (in descending
order) is as follows:

  * Calendar date (2008-01-01)
  * Specific month date (January 1st)
  * Generic month date (Day 15)
  * Offset weekday of specific month (2nd Tuesday in December)
  * Offset weekday (3rd Monday)
  * Normal weekday (Tuesday)

Examples of different timeperiod directives can be found
:ref:`here <obj_def_timeperiod>`.

Timeperiods and Daylight Savings Time (DST)
===========================================

Timezones can be defined at global, host, service or contact level.
There are however some edge cases with timeranges around DST changes.

The behavior of timeranges which either start or end during DST 'gap'
(forward change) or 'overlap' (backward change) is operating-system
defined. Specifically it is mktime implementation defined. So as a rule
of thumb you should avoid timeranges that might start or end during DST.

In the Europe/Paris timezone for example, you need to avoid using
timeranges that might start in DST (02:30-*) as well as timeranges
that might end in DST (*-02:30). Timeranges that cover the entire
02:00-03:00 range are OK (00:00-06:00).

If you use timeranges that start or end within a DST, no guess can
be made on how the timeperiod will behave until one hour after the
end of the DST hour (4 o'clock if the DST occured at 3 o'clock).
After this delay, timeperiods should work again as expected.

How Time Periods Work With Host and Service Checks
==================================================

Host and service definitions have an optional check_period directive
that allows you to specify a timeperiod that should be used to restrict
when regularly scheduled, active checks of the host or service can be
made.

If you do not use the check_period directive to specify a timeperiod,
Centreon Engine will be able to schedule active checks of the host or
service anytime it needs to. This is essentially a 24x7 monitoring
scenario.

Specifying a timeperiod in the check_period directive allows you to
restrict the time that Centreon Engine perform regularly scheduled,
active checks of the host or service. When Centreon Engine attempts to
reschedule a host or service check, it will make sure that the next
check falls within a valid time range within the defined timeperiod. If
it doesn't, Centreon Engine will adjust the next check time to coincide
with the next "valid" time in the specified timeperiod. This means that
the host or service may not get checked again for another hour, day, or
week, etc.

.. note::
   On-demand checks and passive checks are not restricted by the
   timeperiod you specify in the check_period directive. Only regularly
   scheduled active checks are restricted.

Unless you have a good reason not to do so, I would recommend that you
monitor all your hosts and services using timeperiods that cover a 24x7
time range. If you don't do this, you can run into some problems during
"blackout" times (times that are not valid in the timeperiod
definition):

1. The status of the host or service will appear unchanged during the
blackout time.
2. Contacts will mostly likely not get re-notified of problems with a
host or service during blackout times.
3. If a host or service recovers during a blackout time, contacts will
not be immediately notified of the recovery.

How Time Periods Work With Contact Notifications
================================================

By specifying a timeperiod in the notification_period directive of a
host or service definition, you can control when Centreon Engine is
allowed to send notifications out regarding problems or recoveries for
that host or service. When a host notification is about to get sent out,
Centreon Engine will make sure that the current time is within a valid
range in the notification_period timeperiod. If it is a valid time, then
Centreon Engine will attempt to notify each contact of the problem or
recovery.

You can also use timeperiods to control when notifications can be sent
out to individual contacts. By using the service_notification_period and
host_notification_period directives in
:ref:`contact definitions <obj_def_contact>`, you're able to essentially
define an "on call" period for each contact. Contacts will only receive
host and service notifications during the times you specify in the
notification period directives.

Examples of how to create timeperiod definitions for use for on-call
rotations can be found here.

How Time Periods Work With Notification Escalations
===================================================

Service and host notification escalations have an optional
escalation_period directive that allows you to specify a timeperiod when
the escalation is valid and can be used. If you do not use the
escalation_period directive in an escalation definition, the escalation
is considered valid at all times. If you specify a timeperiod in the
escalation_period directive, Centreon Engine will only use the
escalation definition during times that are valid in the timeperiod
definition.

How Time Periods Work With Dependencies
=======================================

By specifying a timeperiod in the notification_period directive of a
host or service definition, you can control when Centreon Engine is
allowed to send notifications out regarding problems or recoveries for
that host or service. When a host notification is about to get sent out,
Centreon Engine will make sure that the current time is within a valid
range in the notification_period timeperiod. If it is a valid time, then
Centreon Engine will attempt to notify each contact of the problem or
recovery.

You can also use timeperiods to control when notifications can be sent
out to individual contacts. By using the service_notification_period and
host_notification_period directives in contact definitions, you're able
to essentially define an "on call" period for each contact. Contacts
will only receive host and service notifications during the times you
specify in the notification period directives.

Examples of how to create timeperiod definitions for use for on-call
rotations can be found :ref:`here <on_call_rotations>`.

How Time Periods Work With Notification Escalations
===================================================

Service and host
:ref:`notification escalations <notification_escalations>` have an
optional escalation_period directive that allows you to specify a
timeperiod when the escalation is valid and can be used. If you do not
use the escalation_period directive in an escalation definition, the
escalation is considered valid at all times. If you specify a timeperiod
in the escalation_period directive, Centreon Engine will only use the
escalation definition during times that are valid in the timeperiod
definition.

How Time Periods Work With Dependencies
=======================================

Service and host :ref:`dependencies <host_and_service_dependencies>`
have an optional dependency_period directive that allows you to specify
a timeperiod when the dependendies are valid and can be used. If you do
not use the dependency_period directive in a dependency definition, the
dependency can be used at any time. If you specify a timeperiod in the
dependency_period directive, Centreon Engine will only use the
dependency definition during times that are valid in the timeperiod
definition.
