.. _predictive_dependency_checks:

Predictive Dependency Checks
****************************

Introduction
============

Host and service :ref:`dependencies <host_and_service_dependencies>` can
be defined to allow you greater control over when checks are executed
and when notifications are sent out. As dependencies are used to control
basic aspects of the monitoring process, it is crucial to ensure that
status information used in the dependency logic is as up to date as
possible.

Centreon Engine allows you to enable predictive dependency checks for
hosts and services to ensure that the dependency logic will have the
most up-to-date status information when it comes to making decisions
about whether to send out notifications or allow active checks of a host
or service.

How Do Predictive Checks Work?
==============================

The image below shows a basic diagram of hosts that are being monitored
by Centreon Engine, along with their parent/child relationships and
dependencies.

The Switch2 host in this example has just changed state from an UP state
to a problem state. Centreon Engine needs to determine whether the host
is DOWN or UNREACHABLE, so it will launch parallel checks of Switch2's
immediate parents (Firewall1) and children (Comp1, Comp2, and
Switch3). This is a normal function of the
:ref:`host reachability <status_and_reachability_network>`
logic.

You will also notice that Switch2 is depending on Monitor1 and File1 for
either notifications or check execution (which one is unimportant in
this example). If predictive host dependency checks are enabled,
Centreon Engine will launch parallel checks of Monitor1 and File1 at the
same time it launches checks of Switch2's immediate parents and
children. Centreon Engine does this because it knows that it will have
to test the dependency logic in the near future (e.g. for purposes of
notification) and it wants to make sure it has the most current status
information for the hosts that take part in the dependency.

.. image:: /_static/images/predictive-dependency-checks.png
   :align: center

That's how predictive dependency checks work. Simple, eh?

.. note::
   Predictive service dependency checks work in a similiar manner to
   what is described above. Except, of course, they deal with services
   instead of hosts.

Enabling Predictive Checks
==========================

Predictive dependency checks involve rather little overhead, so I would
recommend that you enable them. In most cases, the benefits of having
accurate information for the dependency logic outweighs the extra
overhead imposed by these checks.

Enabling predictive dependency checks is easy:

  * Predictive host dependency checks are controlled by the
    :ref:`enable_predictive_host_dependency_checks <main_cfg_opt_predictive_host_dependency_checks>`
    option.
  * Predictive service dependency checks are controlled by the
    :ref:`enable_predictive_service_dependency_checks <main_cfg_opt_predictive_service_dependency_checks>`
    option.



Cached Checks
=============

Predictive dependency checks are on-demand checks and are therefore
subject to the rules of :ref:`cached checks <cached_checks>`. Cached
checks can provide you with performance improvements by allowing
Centreon Engine to forgo running an actual host or service check if it
can use a relatively recent check result instead. More information on
cached checks can be found :ref:`here <cached_checks>`.

