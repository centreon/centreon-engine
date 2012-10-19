Integration Overview
********************

Introduction
============

One of the reasons that Centreon Engine is such a popular monitoring
application is the fact that it can be easily integrated in your
existing infrastructure. There are several methods of integrating
Centreon Engine with the management software you're already using and
you can monitor almost any type of new or custom hardware, service, or
application that you might have.

Integration Points

.. image:: integration_overview.png

To monitor new hardware, services, or applications, check out the docs
on:

  * :ref:`Plugins <get_started/plugins>`
  * :ref:`Plugin API <advanced_centengine_plugin_api>`
  * :ref:`Passive Checks <basics_passive_checks>`
  * :ref:`Event Handlers <advanced_event_handlers>`

To get data into Centreon Engine from external applications, check out
the docs on:

  * :ref:`Passive Checks <basics_passive_checks>`
  * :ref:`External Commands <advanced_external_commands>`

To send status, performance, or notification information from Centreon
Engine to external applications, check out the docs on:

  * :ref:`Event Handlers <advanced_event_handlers>`
  * :ref:`OCSP <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsiveserviceprocessorcommand>`    and
    :ref:`OCHP <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsivehostprocessorcommand>`
    Commands
  * :ref:`Performance Data <advanced_performance_data>`
  * :ref:`Notifications <basics_notifications>`

Integration Examples

I've documented some examples on how to integrate Centreon Engine with
external applications:

  * :ref:`TCP Wrappers <advanced_tcp_wrapper_integration>` (security alerts)
  * :ref:`SNMP Traps <advanced_snmp_wrapper_integration>` (Arcserve backup job status)
