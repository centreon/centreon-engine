.. _performance_data:

Performance Data
****************

Introduction
============

Centreon Engine is designed to allow :ref:`plugins <exploit_plugins>` to
return optional performance data in addition to normal status data, as
well as allow you to pass that performance data to external applications
for processing. A description of the different types of performance
data, as well as information on how to go about processing that data is
described below...

Types of Performance Data
=========================

There are two basic categories of performance data that can be obtained
from Centreon Engine:

  * Check performance data
  * Plugin performance data

Check performance data is internal data that relates to the actual
execution of a host or service check. This might include things like
service check latency (i.e. how "late" was the service check from its
scheduled execution time) and the number of seconds a host or service
check took to execute. This type of performance data is available for
all checks that are performed. The
:ref:`HOSTEXECUTIONTIME <user_configuration_macros_host>` and
:ref:`SERVICEEXECUTIONTIME <user_configuration_macros_service>`
:ref:`macros <understanding_macros>` can be used to determine the number
of seconds a host or service check was running and the
:ref:`HOSTLATENCY <user_configuration_macros_host>` and
:ref:`SERVICELATENCY <user_configuration_macros_service>` macros can be
used to determine how "late" a regularly-scheduled host or service check
was.

Plugin performance data is external data specific to the plugin used to
perform the host or service check. Plugin-specific data can include
things like percent packet loss, free disk space, processor load, number
of current users, etc. - basically any type of metric that the plugin is
measuring when it executes. Plugin-specific performance data is optional
and may not be supported by all plugins. Plugin-specific performance
data (if available) can be obtained by using the
:ref:`HOSTPERFDATA <user_configuration_macros_host>` and
:ref:`SERVICEPERFDATA <user_configuration_macros_service>`
:ref:`macros <understanding_macros>`. Read on for more information on
how plugins can return performance data to Centreon Engine for inclusion
in the :ref:`HOSTPERFDATA <user_configuration_macros_host>` and
:ref:`SERVICEPERFDATA <user_configuration_macros_service>` macros.

Plugin Performance Data
=======================

At a minimum, Centreon Engine plugins must return a single line of
human-readable text that indicates the status of some type of measurable
data. For example, the check_ping plugin might return a line of text
like the following::

  PING ok - Packet loss = 0%, RTA = 0.80 ms

With this simple type of output, the entire line of text is available in
the $HOSTOUTPUT$ or $SERVICEOUTPUT$ :ref:`macros <understanding_macros>`
(depending on whether this plugin was used as a host check or service
check).

Plugins can return optional performance data in their output by sending
the normal, human-readable text string that they usually would, followed
by a pipe character (|), and then a string containing one or more
performance data metrics. Let's take the check_ping plugin as an example
and assume that it has been enhanced to return percent packet loss and
average round trip time as performance data metrics. Sample output from
the plugin might look like this::

  PING ok - Packet loss = 0%, RTA = 0.80 ms | percent_packet_loss=0, rta=0.80

When Centreon Engine sees this plugin output format it will split the
output into two parts:

  * Everything before the pipe character is considered to be the
    "normal" plugin output and will be stored in either the $HOSTOUTPUT$
    or $SERVICEOUTPUT$ macro
  * Everything after the pipe character is considered to be the
    plugin-specific performance data and will be stored in the
    $HOSTPERFDATA$ or $SERVICEPERFDATA$ macro

In the example above, the $HOSTOUTPUT$ or $SERVICEOUTPUT$ macro would
contain "PING ok - Packet loss = 0%, RTA = 0.80 ms" (without quotes) and
the $HOSTPERFDATA$ or $SERVICEPERFDATA$ macro would contain
"percent_packet_loss=0, rta=0.80" (without quotes).

Multiple lines of performace data (as well as normal text output) can be
obtained from plugins, as described in the
:ref:`plugin API documentation <centengine_plugin_api>`.

.. note::
   The Centreon Engine daemon doesn't directly process plugin
   performance data, so it doesn't really care what the performance data
   looks like. There aren't really any inherent limitations on the
   format or content of the performance data. However, if you are using
   an external addon to process the performance data (i.e. PerfParse),
   the addon may be expecting that the plugin returns performance data
   in a specific format. Check the documentation that comes with the
   addon for more information.

Processing Performance Data
===========================

If you want to process the performance data that is available from
Centreon Engine and the plugins, you'll need to use some external module
such as Centreon Broker.
