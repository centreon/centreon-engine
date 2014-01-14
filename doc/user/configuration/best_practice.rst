Best practice
*************

This documentation provide best practice for Centreon-Engine
configuration.

Check options
=============

Disable check options if these options are not necessary.

  * enable_event_handlers=0
  * check_service_freshness=0
  * check_host_freshness=0
  * enable_flap_detection=0
  * obsess_over_services=0
  * obsess_over_hosts=0

Broker modules
==============

Enable external command.

  * broker_module=/path/externalcmd.so

Enable Centreon-Broker.

  * broker_module=/path/cbmod.so

Check the event broker options.

  * event_broker_options=-1

Perfdata
========

With Centreon-Broker the perfdata processing need to disable
beceause this options is not necessary for Centreon-Broker
and make some latency.

  * process_performance_data=0

Tuning
======

Enable cached check to incresed performance.

  * cached_service_check_horizon=20
  * cached_host_check_horizon=20

Optimization
============

Large installation enable some optimization enable it.

  * use_large_installation_tweaks=1

Environment macros have a poor performance disable it.

  * enable_environment_macros=0

Centreon-Engine allow to improve performance without
using setpgid (see the documentation!).

  * use_setpgid=0
