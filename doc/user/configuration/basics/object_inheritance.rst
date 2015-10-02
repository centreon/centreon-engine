.. _object_inheritance:

Object Inheritance
******************

Introduction
============

This documentation attempts to explain object inheritance and how it can
be used in your :ref:`object definitions <obj_def>`.

If you are confused about how recursion and inheritance work after
reading this, take a look at the sample object config files provided in
the Centreon Engine distribution. If that still doesn't help, drop an
email message with a detailed description of your problem to the
centengine-users mailing list.

Basics
======

There are three variables affecting recursion and inheritance that are
present in all object definitions. They are indicated in red as
follows::

  define someobjecttype{
    object-specific variables ...
    name     template_name
    use      name_of_template_to_use
    register [0/1]
  }

The first variable is name. Its just a "template" name that can be
referenced in other object definitions so they can inherit the objects
properties/variables. Template names must be unique amongst objects of
the same type, so you can't have two or more host definitions that have
"hosttemplate" as their template name.

The second variable is use. This is where you specify the name of the
template object that you want to inherit properties/variables from. The
name you specify for this variable must be defined as another object's
template named (using the name variable).

The third variable is register. This variable is used to indicate
whether or not the object definition should be "registered" with
Centreon Engine. By default, all object definitions are registered. If
you are using a partial object definition as a template, you would want
to prevent it from being registered (an example of this is provided
later). Values are as follows: 0 = do NOT register object definition, 1
= register object definition (this is the default). This variable is NOT
inherited; every (partial) object definition used as a template must
explicitly set the register directive to be 0. This prevents the need to
override an inherited register directive with a value of 1 for every
object that should be registered.

Local Variables vs. Inherited Variables
=======================================

One important thing to understand with inheritance is that "local"
object variables always take precedence over variables defined in the
template object. Take a look at the following example of two host
definitions (not all required variables have been supplied)::

  define host{
    host_name            bighost1
    check_command        check-host-alive
    notification_options d,u,r
    max_check_attempts   5
    name                 hosttemplate1
  }

  define host{
    host_name          bighost2
    max_check_attempts 3
    use                hosttemplate1
  }

You'll note that the definition for host bighost1 has been defined as
having hosttemplate1 as its template name. The definition for host
bighost2 is using the definition of bighost1 as its template
object. Once Centreon Engine processes this data, the resulting
definition of host bighost2 would be equivalent to this definition::

  define host{
    host_name            bighost2
    check_command        check-host-alive
    notification_options d,u,r
    max_check_attempts   3
  }

You can see that the check_command and notification_options variables
were inherited from the template object (where host bighost1 was
defined). However, the host_name and max_check_attempts variables were
not inherited from the template object because they were defined
locally. Remember, locally defined variables override variables that
would normally be inherited from a template object. That should be a
fairly easy concept to understand.

.. note::
   If you would like local string variables to be appended to inherited
   string values, you can do so. Read more about how to accomplish this
   :ref:`below <object_inheritance_additive_inheritance_string_values>`.

Inheritance Chaining
====================

Objects can inherit properties/variables from multiple levels of
template objects. Take the following example::

  define host{
    host_name            bighost1
    check_command        check-host-alive
    notification_options d,u,r
    max_check_attempts   5
    name                 hosttemplate1
  }

  define host{
    host_name          bighost2
    max_check_attempts 3
    use                hosttemplate1
    name               hosttemplate2
  }

  define host{
    host_name bighost3
    use       hosttemplate2
  }

You'll notice that the definition of host bighost3 inherits variables
from the definition of host bighost2, which in turn inherits variables
from the definition of host bighost1. Once Centreon Engine processes
this configuration data, the resulting host definitions are equivalent
to the following::

  define host{
    host_name            bighost1
    check_command        check-host-alive
    notification_options d,u,r
    max_check_attempts   5
  }

  define host{
    host_name            bighost2
    check_command        check-host-alive
    notification_options d,u,r
    max_check_attempts   3
  }

  define host{
    host_name            bighost3
    check_command        check-host-alive
    notification_options d,u,r
    max_check_attempts   3
  }

There is no inherent limit on how "deep" inheritance can go, but you'll
probably want to limit yourself to at most a few levels in order to
maintain sanity.

Using Incomplete Object Definitions as Templates
================================================

