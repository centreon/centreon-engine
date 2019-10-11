=====================
Centreon Engine 19.10
=====================

*********
Bug fixes
*********

Notifications
=============

The notifications mechanism has been totally rewritten. First notification delay
and last recovery notification delay should work as expected.

Pipes in results
================

Pipes are allowed in the centreon-engine plugin return value.

***********
Improvements
***********

Optimizations
=============

Migration of the code to C++11. This implies many changes in data structures.
Some parts have also been rewritten and optimized.

Cmake cleanup
=============

The build directory is gone away. CMake is used as intended, this solves issues
with some ide (like kdevelop)...

Better test coverage
====================

We now have 123 new unit test (+63%). It allow us to have a better code coverage
of the code base.
