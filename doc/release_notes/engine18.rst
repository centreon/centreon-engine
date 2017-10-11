=====================
Centreon Engine 1.8.1
=====================

*********
Bug fixes
*********

Generate environment macros when using large installation tweaks
================================================================

Environment macros were not generated when using large installation
tweaks, in contradiction with the documentation. Only summary macros
are not generated now.

=====================
Centreon Engine 1.8.0
=====================

*********
Bug fixes
*********

Make timezone macros work
=========================

The $HOSTTIMEZONE$, $SERVICETIMEZONE$ and $CONTACTTIMEZONE$ macros were
not included in the generated macro array. As the array size was
increased, these macros are now working.

Print initial states when creating new objects at reload
========================================================

Initial host and service states are not printed when new objects are
created during a configuration reload. This prevented Centreon Web's
reporting dashboard to work properly on such hosts and services until
a process restart occured.
