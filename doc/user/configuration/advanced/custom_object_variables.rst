.. _custom_object_variables:

Custom Object Variables
***********************

Introduction
============

Users often request that new variables be added to host and service
definitions. These include variables for SNMP community, MAC address,
etc. The list is endless. The problem that I see with doing this is
that it makes Centreon Engine less generic and more
infrastructure-specific. Centreon Engine was intended to be flexible,
which meant things needed to be designed in a generic manner. Host
definitions in Centreon Engine, for example, have a generic "address"
variable that can contain anything from an IP address to
human-readable driving directions - whatever is appropriate for the
user's setup.

Still, there needs to be a method for admins to store information about
their infrastructure components in their Centreon Engine configuration
without imposing a set of specific variables on others. Centreon Engine
attempts to solve this problem by allowing users to define custom
variables in their object definitions. Custom variables allow users to
define additional properties in their host and service definitions, and
use their values in event handlers and host and service checks.

Custom Variable Basics
======================

There are a few important things that you should note about custom
variables:

  * Custom variable names must begin with an underscore (_) to prevent
    name collision with standard variables
  * Custom variable names are converted to all uppercase before use
  * Custom variables are :ref:`inherited <object_inheritance>`
    from object templates like normal variables
  * Scripts can reference custom variable values with
    :ref:`macros and environment variables <understanding_macros>`

Examples
========

Here's an example of how custom variables can be defined in different
types of object definitions::

  define host{
    host_name      linuxserver
    _mac_address 00:06:5B:A6:AD:AA ; <-- Custom MAC_ADDRESS variable
    _rack_number R32               ; <-- Custom RACK_NUMBER variable
    ...
  }

  define service{
    host_name       linuxserver
    description     Memory Usage
    _SNMP_community public   ; <-- Custom SNMP_COMMUNITY variable
    _TechContact    Jane Doe ; <-- Custom TECHCONTACT variable
    ....
  }


Custom Variables As Macros
==========================

Custom variable values can be referenced in scripts and executables that
Centreon Engine runs for checks, event handlers, etc. by using
:ref:`macros <understanding_macros>` or
environment variables. Custom variable macros are trusted (because you
define them) and therefore not cleaned/sanitized before they are made
available to scripts.

In order to prevent name collision among custom variables from different
object types, Centreon Engine prepends "_HOST" or "_SERVICE" to the
beginning of custom host or service variables, respectively, in macro
names. The table below shows the corresponding macro names for the custom
variables that were defined in the example above.

=========== ============== ========================
Object Type Variable Name  Macro Name
=========== ============== ========================
Host        MAC_ADDRESS    $_HOSTMAC_ADDRESS$
Host        RACK_NUMBER    $_HOSTRACK_NUMBER$
Service     SNMP_COMMUNITY $_SERVICESNMP_COMMUNITY$
Service     TECHCONTACT    $_SERVICETECHCONTACT$
=========== ============== ========================

Custom Variables And Inheritance
================================

Custom object variables are :ref:`inherited <object_inheritance>`
just like standard host or service.
