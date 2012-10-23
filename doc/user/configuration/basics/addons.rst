Addons
******

Introduction
============

There are a lot of "addon" software packages that are available for
Centreon Engine. Addons can be used to extend Centreon Engine'
functionality or integrate Centreon Engine with other applications.

Addons are available for:

  * Managing the config files through a web interface
  * Monitoring remote hosts (NIX, Windows ...)
  * Submitting passive checks from remote hosts
  * Simplifying/extending the notification logic
  * ...and much more

You can find many addons for Centreon Engine by visiting:

  * `nagios.org <http://www.nagios.org>`_
  * `Nagios' SourceForge.net page <http://sourceforge.net/projects/nagios>`_
  * `nagiosexchange.org <http://www.nagiosexchange.org>`_

I'll give a brief introduction to a few of the addons that I've
developed for Centreon Engine...

.. _addons_nrpe:

NRPE
====

.. image:: /_static/images/nrpe.png
   :align: center

NRPE is an addon that allows you to execute :ref:`plugins <exploit_plugins>`
on remote Linux/Unix hosts. This is useful if you need to monitor local
resources/attributes like disk usage, CPU load, memory usage, etc. on a
remote host. Similiar functionality can be accomplished by using the
check_by_ssh plugin, although it can impose a higher CPU load on the
monitoring machine - especially if you are monitoring hundreds or
thousands of hosts.

The NRPE addon and documentation can be found at http://www.nagios.org/.

.. _addons_nsca:

NSCA
====

.. image:: /_static/images/nsca.png
   :align: center

NSCA is an addon that allows you to send
:ref:`passive check <passive_checks>`
results from remote Linux/Unix hosts to the Centreon Engine daemon
running on the monitoring server. This is very useful in
:ref:`distributed <distributed_monitoring>` and
:ref:`redundant/failover <redundant_and_failover_monitoring>`
monitoring setups.

The NSCA addon can be found at http://www.nagios.org/.

NDOUtils
========

.. image:: /_static/images/ndoutils.png
   :align: center

NDOUtils is an addon that allows you to store all status information
from Centreon Engine in a MySQL database. Multiple instances of Centreon
Engine can all store their information in a central database for
centralized reporting. This will likely serve as the basis for a new
PHP-based web interface for Centreon Engine in the future.

The NDOUtils addon and documentation can be found at
http://www.nagios.org/.

Nagios Exchange - Hundreds of Other Addons
==========================================

Hundreds of community-developed Centreon Engine addons can be found on
the Nagios Exchange website at exchange.nagios.org
