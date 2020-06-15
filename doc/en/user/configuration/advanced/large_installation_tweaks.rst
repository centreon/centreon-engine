Large Installation Tweaks
*************************

Introduction
============

Users with large Centreon Engine installations may benefit from the
:ref:`use_large_installation_tweaks <main_cfg_opt_large_installation_tweaks>`
configuration option. Enabling this option allows the Centreon Engine
daemon to take certain shortcuts which result in lower system load and
better performance.

Effects
=======

When you enable the
:ref:`use_large_installation_tweaks <main_cfg_opt_large_installation_tweaks>`
option in your main Centreon Engine config file, several changes are
made to the way the Centreon Engine daemon operates:

  * No Summary Macros In Environment Variables - The :ref:`summary
    macros <macros_summary>` will not be available to you as environment
    variables. Calculating the values of these macros can be quite
    time-intensive in large configurations, so they are not available as
    environment variables when use this option. Summary macros will
    still be available as regular macros if you pass them to to your
    scripts as arguments.
  * Different Memory Cleanup - Normally Centreon Engine will free all
    allocated memory in child processes before they exit. This is
    probably best practice, but is likely unnecessary in most
    installations, as most OSes will take care of freeing allocated
    memory when processes exit. The OS tends to free allocated memory
    faster than can be done within Centreon Engine itself, so Centreon
    Engine won't attempt to free memory in child processes if you enable
    this option.
  * Checks fork() Less - Normally Centreon Engine will fork() twice when
    it executes host and service checks. This is done to (1) ensure a
    high level of resistance against plugins that go awry and segfault
    and (2) make the OS deal with cleaning up the grandchild process
    once it exits. The extra fork() is not really necessary, so it is
    skipped when you enable this option. As a result, Centreon Engine
    will itself clean up child processes that exit (instead of leaving
    that job to the OS). This feature should result in significant load
    savings on your Centreon Engine installation.

