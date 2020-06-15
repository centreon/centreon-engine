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

.. image:: /_static/images/integration_overview.png
   :align: center

To monitor new hardware, services, or applications, check out the docs
on:

  * :ref:`Plugins <exploit_plugins>`
  * :ref:`Plugin API <centengine_plugin_api>`
  * :ref:`Passive Checks <passive_checks>`
  * :ref:`Event Handlers <event_handlers>`

To get data into Centreon Engine from external applications, check out
the docs on:

  * :ref:`Passive Checks <passive_checks>`
  * :ref:`External Commands <external_commands>`

To send status, performance, or notification information from Centreon
Engine to external applications, check out the docs on:

  * :ref:`Event Handlers <event_handlers>`
  * :ref:`OCSP <main_cfg_opt_obsessive_compulsive_service_processor_command>`
    and
    :ref:`OCHP <main_cfg_opt_obsessive_compulsive_host_processor_command>`
    Commands
  * :ref:`Performance Data <performance_data>`
  * :ref:`Notifications <notifications>`

Integration Examples

I've documented some examples on how to integrate Centreon Engine with
external applications:

  * :ref:`TCP Wrappers <tcp_wrapper_integration>` (security alerts)
  * :ref:`SNMP Traps <snmp_wrapper_integration>` (Arcserve backup job status)
