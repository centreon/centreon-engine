=====================
Centreon Engine 1.7.0
=====================

**********
What's new
**********

Rewritten timeperiods
=====================

The timeperiod code was heavily modified to be more robust and handle
more edge cases such as DST changes or timeperiods that span between
two years. Unit tests ensure that these edge cases work properly.
