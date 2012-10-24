.. _distributed_monitoring:

Distributed Monitoring
**********************

Introduction
============

Centreon Engine can be configured to support distributed monitoring of
network services and resources. I'll try to briefly explan how this can
be accomplished...

Goals
=====

The goal in the distributed monitoring environment that I will describe
is to offload the overhead (CPU usage, etc.) of performing service
checks from a "central" server onto one or more "distributed"
servers. Most small to medium sized shops will not have a real need for
setting up such an environment. However, when you want to start
monitoring hundreds or even thousands of hosts (and several times that
many services) using Centreon Engine, this becomes quite important.

Reference Diagram
=================

The diagram below should help give you a general idea of how distributed
monitoring works with Centreon Engine. I'll be referring to the items
shown in the diagram as I explain things...

Central Server vs. Distributed Servers
======================================

When setting up a distributed monitoring environment with Centreon
Engine, there are differences in the way the central and distributed
servers are configured. I'll show you how to configure both types of
servers and explain what effects the changes being made have on the
overall monitoring. For starters, lets describe the purpose of the
different types of servers...

The function of a distributed server is to actively perform checks all
the services you define for a "cluster" of hosts. I use the term
"cluster" loosely - it basically just mean an arbitrary group of hosts
on your network. Depending on your network layout, you may have several
cluters at one physical location, or each cluster may be separated by a
WAN, its own firewall, etc. The important thing to remember to that for
each cluster of hosts (however you define that), there is one
distributed server that runs Centreon Engine and monitors the services
on the hosts in the cluster. A distributed server is usually a
bare-bones installation of Centreon Engine. It doesn't have to have the
web interface installed, send out notifications, run event handler
scripts, or do anything other than execute service checks if you don't
want it to. More detailed information on configuring a distributed
server comes later...

The purpose of the central server is to simply listen for service check
results from one or more distributed servers. Even though services are
occassionally actively checked from the central server, the active
checks are only performed in dire circumstances, so lets just say that
the central server only accepts passive check for now. Since the central
server is obtaining :ref:`passive service check <passive_checks>`
results from one or more distributed servers, it serves as the focal
point for all monitoring logic (i.e. it sends out notifications, runs
event handler scripts, determines host states, has the web interface
installed, etc).

Obtaining Service Check Information From Distributed Monitors
=============================================================

Okay, before we go jumping into configuration detail we need to know how
to send the service check results from the distributed servers to the
central server. I've already discussed how to submit passive check
results to Centreon Engine from same host that Centreon Engine is
running on (as described in the documentation on
:ref:`passive checks <passive_checks>`), but I haven't given any
info on how to submit passive check results from other hosts.

In order to facilitate the submission of passive check results to a
remote host, I've written the :ref:`nsca addon <addons_nsca>`. The addon
consists of two pieces. The first is a client program (send_nsca) which
is run from a remote host and is used to send the service check results
to another server. The second piece is the nsca daemon (nsca) which
either runs as a standalone daemon or under inetd and listens for
connections from client programs. Upon receiving service check
information from a client, the daemon will sumbit the check information
to Centreon Engine (on the central server) by inserting a
PROCESS_SVC_CHECK_RESULT command into the
:ref:`external command file <main_cfg_opt_external_command_file>`,
along with the check results. The next time Centreon Engine checks for
:ref:`external commands <external_commands>`, it will find the passive
service check information that was sent from the distributed server and
process it. Easy, huh?

Distributed Server Configuration
================================

So how exactly is Centreon Engine configured on a distributed server?
Basically, its just a bare-bones installation. You don't need to install
the web interface or have notifications sent out from the server, as
this will all be handled by the central server.

Key configuration changes:

  * Only those services and hosts which are being monitored directly by
    the distributed server are defined in the
    :ref:`object configuration file <object_configuration_overview>`.
  * The distributed server has its
    :ref:`enable_notifications <main_cfg_opt_notifications>`
    directive set to 0. This will prevent any notifications from being
    sent out by the server.
  * The distributed server is configured to
    :ref:`obsess over services <main_cfg_opt_obsess_over_services>`.
  * The distributed server has an
    :ref:`ocsp command <main_cfg_opt_obsessive_compulsive_service_processor_command>`
    defined (as described below).

In order to make everything come together and work properly, we want the
distributed server to report the results of all service checks to
Centreon Engine. We could use :ref:`event handlers <event_handlers>` to
report changes in the state of a service, but that just doesn't cut
it. In order to force the distributed server to report all service check
results, you must enabled the
:ref:`obsess_over_services <main_cfg_opt_obsess_over_services>`
option in the main configuration file and provide a
:ref:`ocsp_command <main_cfg_opt_obsessive_compulsive_service_processor_command>`
to be run after every service check. We will use the ocsp command to
send the results of all service checks to the central server, making use
of the send_nsca client and nsca daemon (as described above) to handle
the tranmission.

