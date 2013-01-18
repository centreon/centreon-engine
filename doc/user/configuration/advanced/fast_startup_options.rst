.. _fast_startup_options:

Fast Startup Options
********************

Introduction
============

There are a few things you can do that can decrease the amount of time
it take Centreon Engine to startup (or restart). These speedups involve
easing some of the burden involved in processing your configuration
files.

Using these techniques is particularly useful when you have one or more
of the following:

  * Large configurations
  * Complex configurations (heavy use of template features)
  * Installations where frequest restarts are necessary

Background
==========

Whenever Centreon Engine starts/restarts it has to process your
configuration files before it can get down to the business of
monitoring. This configuration startup process involves a number of
steps:

  * Reading the config files
  * Resolving template definitions
  * "Recombobulating" your objects (my term for the various types of
    work that occurs)
  * Duplicating object definitions
  * Inheriting object properties
  * Sorting your object definitions
  * Verifying object relationship integrity
  * Checking for circular paths
  * and more...

Some of these steps can be quite time-consuming when you have large or
complex configurations. Is there a way to speed any of these steps up?
Yes!

Evaluating Startup Times
========================

Before we get on to making things faster, we need to see what's possible
and whether or not we should even bother with the whole thing. This is
easy to do - simply start centengine with the -s command line switch to
get timing and scheduling information.

An example of the output (abbreviated to only show relevant portions) is
shown below. For this example, I'm using a Centreon Engine config that
has 25 hosts defined and just over 10,000 services::

  $ /usr/sbin/centengine -s /etc/centreon-engine/centengine.cfg
  Timing information on object configuration processing is listed
  below. You can use this information to see if precaching your
  object configuration would be useful.

  Object Config Source: Config files (uncached)

  OBJECT CONFIG PROCESSING TIMES (* = Potential for precache savings
  with -u option)

  ----------------------------------

  Read: 0.486780 sec
  Resolve: 0.004106 sec *
  Recomb Contactgroups: 0.000077 sec *
  Recomb Hostgroups: 0.000172 sec *
  Dup Services: 0.028801 sec *
  Recomb Servicegroups: 0.010358 sec *
  Duplicate: 5.666932 sec *
  Inherit: 0.003770 sec *
  Recomb Contacts: 0.030085 sec *
  Sort: 2.648863 sec *
  Register: 2.654628 sec
  Free: 0.021347 sec

  ============

  TOTAL: 11.555925 sec * = 8.393170 sec (72.63%) estimated savings
  Timing information on configuration verification is listed below.
  CONFIG VERIFICATION TIMES (* = Potential for speedup with -x
  option)

  ----------------------------------

  Object Relationships: 1.400807 sec
  Circular Paths: 54.676622 sec *
  Misc: 0.006924 sec

  ============

  TOTAL: 56.084353 sec * = 54.676622 sec (97.5%) estimated savings

Okay, lets see what happened. Looking at the totals, it took roughly
11.6 seconds to process the configuration files and another 56 seconds
to verify the config. That means that every time I start or restart
Centreon Engine with this configuration, it will take nearly 68 seconds
of startup work before it can monitor anything! That's not acceptable if
I have to restart Centreon Engine on a semi-regular basis.

What can I do about this? Take another look at the output and you'll see
that Centreon Engine estimates that I could save about 8.4 seconds off
the configuration processing time and another 54.7 off the verification
times. In total, Centreon Engine thinks I could save 63 seconds of the
normal startup time if some optimizations were taken.

Whoa! From 68 seconds to just 5 seconds? Yep, read on for how to do it.

Pre-Caching Object Configuration
================================

Centreon Engine can spend quite a bit of time parsing your config files,
especially if you make use of the template features such as inheritance,
etc. In order to reduce the time it takes to parse your config, you can
have Centreon Engine pre-process and pre-cache your config files for
future use.

When you run Centreon Engine with the -p command line option, Centreon
Engine will read your config files in, process them, and save them to a
pre-cached object config file (specified by the
:ref:`precached_object_file <main_cfg_opt_precached_object_file>`
directive). This pre-cached config file will contain pre-processed
configuration entries that are easier/faster for Centreon Engine to
process in the future.

You must use the -p command line option along with either the -v or -s
command line options, as shown below. This ensures that your
configuration is verified before the precached file is created::

  $ /usr/sbin/centengine -pv /etc/centreon-engine/centengine.cfg

The size of your precached config file will most likely be significantly
larger than the sum of the sizes of your object config files. This is
normal and by design.

.. image:: /_static/images/fast-startup1.png
   :align: center

Once the precached object configuration file have been created, you can
start Centreon Engine and tell it to use the precached config file
instead of your object config file(s) by using the -u command line
option::

  $ /usr/sbin/centengine -ud /etc/centreon-engine/centengine.cfg

.. note::
   If you modify your configuration files, you will need to re-verify
   and re-cache your configuration files before restarting Centreon
   Engine. If you don't re-generate the precached object file, Centreon
   Engine will continue to use your old configuration because it is now
   reading from the precached file, rather than your source
   configuration files.

.. image:: /_static/images/fast-startup2.png
   :align: center

Skipping Circular Path Tests
============================

The second (and most time-intensive) portion of the configuration
startup phase is the circular path check. In the example above, it took
nearly a minute to perform this step of the configuration verification.

What is the circular path check and why does it take so long? The
circular patch check is designed to ensure that you don't define any
circular paths in your host, host dependency, or service dependency
definitions. If a circular path existed in your config files, Centreon
Engine could end up in a deadlock situation. The most likely reason for
the check taking so long is that I'm not using an efficient algorithm. A
much more efficient algorithm for detecting circular paths would be most
welcomed. Hint: That means all you CompSci graduate students who have
been emailing me about doing your thesis on Centreon Engine can
contribute some code back. :-)

If you want to skip the circular path check when Centreon Engine starts,
you can add the -x command line option like this::

  $ /usr/sbin/centengine -xd /etc/centreon-engine/centengine.cfg

.. note::
   It is of utmost importance that you verify your configuration before
   starting/restarting Centreon Engine when skipping circular path
   checks. Failure to do so could lead to deadlocks in the Centreon
   Engine logic. You have been warned.

Putting It All Together
=======================

Follow these steps if you want to make use of potential speedups from
pre-caching your configuration and skipping circular path checks.

1. Verify your configuration and create the precache file with the
   following command::

     $ /usr/sbin/centengine -vp /etc/centreon-engine/centengine.cfg

2. Stop Centreon Engine if it is currently running.

3. Start Centreon Engine like so to use the precached config file and
   skip circular path checks::

     $ /usr/sbin/centengine -uxd /etc/centreon-engine/centengine.cfg

4. When you modify your original configuration files in the future and
   need to restart Centreon Engine to make those changes take place,
   repeat step 1 to re-verify your config and regenerate your cached
   config file. Once that is done you can restart Centreon Engine
   through the web interface or by sending a SIGHUP signal. If you don't
   re-generate the precached object file, Centreon Engine will continue
   to use your old confguration because it is now reading from the
   precached file, rather than your source configuration files.

5. That's it! Enjoy the increased startup speed.

