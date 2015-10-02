#######
Modules
#######

This is a way of extending functionality in Centreon Engine without
having to alter main code. Modules are based on shared code libraries.
Modules are hooked into the Centreon Engine process. Modules uses
callback routines which have to be served by the modules. These routines
are executed when special events occur in the Centreon Engine server
process.

External Command
================

The external commands system was moved on a module. To use it, you need
to add line on Centreon Engine configuration::

    broker_module=/usr/lib/centreon-engine/externalcmd.so