It is possible to use incomplete object definitions as templates for use
by other object definitions. By "incomplete" definition, I mean that all
required variables in the object have not been supplied in the object
definition. It may sound odd to use incomplete definitions as templates,
but it is in fact recommended that you use them. Why? Well, they can
serve as a set of defaults for use in all other object definitions. Take
the following example::

  define host{
    check_command        check-host-alive
    notification_options d,u,r
    max_check_attempts   5
    name                 generichosttemplate
    register             0
  }

  define host{
    host_name bighost1
    address   192.168.1.3
    use       generichosttemplate
  }

  define host{
    host_name bighost2
    address   192.168.1.4
    use       generichosttemplate
  }

Notice that the first host definition is incomplete because it is
missing the required host_name variable. We don't need to supply a host
name because we just want to use this definition as a generic host
template. In order to prevent this definition from being registered with
Centreon Engine as a normal host, we set the register variable to 0.

The definitions of hosts bighost1 and bighost2 inherit their values from
the generic host definition. The only variable we've chosed to override
is the address variable. This means that both hosts will have the exact
same properties, except for their host_name and address variables. Once
Centreon Engine processes the config data in the example, the resulting
host definitions would be equivalent to specifying the following::

  define host{
    host_name            bighost1
    address              192.168.1.3
    check_command        check-host-alive
    notification_options d,u,r
    max_check_attempts   5
  }

  define host{
    host_name            bighost2
    address              192.168.1.4
    check_command        check-host-alive
    notification_options d,u,r
    max_check_attempts   5
  }

At the very least, using a template definition for default variables
will save you a lot of typing. It'll also save you a lot of headaches
later if you want to change the default values of variables for a large
number of hosts.

Custom Object
=============

Variables

Any :ref:`custom object variables <custom_object_variables>`
that you define in your host, service, or contact definition templates
will be inherited just like other standard variables. Take the following
example::

  define host{
    _customvar1     somevalue  ; <-- Custom host variable
    _snmp_community public ; <-- Custom host variable
    name            generichosttemplate
    register        0
  }

  define host{
    host_name bighost1
    address   192.168.1.3
    use       generichosttemplate
  }

The host bighost1 will inherit the custom host variables _customvar1 and
_snmp_community, as well as their respective values, from the
generichosttemplate definition. The effective result is a definition for
bighost1 that looks like this::

  define host{
    host_name       bighost1
    address         192.168.1.3
    _customvar1     somevalue
    _snmp_community public
  }

Cancelling Inheritance of String Values
=======================================

In some cases you may not want your host, service, or contact
definitions to inherit values of string variables from the templates
they reference. If this is the case, you can specify "null" (without
quotes) as the value of the variable that you do not want to
inherit. Take the following example::

  define host{
    event_handler my-event-handler-command
    name          generichosttemplate
    register      0
  }

  define host{
    host_name     bighost1
    address       192.168.1.3
    event_handler null
    use           generichosttemplate
  }

In this case, the host bighost1 will not inherit the value of the
event_handler variable that is defined in the generichosttemplate. The
resulting effective definition of bighost1 is the following::

  define host{
    host_name bighost1
    address   192.168.1.3
  }

.. _object_inheritance_additive_inheritance_string_values:

Additive Inheritance of String Values
=====================================

Centreon Engine gives preference to local variables instead of values
inherited from templates. In most cases local variable values override
those that are defined in templates. In some cases it makes sense to
allow Centreon Engine to use the values of inherited and local variables
together.

This "additive inheritance" can be accomplished by prepending the local
variable value with a plus sign (+). This features is only available for
standard (non-custom) variables that contain string values. Take the
following example::

  define host{
    hostgroups all-servers
    name       generichosttemplate
    register   0
  }

  define host{
    host_name  linuxserver1
    hostgroups +linux-servers,web-servers
    use        generichosttemplate
  }

In this case, the host linuxserver1 will append the value of its local
hostgroups variable to that from generichosttemplate. The resulting
effective definition of linuxserver1 is the following::

  define host{
    host_name  linuxserver1
    hostgroups all-servers,linux-servers,web-servers
  }

Implied Inheritance
===================

Normally you have to either explicitly specify the value of a required
variable in an object definition or inherit it from a template. There
are a few exceptions to this rule, where Centreon Engine will assume
that you want to use a value that instead comes from a related
object. For example, the values of some service variables will be copied
from the host the service is associated with if you don't otherwise
specify them.

The following table lists the object variables that will be implicitly
inherited from related objects if you don't explicitly specify their
value in your object definition or inherit them from a template.

