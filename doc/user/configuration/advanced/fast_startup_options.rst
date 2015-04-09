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

Skipping Circular Path Tests
============================

One of the most time-intensive) portion of the configuration startup
phase is the circular path check. For example, it can take nearly a
minute to perform this step of the configuration verification on an
average configuration.

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