In order to accomplish this, you'll need to define an ocsp command like
this::

  ocsp_command=submit_check_result

The command definition for the submit_check_result command looks
something like this::

  define command{
    command_name submit_check_result
    command_line /usr/lib/nagios/plugins/event_handlers/submit_check_result $HOSTNAME$ '$SERVICEDESC$' $SERVICESTATE$ '$SERVICEOUTPUT$'
  }

The submit_check_result shell scripts looks something like this (replace
central_server with the IP address of the central server)::

  #!/bin/sh
  # Arguments:
  # $1 = host_name (Short name of host that the service is
  # associated with)
  # $2 = svc_description (Description of the service)
  # $3 = state_string (A string representing the status of
  # the given service - "OK", "WARNING", "CRITICAL"
  # or "UNKNOWN")
  # $4 = plugin_output (A text string that should be used
  # as the plugin output for the service checks)

  # Convert the state string to the corresponding return code
  return_code=-1

  case "$3" in
    OK)
      return_code=0
      ;;
    WARNING)
      return_code=1
      ;;
    CRITICAL)
      return_code=2
      ;;
    UNKNOWN)
      return_code=-1
      ;;
  esac

  # pipe the service check info into the send_nsca program, which
  # in turn transmits the data to the nsca daemon on the central
  # monitoring server
  /bin/printf "%s\t%s\t%s\t%s\n" "$1" "$2" "$return_code" "$4" | /usr/local/nsca/bin/send_nsca -H central_server -c /etc/centreon-engine/send_nsca.cfg

The script above assumes that you have the send_nsca program and it
configuration file (``send_nsca.cfg``) located in the
``/usr/local/nsca/bin/`` and ``/etc/centreon-engine/`` directories,
respectively.

That's it! We've sucessfully configured a remote host running Centreon
Engine to act as a distributed monitoring server. Let's go over exactly
what happens with the distributed server and how it sends service check
results to Centreon Engine (the steps outlined below correspond to the
numbers in the reference diagram above):

  * After the distributed server finishes executing a service check, it
    executes the command you defined by the
    :ref:`ocsp_command <main_cfg_opt_obsessive_compulsive_service_processor_command>`
    variable. In our example, this is the
    ``/usr/lib/nagios/plugins/event_handlers/submit_check_result``
    script. Note that the definition for the submit_check_result command
    passed four pieces of information to the script: the name of the
    host the service is associated with, the service description, the
    return code from the service check, and the plugin output from the
    service check.
  * The submit_check_result script pipes the service check information
    (host name, description, return code, and output) to the send_nsca
    client program.
  * The send_nsca program transmits the service check information to the
    nsca daemon on the central monitoring server.
  * The nsca daemon on the central server takes the service check
    information and writes it to the external command file for later
    pickup by Centreon Engine.
  * The Centreon Engine process on the central server reads the external
    command file and processes the passive service check information
    that originated from the distributed monitoring server.

Central Server Configuration
============================

We've looked at how distributed monitoring servers should be configured,
so let's turn to the central server. For all intensive purposes, the
central is configured as you would normally configure a standalone
server. It is setup as follows:

  * The central server has the web interface installed (optional, but
    recommended)
  * The central server has its :ref:`enable_notifications <main_cfg_opt_notifications>`
    directive set to 1. This will enable notifications. (optional, but
    recommended)
  * The central server has :ref:`active service checks <main_cfg_opt_service_check_execution>`
    disabled (optional, but recommended - see notes below)
  * The central server has :ref:`external command checks <main_cfg_opt_external_command_check>`
    enabled (required)
  * The central server has :ref:`passive service checks <main_cfg_opt_passive_service_check_acceptance>`
    enabled (required) There are three other very important things that
    you need to keep in mind when configuring the central server:
  * The central server must have service definitions for all services
    that are being monitored by all the distributed servers. Centreon
    Engine will ignore passive check results if they do not correspond
    to a service that has been defined.
  * If you're only using the central server to process services whose
    results are going to be provided by distributed hosts, you can
    simply disable all active service checks on a program-wide basis by
    setting the :ref:`execute_service_checks <main_cfg_opt_service_check_execution>`
    directive to 0. If you're using the central server to actively
    monitor a few services on its own (without the aid of distributed
    servers), the enable_active_checks option of the defintions for
    service being monitored by distributed servers should be set
    to 0. This will prevent Centreon Engine from actively checking those
    services.

It is important that you either disable all service checks on a
program-wide basis or disable the enable_active_checks option in the
definitions for each service that is monitored by a distributed
server. This will ensure that active service checks are never executed
under normal circumstances. The services will keep getting rescheduled
at their normal check intervals (3 minutes, 5 minutes, etc...), but the
won't actually be executed. This rescheduling loop will just continue
all the while Centreon Engine is running. I'll explain why this is done
in a bit...