=================== ===================== ==========================================================
Object Type         Object Variable       Implied Source
=================== ===================== ==========================================================
Services            contacts              contacts in the associated host definition
Services            contact_groups        contact_groups in the associated host definition
Services            notification_interval notification_interval in the associated host definition
Services            notification_period   notification_period in the associated host definition
Services            timezone              timezone in the associated host definition
Host Escalations    contacts              contacts in the associated host definition
Host Escalations    contact_groups        contact_groups in the associated host definition
Host Escalations    notification_interval notification_interval in the associated host definition
Host Escalations    escalation_period     notification_period in the associated host definition
Service Escalations contacts              contacts in the associated service definition
Service Escalations contact_groups        contact_groups in the associated service definition
Service Escalations notification_interval notification_interval in the associated service definition
Service Escalations escalation_period     notification_period in the associated service definition
=================== ===================== ==========================================================

Implied/Additive Inheritance in Escalations
===========================================

Service and host escalation definitions can make use of a special rule
that combines the features of implied and additive inheritance. If
escalations 1) do not inherit the values of their contact_groups or
contacts directives from another escalation template and 2) their
contact_groups or contacts directives begin with a plus sign (+), then
the values of their corresponding host or service definition's
contact_groups or contacts directives will be used in the additive
inheritance logic.

Here's an example::

  define host{
    name           linux-server
    contact_groups linux-admins
    ...
  }

  define hostescalation{
    host_name      linux-server
    contact_groups +management
    ...
  }

This is a much simpler equivalent to::

  define hostescalation{
    host_name      linux-server
    contact_groups linux-admins,management
    ...
  }

Important values
================

Service templates can make use of a special rule which gives precedence
to their check_command value. If the check_command is prefixed with an
exclamation mark (!), then the template's check_command is marked as
important and will be used over the check_command defined for the
service (this is styled after CSS syntax, which uses ! as an important
attribute).

Why is this useful? It is mainly useful when setting a different
check_command for distributed systems. You may want to set a freshness
threshold and a check_command that forces the service into a failed
state, but this doesn't work with the normal templating system. Using
this important flag allows the custom check_command to be written, but a
general distributed template can be used to overrule the check_command
when used on a central Centreon Engine server.

For instance::

  # On master
  define service{
    name                  service-distributed
    register              0
    active_checks_enabled 0
    check_freshness       1
    check_command         !set_to_stale
  }

  # On slave
  define service{
    name                  service-distributed
    register              0
    active_checks_enabled 1
  }

  # Service definition, used by master and slave
  define service{
    host_name           host1
    service_description serviceA
    check_command       check_http...
    use                 service-distributed
    ...
  }

Multiple Inheritance Sources
============================

Thus far, all examples of inheritance have shown object definitions
inheriting variables/values from just a single source. You are also able
to inherit variables/values from multiple sources for more complex
configurations, as shown below::

  # Generic host template
  define host{
    name                  generic-host
    active_checks_enabled 1
    check_interval        10
    ...
    register              0
  }

  # Development web server template
  define host{
    name                 development-server
    check_interval       15
    notification_options d,u,r
    ...
    register             0
  }

  # Development web server
  define host{
    use       generic-host,development-server
    host_name devweb1
    ...
  }

.. image:: /_static/images/multiple-templates1.png
   :align: center

In the example above, devweb1 is inheriting variables/values from two
sources: generic-host and development-server. You'll notice that a
check_interval variable is defined in both sources. Since generic-host
was the first template specified in devweb1's use directive, its value
for the check_interval variable is inherited by the devweb1 host. After
inheritance, the effective definition of devweb1 would be as follows::

  # Development web server
  define host{
    host_name             devweb1
    active_checks_enabled 1
    check_interval        10
    notification_options  d,u,r
    ...
  }

Precedence With Multiple Inheritance Sources
============================================

When you use multiple inheritance sources, it is important to know how
Centreon Engine handles variables that are defined in multiple
sources. In these cases Centreon Engine will use the variable/value from
the first source that is specified in the use directive. Since
inheritance sources can themselves inherit variables/values from one or
more other sources, it can get tricky to figure out what variable/value
pairs take precedence.

Consider the following host definition that references three templates::

  # Development web server
  define host{
    use       1, 4, 8
    host_name devweb1
    ...
  }

If some of those referenced templates themselves inherit
variables/values from one or more other templates, the precendence rules
are shown to the right.

Testing, trial, and error will help you better understand exactly how
things work in complex inheritance situations like this. :-)

.. image:: /_static/images/multiple-templates2.png
   :align: center
