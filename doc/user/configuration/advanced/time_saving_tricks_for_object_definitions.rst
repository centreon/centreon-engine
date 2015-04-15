.. _obj_def_tricks:

Time-Saving Tricks For Object Definitions
*****************************************

Introduction
============

This documentation attempts to explain how you can exploit the
(somewhat) hidden features of
:ref:`template-based object definitions <obj_def>` to
save your sanity. How so, you ask?  Several types of objects allow you
to specify multiple host names in definitions, allowing you to "copy"
the object defintion to multiple hosts or services. I'll cover each
type of object that supports these features seperately. For starters,
the object types which support this time-saving feature are as follows:

  * :ref:`Services <obj_def_tricks_service>`
  * :ref:`Service escalations <obj_def_tricks_service_escalation>`
  * :ref:`Service dependencies <obj_def_tricks_service_dependency>`
  * :ref:`Host escalations <obj_def_tricks_host_escalation>`
  * :ref:`Host dependencies <obj_def_tricks_host_dependency>`

Object types that are not listed above (i.e. timeperiods, commands,
etc.) do not support the features I'm about to describe.

.. _obj_def_tricks_service:

Service Definitions
===================

Multiple Hosts: If you want to create identical
:ref:`services <obj_def_service>` that are assigned to multiple hosts,
you can specify multiple hosts in the host_name directive. The
definition below would create a service called SOMESERVICE on hosts
HOST1 through HOSTN. All the instances of the SOMESERVICE service would
be identical (i.e. have the same check command, max check attempts,
notification period, etc.)::

  define service{
    host_name           HOST1,HOST2,HOST3,...,HOSTN
    service_description SOMESERVICE
    other service directives ...
  }

All Hosts: If you want to create identical services that are assigned to
all hosts that are defined in your configuration files, you can use a
wildcard in the host_name directive. The definition below would create a
service called SOMESERVICE on all hosts that are defined in your
configuration files. All the instances of the SOMESERVICE service would
be identical (i.e. have the same check command, max check attempts,
notification period, etc.)::

  define service{
    host_name           *
    service_description SOMESERVICE
    other service directives ...
  }

.. _obj_def_tricks_service_escalation:

Service Escalation Definitions
==============================

Multiple Hosts: If you want to create
:ref:`service escalations <obj_def_service_escalation>`
for services of the same name/description that are assigned to multiple
hosts, you can specify multiple hosts in the host_name directive. The
definition below would create a service escalation for services called
SOMESERVICE on hosts HOST1 through HOSTN. All the instances of the
service escalation would be identical (i.e. have the same contact
groups, notification interval, etc.)::

  define serviceescalation{
    host_name           HOST1,HOST2,HOST3,...,HOSTN
    service_description SOMESERVICE
    other escalation directives ...
  }

All Hosts: If you want to create identical service escalations for
services of the same name/description that are assigned to all hosts
that are defined in your configuration files, you can use a wildcard in
the host_name directive. The definition below would create a service
escalation for all services called SOMESERVICE on all hosts that are
defined in your configuration files. All the instances of the service
escalation would be identical (i.e. have the same contact groups,
notification interval, etc.)::

  define serviceescalation{
    host_name           *
    service_description SOMESERVICE
    other escalation directives ...
  }

All Services On Same Host: If you want to create
:ref:`service escalations <obj_def_service_escalation>` for all services
assigned to a particular host, you can use a wildcard in the
service_description directive. The definition below would create a
service escalation for all services on host HOST1. All the instances of
the service escalation would be identical (i.e. have the same contact
groups, notification interval, etc.).

If you feel like being particularly adventurous, you can specify a
wildcard in both the host_name and service_description directives. Doing
so would create a service escalation for all services that you've
defined in your configuration files::

  define serviceescalation{
    host_name           HOST1
    service_description *
    other escalation directives ...
  }

Multiple Services On Same Host: If you want to create
:ref:`service escalations <obj_def_service_escalation>` for all multiple
services assigned to a particular host, you can use a specify more than
one service description in the service_description directive. The
definition below would create a service escalation for services SERVICE1
through SERVICEN on host HOST1. All the instances of the service
escalation would be identical (i.e. have the same contact groups,
notification interval, etc.)::

  define serviceescalation{
    host_name           HOST1
    service_description SERVICE1,SERVICE2,...,SERVICEN
    other escalation directives ...
  }

