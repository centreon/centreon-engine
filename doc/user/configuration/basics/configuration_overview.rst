Configuration Overview
**********************

Introduction
============

There are several different configuration files that you're going to
need to create or edit before you start monitoring anything. Be patient!
Configuring Centreon Engine can take quite a while, especially if you're
first-time user. Once you figure out how things work, it'll all be well
worth your time. :-)

.. note::
   Sample configuration files are installed in the
   ``/etc/centreon-engine/`` directory when you follow the
   :ref:`quickstart installation guide <exploit_quickstart>`.

.. image:: /_static/images/configoverview.png
   :align: center

Main
====

Configuration File

The main configuration file contains a number of directives that affect
how the Centreon Engine daemon operates. This is where you're going to
want to get started in your configuration adventures.

Documentation for the main configuration file can be found
:ref:`here <main_cfg_opt>`.

Resource File(s)
================

Resource files can be used to store user-defined macros. The main point
of having resource files is to use them to store sensitive configuration
information (like passwords).

You can specify one or more optional resource files by using the
:ref:`resource_file <main_cfg_opt_resource_file>` directive in your main
configuration file.

Object
======

Definition Files

Object definition files are used to define hosts, services, hostgroups,
contacts, contactgroups, commands, etc. This is where you define all the
things you want monitor and how you want to monitor them.

You can specify one or more object definition files by using the
:ref:`cfg_file <main_cfg_opt_object_configuration_file>` and/or
:ref:`cfg_dir <main_cfg_opt_object_configuration_directory>` directives
in your main configuration file.

An introduction to object definitions, and how they relate to each
other, can be found :ref:`here <object_configuration_overview>`.
