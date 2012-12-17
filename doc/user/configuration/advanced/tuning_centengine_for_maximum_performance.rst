.. _tuning_centengine_for_maximum_performance:

Tuning Centreon Engine For Maximum Performance
**********************************************

Introduction
============

So you've finally got Centreon Engine up and running and you want to
know how you can tweak it a bit. Tuning Centreon Engine to increase
performance can be necessary when you start monitoring a large number (>
1,000) of hosts and services. Here are a few things to look at for
optimizing Centreon Engine...

Optimization Tips
=================

  * Use large installation tweaks. Enabling the
    :ref:`use_large_installation_tweaks <main_cfg_opt_large_installation_tweaks>`
    option may provide you with better performance. Read more about what
    this option does :ref:`here <main_cfg_opt_large_installation_tweaks>`.
  * Disable environment macros. Macros are normally made available to
    check, notification, event handler, etc. commands as environment
    variables. This can be a problem in a large Centreon Engine
    installation, as it consumes some additional memory and (more
    importantly) more CPU. If your scripts don't need to access the
    macros as environment variables (e.g. you pass all necessary macros
    on the command line), you don't need this feature. You can prevent
    macros from being made available as environment variables by using
    the :ref:`enable_environment_macros <main_cfg_opt_environment_macros>`
    option.
  * Check Result Reaper Frequency. The
    :ref:`check_result_reaper_frequency <main_cfg_opt_check_result_reaper_frequency>`
    variable determines how often Centreon Engine should check for host
    and service check results that need to be processed. The maximum
    amount of time it can spend processing those results is determined
    by the max reaper time (see below). If your reaper frequency is too
    high (too infrequent), you might see high latencies for host and
    service checks.
  * Max Reaper Time. The :ref:`max_check_result_reaper_time <main_cfg_opt_maximum_check_result_reaper_time>`
    variables determines the maximum amount of time the Centreon Engine
    daemon can spend processing the results of host and service checks
    before moving on to other things - like executing new host and
    service checks. Too high of a value can result in large latencies
    for your host and service checks. Too low of a value can have the
    same effect. If you're experiencing high latencies, adjust this
    variable and see what effect it has.
  * Adjust buffer slots. You may need to adjust the value of the
    :ref:`external_command_buffer_slots <main_cfg_opt_external_command_buffer_slots>`
    option.
  * Check service latencies to determine best value for maximum
    concurrent checks. Centreon Engine can restrict the number of
    maximum concurrently executing service checks to the value you
    specify with the :ref:`max_concurrent_checks <main_cfg_opt_maximum_concurrent_service_checks>`
    option. This is good because it gives you some control over how much
    load Centreon Engine will impose on your monitoring host, but it can
    also slow things down. If you are seeing high latency values (> 10
    or 15 seconds) for the majority of your service checks, you are
    probably starving Centreon Engine of the checks it needs. That's not
    Centreon Engine's fault - its yours. Under ideal conditions, all
    service checks would have a latency of 0, meaning they were executed
    at the exact time that they were scheduled to be executed. However,
    it is normal for some checks to have small latency values. I would
    recommend taking the minimum number of maximum concurrent checks
    reported when running Centreon Engine with the -s command line
    argument and doubling it. Keep increasing it until the average check
    latency for your services is fairly low. More information on service
    check scheduling can be found :ref:`here <scheduling_service_and_host>`.
  * Use passive checks when possible. The overhead needed to process the
    results of :ref:`passive service checks <passive_checks>` is
    much lower than that of "normal" active checks, so make use of that
    piece of info if you're monitoring a slew of services. It should be
    noted that passive service checks are only really useful if you have
    some external application doing some type of monitoring or
    reporting, so if you're having Centreon Engine do all the work, this
    won't help things.
  * Avoid using interpreted plugins. One thing that will significantly
    reduce the load on your monitoring host is the use of compiled
    (C/C++, etc.) plugins rather than interpreted script (Perl, etc)
    plugins. While Perl scripts and such are easy to write and work
    well, the fact that they are compiled/interpreted at every execution
    instance can significantly increase the load on your monitoring host
    if you have a lot of service checks. If you want to use Perl
    plugins, consider compiling them into true executables using
    perlcc(1) (a utility which is part of the standard Perl
    distribution).
  * Optimize host check commands. If you're checking host states using
    the check_ping plugin you'll find that host checks will be performed
    much faster if you break up the checks. Instead of specifying a
    max_attempts value of 1 in the host definition and having the
    check_ping plugin send 10 ICMP packets to the host, it would be much
    faster to set the max_attempts value to 10 and only send out 1 ICMP
    packet each time. This is due to the fact that Centreon Engine can
    often determine the status of a host after executing the plugin
    once, so you want to make the first check as fast as possible. This
    method does have its pitfalls in some situations (i.e. hosts that
    are slow to respond may be assumed to be down), but you'll see
    faster host checks if you use it. Another option would be to use a
    faster plugin (i.e. check_fping) as the host_check_command instead
    of check_ping.
  * Schedule regular host checks. Scheduling regular checks of hosts can
    actually help performance in Centreon Engine. This is due to the way
    the :ref:`cached check logic <cached_checks>` works (see
    below). Prior to Centreon Engine 3, regularly scheduled host checks
    used to result in a big performance hit. This is no longer the case,
    as host checks are run in parallel - just like service checks. To
    schedule regular checks of a host, set the check_interval directive
    in the :ref:`host definition <obj_def_host>`
    to something greater than 0.
  * Enable cached host checks. Beginning in Centreon Engine 1, on-demand
    host checks can benefit from caching. On-demand host checks are
    performed whenever Centreon Engine detects a service state
    change. These on-demand checks are executed because Centreon Engine
    wants to know if the host associated with the service changed
    state. By enabling cached host checks, you can optimize
    performance. In some cases, Centreon Engine may be able to used the
    old/cached state of the host, rather than actually executing a host
    check command. This can speed things up and reduce load on
    monitoring server. In order for cached checks to be effective, you
    need to schedule regular checks of your hosts (see above). More
    information on cached checks can be found
    :ref:`here <cached_checks>`.
  * Don't use agressive host checking. Unless you're having problems
    with Centreon Engine recognizing host recoveries, I would recommend
    not enabling the
    :ref:`use_aggressive_host_checking <main_cfg_opt_aggressive_host_checking>`
    option. With this option turned off host checks will execute much
    faster, resulting in speedier processing of service check
    results. However, host recoveries can be missed under certain
    circumstances when this it turned off. For example, if a host
    recovers and all of the services associated with that host stay in
    non-OK states (and don't "wobble" between different non-OK states),
    Centreon Engine may miss the fact that the host has recovered. A few
    people may need to enable this option, but the majority don't and I
    would recommendnot using it unless you find it necessary...
  * External command optimizations. If you're processing a lot of
    external commands (i.e. passive checks in a
    :ref:`distributed setup <distributed_monitoring>`, you'll probably
    want to set the
    :ref:`command_check_interval <main_cfg_opt_external_command_check_interval>`
    variable to -1. This will cause Centreon Engine to check for
    external commands as often as possible. You should also consider
    increasing the number of available
    :ref:`external command buffer slots <main_cfg_opt_external_command_buffer_slots>`.
    Buffers slots are used to hold external commands that have been read
    from the
    :ref:`external command file <main_cfg_opt_external_command_file>`
    (by a separate thread) before they are processed by the Centreon
    Engine daemon. If your Centreon Engine daemon is receiving a lot of
    passive checks or external commands, you could end up in a situation
    where the buffers are always full. This results in child processes
    (external scripts, NSCA daemon, etc.) blocking when they attempt to
    write to the external command file.
  * Disable use setpgid. When you enable setpgid, we force Centreon
    Engine to use low performance create process. See
    :ref:`this <main_cfg_opt_use_setpgid>` documentation.
  * Optimize hardware for maximum performance.

    .. note::
       Hardware performance shouldn't be an issue unless: 1) you're
       monitoring thousands of services, 2) you're doing a lot of
       post-processing of performance data, etc. Your system
       configuration and your hardware setup are going to directly
       affect how your operating system performs, so they'll affect how
       Centreon Engine performs. The most common hardware optimization
       you can make is with your hard drives. CPU and memory speed are
       obviously factors that affect performance, but disk access is
       going to be your biggest bottleneck. Don't store plugins, the
       status log, etc on slow drives (i.e. old IDE drives or NFS
       mounts). If you've got them, use UltraSCSI drives or fast IDE
       drives. An important note for IDE/Linux users is that many Linux
       installations do not attempt to optimize disk access. If you
       don't change the disk access parameters (by using a utility like
       hdparam), you'll loose out on a lot of the speedy features of the
       new IDE drives.
