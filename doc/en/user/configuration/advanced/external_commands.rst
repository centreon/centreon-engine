.. _external_commands:

External Commands
*****************

Introduction
============

Centreon Engine can process commands from external applications and
alter various aspects of its monitoring functions based on the commands
it receives. External applications can submit commands by writing to the
:ref:`command file <main_cfg_opt_external_command_file>`,
which is periodically processed by the Centreon Engine daemon.

.. image:: /_static/images/external_commands.png
   :align: center

Enabling External Commands
==========================

In order to have Centreon Engine process external commands, make sure
you do the following:

  * enable external command checking with the
    :ref:`check_external_commands <main_cfg_opt_external_command_check>`
    option
  * set the frequency of command checks with the
    :ref:`command_check_interval <main_cfg_opt_external_command_check_interval>`
    option
  * specify the location of the command file with the
    :ref:`command_file <main_cfg_opt_external_command_file>` option
  * setup proper permissions on the directory containing the external
    command file, as described in the
    :ref:`quickstart guide <exploit_quickstart>`

When Does Centreon Engine Check For External Commands?
======================================================

  * At regular intervals specified by the
    :ref:`command_check_interval <main_cfg_opt_external_command_check_interval>`
    option in the main configuration file
  * Immediately after :ref:`event handlers <event_handlers>` are
    executed. This is in addtion to the regular cycle of external
    command checks and is done to provide immediate action if an event
    handler submits commands to Centreon Engine.

Using External Commands
=======================

External commands can be used to accomplish a variety of things while
Centreon Engine is running. Example of what can be done include
temporarily disabling notifications for services and hosts, temporarily
disabling service checks, forcing immediate service checks, adding
comments to hosts and services, etc.

Command Format
==============

External commands that are written to the
:ref:`command file <main_cfg_opt_external_command_file>`
have the following format::

  [time] command_id;command_arguments

...where time is the time (in time_t format) that the external
application submitted the external command to the command file. The
values for the command_id and command_arguments arguments will depend on
what command is being submitted to Centreon Engine.

A full listing of external commands that can be used (along with
examples of how to use them) can be found online at the following
`URL <http://www.nagios.org/developerinfo/externalcommands/>`_

