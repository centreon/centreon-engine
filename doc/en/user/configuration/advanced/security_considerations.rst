Security Considerations
***********************

Introduction
============

.. image:: /_static/images/security.png
   :align: center

This is intended to be a brief overview of some things you should keep
in mind when installing Centreon Engine, so as set it up in a secure
manner.

Your monitoring box should be viewed as a backdoor into your other
systems. In many cases, the Centreon Engine server might be allowed
access through firewalls in order to monitor remote servers. In most all
cases, it is allowed to query those remote servers for various
information. Monitoring servers are always given a certain level of
trust in order to query remote systems. This presents a potential
attacker with an attractive backdoor to your systems. An attacker might
have an easier time getting into your other systems if they compromise
the monitoring server first. This is particularly true if you are making
use of shared SSH keys in order to monitor remote systems.

If an intruder has the ability to submit check results or external
commands to the Centreon Engine daemon, they have the potential to
submit bogus monitoring data, drive you nuts you with bogus
notifications, or cause event handler scripts to be triggered. If you
have event handler scripts that restart services, cycle power, etc. this
could be particularly problematic.

Another area of concern is the ability for intruders to sniff monitoring
data (status information) as it comes across the wire. If communication
channels are not encrypted, attackers can gain valuable information by
watching your monitoring information. Take as an example the following
situation: An attacker captures monitoring data on the wire over a
period of time and analyzes the typical CPU and disk load usage of your
systems, along with the number of users that are typically logged into
them. The attacker is then able to determine the best time to compromise
a system and use its resources (CPU, etc.) without being noticed.

Here are some tips to help ensure that you keep your systems secure when
implementing a Centreon Engine-based monitoring solution...

Best Practices
==============

Use
---

I would recommend that you install Centreon Engine on a server that is
dedicated to monitoring (and possibly other admin tasks). Protect your
monitoring server as if it were one of the most important servers on
your network. Keep running services to a minimum and lock down access to
it via TCP wrappers, firewalls, etc. Since the Centreon Engine server is
allowed to talk to your servers and may be able to poke through your
firewalls, allowing users access to your monitoring server can be a
security risk. Remember, its always easier to gain root access through a
system security hole if you have a local account on a box.

.. image:: /_static/images/security3.png
   :align: center

Don't Run Centreon Engine As Root
---------------------------------

Centreon Centreon Engine doesn't need to run as root, so don't do it. If
you need to execute event handlers or plugins which require root access,
you might want to try using `sudo <http://www.courtesan.com/sudo/sudo>`_.

Down The Check Result Directory
-------------------------------

Make sure that only the centengine user is able to read/write in the
check result path. If users other than centengine (or root) are able to
write to this directory, they could send fake host/service check results
to the Centreon Engine daemon. This could result in annoyances (bogus
notifications) or security problems (event handlers being kicked off).

Lock Down The External Command File
-----------------------------------

If you enable :ref:`external commands <external_commands>`, make sure
you set proper permissions on the ``/var/log/centreon-engine/rw``
directory. You only want the Centreon Engine user (usually centengine)
and the web server user (usually nobody, httpd, apache2, or www-data) to
have permissions to write to the command file. If you've installed
Centreon Engine on a machine that is dedicated to monitoring and admin
tasks and is not used for public accounts, that should be fine. If
you've installed it on a public or multi-user machine (not recommended),
allowing the web server user to have write access to the command file
can be a security problem. After all, you don't want just any user on
your system controlling Centreon Engine through the external command
file.

Use Full Paths In Command Definitions
-------------------------------------

When you define commands, make sure you specify the full path (not a
relative one) to any scripts or binaries you're executing.

Strip Dangerous Characters From Macros
--------------------------------------

Use the
:ref:`illegal_macro_output_chars <main_cfg_opt_illegal_macro_output_characters>`
directive to strip dangerous characters from the $HOSTOUTPUT$,
$SERVICEOUTPUT$, $HOSTPERFDATA$, and $SERVICEPERFDATA$ macros before
they're used in notifications, etc. Dangerous characters can be anything
that might be interpreted by the shell, thereby opening a security
hole. An example of this is the presence of backtick (`) characters in
the $HOSTOUTPUT$, $SERVICEOUTPUT$, $HOSTPERFDATA$, and/or
$SERVICEPERFDATA$ macros, which could allow an attacker to execute an
arbitrary command as the centengine user (one good reason not to run
Centreon Engine as the root user).

Secure Access to Remote Agents
------------------------------

Make sure you lock down access to agents (NRPE, NSClient, SNMP, etc.) on
remote systems using firewalls, access lists, etc. You don't want
everyone to be able to query your systems for status information. This
information could be used by an attacker to execute remote event handler
scripts or to determine the best times to go unnoticed.

.. image:: /_static/images/security1.png
   :align: center

Secure Communication Channels
-----------------------------

Make sure you encrypt communication channels between different Centreon
Engine installations and between your Centreon Engine servers and your
monitoring agents whenever possible. You don't want someone to be able
to sniff status information going across your network. This information
could be used by an attacker to determine the best times to go
unnoticed.

.. image:: /_static/images/security2.png
   :align: center
