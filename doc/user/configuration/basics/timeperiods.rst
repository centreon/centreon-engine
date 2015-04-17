.. _timeperiods:

Time Periods
************

Introduction
============

:ref:`Timeperiod <obj_def_timeperiod>` definitions allow you to control
when various aspects of the monitoring and alerting logic can
operate. For instance, you can restrict:

  * When regularly scheduled host and service checks can be performed
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
definition) like the status of the host or service will appear
unchanged during the blackout time.

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