That's it! Easy, huh?

Problems With Passive Checks
============================

For all intensive purposes we can say that the central server is relying
solely on passive checks for monitoring. The main problem with relying
completely on passive checks for monitoring is the fact that Centreon
Engine must rely on something else to provide the monitoring data. What
if the remote host that is sending in passive check results goes down or
becomes unreachable? If Centreon Engine isn't actively checking the
services on the host, how will it know that there is a problem?

Fortunately, there is a way we can handle these types of problems...

Freshness Checking
==================

Centreon Engine supports a feature that does "freshness" checking on the
results of service checks. More information freshness checking can be
found :ref:`here <freshness_checks>`. This features gives some
protection against situations where remote hosts may stop sending
passive service checks into the central monitoring server. The purpose
of "freshness" checking is to ensure that service checks are either
being provided passively by distributed servers on a regular basis or
performed actively by the central server if the need arises. If the
service check results provided by the distributed servers get "stale",
Centreon Engine can be configured to force active checks of the service
from the central monitoring host.

So how do you do this? On the central monitoring server you need to
configure services that are being monitoring by distributed servers as
follows...

  * The check_freshness option in the service definitions should be set
    to 1. This enables "freshness" checking for the services.
  * The freshness_threshold option in the service definitions should be
    set to a value (in seconds) which reflects how "fresh" the results
    for the services (provided by the distributed servers) should be.
  * The check_command option in the service definitions should reflect
    valid commands that can be used to actively check the service from
    the central monitoring server.

Centreon Engine periodically checks the "freshness" of the results for
all services that have freshness checking enabled. The
freshness_threshold option in each service definition is used to
determine how "fresh" the results for each service should be. For
example, if you set this value to 300 for one of your services, Centreon
Engine will consider the service results to be "stale" if they're older
than 5 minutes (300 seconds). If you do not specify a value for the
freshness_threshold option, Centreon Engine will automatically calculate
a "freshness" threshold by looking at either the normal_check_interval
or retry_check_interval options (depending on what
:ref:`type of state <state_types>` the service is in). If the
service results are found to be "stale", Centreon Engine will run the
service check command specified by the check_command option in the
service definition, thereby actively checking the service.

Remember that you have to specify a check_command option in the service
definitions that can be used to actively check the status of the service
from the central monitoring server. Under normal circumstances, this
check command is never executed (because active checks were disabled on
a program-wide basis or for the specific services). When freshness
checking is enabled, Centreon Engine will run this command to actively
check the status of the service even if active checks are disabled on a
program-wide or service-specific basis.

If you are unable to define commands to actively check a service from
the central monitoring host (or if turns out to be a major pain), you
could simply define all your services with the check_command option set
to run a dummy script that returns a critical status. Here's an
example... Let's assume you define a command called 'service-is-stale'
and use that command name in the check_command option of your
services. Here's what the definition would look like::

  define command{
    command_name service-is-stale
    command_line /usr/lib/nagios/plugins/check_dummy 2 "CRITICAL: Service results are stale"
  }

When Centreon Engine detects that the service results are stale and runs
the service-is-stale command, the check_dummy plugin is executed and the
service will go into a critical state. This would likely cause
notifications to be sent out, so you'll know that there's a problem.

Performing Host Checks
======================

At this point you know how to obtain service check results passivly from
distributed servers. This means that the central server is not actively
checking services on its own. But what about host checks? You still need
to do them, so how?

Since host checks usually compromise a small part of monitoring activity
(they aren't done unless absolutely necessary), I'd recommend that you
perform host checks actively from the central server. That means that
you define host checks on the central server the same way that you do on
the distributed servers (and the same way you would in a normal,
non-distributed setup).

Passive host checks are available (read
:ref:`here <passive_checks>`), so you could use them in your
distributed monitoring setup, but they suffer from a few problems. The
biggest problem is that Centreon Engine does not translate passive host
check problem states (DOWN and UNREACHABLE) when they are
processed. This means that if your monitoring servers have a different
parent/child host structure (and they will, if you monitoring servers
are in different locations), the central monitoring server will have an
inaccurate view of host states.

If you do want to send passive host checks to a central server in your
distributed monitoring setup, make sure:

  * The central server has
    :ref:`passive host checks <main_cfg_opt_passive_host_check_acceptance>`
    enabled (required)
  * The distributed server is configured to
    :ref:`obsess over hosts <main_cfg_opt_obsess_over_hosts>`.
  * The distributed server has an
    :ref:`ochp command <main_cfg_opt_obsessive_compulsive_host_processor_command>`
    defined.

The ochp command, which is used for processing host check results, works
in a similiar manner to the ocsp command, which is used for processing
service check results (see documentation above). In order to make sure
passive host check results are up to date, you'll want to enable
:ref:`freshness checking <freshness_checks>` for hosts
(similiar to what is described above for services).

