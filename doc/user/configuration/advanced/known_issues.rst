Known Issues
************

Timeperiods
===========

Exclusions and Host/Service Checks - There is a bug in the service/host
check scheduling logic that rears its head when you use timeperiod
definitions that use the exclude directive. The problem occurs when
Centreon Engine Core tries to re-schedule the next check. In this case,
the scheduling logic may incorrectly schedule the next check further out
in the future than it should. In essence, it skips over the (missing)
logic where it could determine an earlier possible time using the
exception times. Imperfect Solution: Don't use timeperiod definitions
that exclude other timeperods for your host/service check periods. A fix
is being worked on, and will hopefully make it into a 3.4.x release.