.. _obj_def_tricks_service_dependency:

Service Dependency Definitions
==============================

Multiple Hosts: If you want to create
:ref:`service dependencies <obj_def_service_dependency>` for services of
the same name/description that are assigned to multiple hosts, you can
specify multiple hosts in the host_name and or dependent_host_name
directives. In the example below, service SERVICE2 on hosts HOST3 and
HOST4 would be dependent on service SERVICE1 on hosts HOST1 and
HOST2. All the instances of the service dependencies would be identical
except for the host names (i.e. have the same notification failure
criteria, etc.)::

  define servicedependency{
    host_name                     HOST1,HOST2
    service_description           SERVICE1
    dependent_host_name           HOST3,HOST4
    dependent_service_description SERVICE2
    other dependency directives ...
  }

All Services On A Host: If you want to create service dependencies for
all services assigned to a particular host, you can use a wildcard in
the service_description and/or dependent_service_description
directives. In the example below, all services on host HOST2 would be
dependent on all services on host HOST1. All the instances of the
service dependencies would be identical (i.e. have the same notification
failure criteria, etc.)::

  define servicedependency{
    host_name                     HOST1
    service_description           *
    dependent_host_name           HOST2
    dependent_service_description *
    other dependency directives ...
  }

Multiple Services On A Host: If you want to create service dependencies
for multiple services assigned to a particular host, you can specify
more than one service description in the service_description and/or
dependent_service_description directives as follows::

  define servicedependency{
    host_name                     HOST1
    service_description           SERVICE1,SERVICE2,...,SERVICEN
    dependent_host_name           HOST2
    dependent_service_description SERVICE1,SERVICE2,...,SERVICEN
    other dependency directives ...
  }

Same Host Dependencies: If you want to create service dependencies for
multiple services that are dependent on services on the same host, leave
the dependent_host_name directive empty. The example below assumes that
hosts HOST1 and HOST2 have at least the following four services
associated with them: SERVICE1, SERVICE2, SERVICE3, and SERVICE4. In
this example, SERVICE3 and SERVICE4 on HOST1 will be dependent on both
SERVICE1 and SERVICE2 on HOST1. Similiarly, SERVICE3 and SERVICE4 on
HOST2 will be dependent on both SERVICE1 and SERVICE2 on HOST2::

  define servicedependency{
    host_name                     HOST1,HOST2
    service_description           SERVICE1,SERVICE2
    dependent_service_description SERVICE3,SERVICE4
    other dependency directives ...
  }

.. _obj_def_tricks_host_escalation:

Host Escalation Definitions
===========================

Multiple Hosts: If you want to create
:ref:`host escalations <obj_def_host_escalation>` for multiple hosts,
you can specify multiple hosts in the host_name directive. The
definition below would create a host escalation for hosts HOST1 through
HOSTN. All the instances of the host escalation would be identical
(i.e. have the same contact groups, notification interval, etc.)::

  define hostescalation{
    host_name HOST1,HOST2,HOST3,...,HOSTN
    other escalation directives ...
  }

All Hosts: If you want to create identical host escalations for all
hosts that are defined in your configuration files, you can use a
wildcard in the host_name directive. The definition below would create a
hosts escalation for all hosts that are defined in your configuration
files. All the instances of the host escalation would be identical
(i.e. have the same contact groups, notification interval, etc.)::

  define hostescalation{
    host_name *
    other escalation directives ...
  }

.. _obj_def_tricks_host_dependency:

Host Dependency Definitions
===========================

Multiple Hosts: If you want to create
:ref:`host dependencies <obj_def_host_dependency>` for multiple hosts,
you can specify multiple hosts in the host_name and/or
dependent_host_name directives. The definition below would be equivalent
to creating six seperate host dependencies. In the example above, hosts
HOST3, HOST4 and HOST5 would be dependent upon both HOST1 and HOST2. All
the instances of the host dependencies would be identical except for the
host names (i.e. have the same notification failure criteria, etc.)::

  define hostdependency{
    host_name           HOST1,HOST2
    dependent_host_name HOST3,HOST4,HOST5
    other dependency directives ...
  }
