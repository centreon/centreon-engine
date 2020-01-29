.. _standard_macros:

Standard Macros
***************

Macro Validity
==============

Although macros can be used in all commands you define, not all macros
may be "valid" in a particular type of command. For example, some macros
may only be valid during service notification commands, whereas other
may only be valid during host check commands. There are ten types of
commands that Centreon Engine recognizes and treats differently. They
are as follows:

  * Service checks
  * Service notifications
  * Host checks
  * Host notifications
  * Service :ref:`event handlers <event_handlers>` and/or a global
    service event handler
  * Host :ref:`event handlers <event_handlers>` and/or a global host
    event handler
  * :ref:`OCSP <main_cfg_opt_obsessive_compulsive_service_processor_command>`
    command
  * :ref:`OCHP <main_cfg_opt_obsessive_compulsive_host_processor_command>`
    command
  * Service :ref:`performance data <performance_data>` commands
  * Host :ref:`performance data <performance_data>` commands

The tables below list all macros currently available in Centreon Engine,
along with a brief description of each and the types of commands in
which they are valid. If a macro is used in a command in which it is
invalid, it is replaced with an empty string. It should be noted that
macros consist of all uppercase characters and are enclosed in $
characters.

Macro Availability Chart
========================

Legend
------

======= ==========================
No      The macro is not available
**Yes** The macro is available
======= ==========================

.. _user_configuration_macros_host:

Host Macros :sup:`3`
--------------------

============================== ============== ===================== ================ ================== =============================== ============================ ================= ==============
Macro Name                     Service Checks Service Notifications Host Checks      Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
============================== ============== ===================== ================ ================== =============================== ============================ ================= ==============
`HOSTNAME`_                    **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTDISPLAYNAME`_             **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTALIAS`_                   **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTADDRESS`_                 **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTSTATE`_                   **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTSTATEID`_                 **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LASTHOSTSTATE`_               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LASTHOSTSTATEID`_             **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTSTATETYPE`_               **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTATTEMPT`_                 **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`MAXHOSTATTEMPTS`_             **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTEVENTID`_                 **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LASTHOSTEVENTID`_             **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTPROBLEMID`_               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LASTHOSTPROBLEMID`_           **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTLATENCY`_                 **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTEXECUTIONTIME`_           **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTDURATION`_                **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTDURATIONSEC`_             **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTDOWNTIME`_                **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTPERCENTCHANGE`_           **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTGROUPNAME`_               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTGROUPNAMES`_              **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LASTHOSTCHECK`_               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LASTHOSTSTATECHANGE`_         **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LASTHOSTUP`_                  **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LASTHOSTDOWN`_                **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LASTHOSTUNREACHABLE`_         **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTOUTPUT`_                  **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LONGHOSTOUTPUT`_              **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTPERFDATA`_                **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTCHECKCOMMAND`_            **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTACKAUTHOR`_ :sup:`8`      No             No                    No               **Yes**            No                              No                           No                No
`HOSTACKAUTHORNAME`_ :sup:`8`  No             No                    No               **Yes**            No                              No                           No                No
`HOSTACKAUTHORALIAS`_ :sup:`8` No             No                    No               **Yes**            No                              No                           No                No
`HOSTACKCOMMENT`_ :sup:`8`     No             No                    No               **Yes**            No                              No                           No                No
`HOSTACTIONURL`_               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTNOTESURL`_                **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTNOTES`_                   **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTSERVICES`_           **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTSERVICESOK`_         **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTSERVICESWARNING`_    **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTSERVICESUNKNOWN`_    **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTSERVICESCRITICAL`_   **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTID`_                      **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTTIMEZONE`_                **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
============================== ============== ===================== ================ ================== =============================== ============================ ================= ==============

Host Group Macros
-----------------

============================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                     Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
============================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
`HOSTGROUPALIAS`_ :sup:`5`     **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTGROUPMEMBERS`_ :sup:`5`   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTGROUPNOTES`_ :sup:`5`     **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTGROUPNOTESURL`_ :sup:`5`  **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTGROUPACTIONURL`_ :sup:`5` **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
============================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

.. _user_configuration_macros_service:

Service Macros
--------------

================================= ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                        Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
================================= ============== ===================== =========== ================== =============================== ============================ ================= ==============
`SERVICEDESC`_                    **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEDISPLAYNAME`_             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICESTATE`_ :sup:`2`          **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICESTATEID`_ :sup:`2`        **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICESTATE`_               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICESTATEID`_             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICESTATETYPE`_               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEATTEMPT`_                 **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`MAXSERVICEATTEMPTS`_             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEISVOLATILE`_              **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEEVENTID`_                 **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICEEVENTID`_             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEPROBLEMID`_               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICEPROBLEMID`_           **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICELATENCY`_                 **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEEXECUTIONTIME`_ :sup:`2`  **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEDURATION`_                **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEDURATIONSEC`_             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEDOWNTIME`_                **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEPERCENTCHANGE`_           **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEGROUPNAME`_               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEGROUPNAMES`_              **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICECHECK`_               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICESTATECHANGE`_         **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICEOK`_                  **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICEWARNING`_             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICEUNKNOWN`_             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LASTSERVICECRITICAL`_            **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEOUTPUT`_ :sup:`2`         **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`LONGSERVICEOUTPUT`_ :sup:`2`     **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEPERFDATA`_ :sup:`2`       **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICECHECKCOMMAND`_            **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEACKAUTHOR`_ :sup:`8`      No             **Yes**               No          No                 No                              No                           No                No
`SERVICEACKAUTHORNAME`_ :sup:`8`  No             **Yes**               No          No                 No                              No                           No                No
`SERVICEACKAUTHORALIAS`_ :sup:`8` No             **Yes**               No          No                 No                              No                           No                No
`SERVICEACKCOMMENT`_ :sup:`8`     No             **Yes**               No          No                 No                              No                           No                No
`SERVICEACTIONURL`_               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICENOTESURL`_                **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICENOTES`_                   **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICEID`_                      **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
`SERVICETIMEZONE`_                **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
================================= ============== ===================== =========== ================== =============================== ============================ ================= ==============

Service Group Macros
--------------------

================================= ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                        Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
================================= ============== ===================== =========== ================== =============================== ============================ ================= ==============
`SERVICEGROUPALIAS`_ :sup:`6`     **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`SERVICEGROUPMEMBERS`_ :sup:`6`   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`SERVICEGROUPNOTES`_ :sup:`6`     **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`SERVICEGROUPNOTESURL`_ :sup:`6`  **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`SERVICEGROUPACTIONURL`_ :sup:`6` **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
================================= ============== ===================== =========== ================== =============================== ============================ ================= ==============

Contact Macros
--------------

================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name         Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
`CONTACTNAME`_     No             **Yes**               No          **Yes**            No                              No                           No                No
`CONTACTALIAS`_    No             **Yes**               No          **Yes**            No                              No                           No                No
`CONTACTEMAIL`_    No             **Yes**               No          **Yes**            No                              No                           No                No
`CONTACTPAGER`_    No             **Yes**               No          **Yes**            No                              No                           No                No
`CONTACTADDRESSn`_ No             **Yes**               No          **Yes**            No                              No                           No                No
`CONTACTTIMEZONE`_ No             **Yes**               No          **Yes**            No                              No                           No                No
================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Contact Group Macros
--------------------

=============================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                      Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=============================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
`CONTACTGROUPALIAS`_ :sup:`7`   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`CONTACTGROUPMEMBERS`_ :sup:`7` **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
=============================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

.. _macros_summary:

Summary Macros
--------------

=========================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                                  Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=========================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
`TOTALHOSTSUP`_ :sup:`10`                   **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTSDOWN`_ :sup:`10`                 **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTSUNREACHABLE`_ :sup:`10`          **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTSDOWNUNHANDLED`_ :sup:`10`        **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTSUNREACHABLEUNHANDLED`_ :sup:`10` **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTPROBLEMS`_ :sup:`10`              **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALHOSTPROBLEMSUNHANDLED`_               **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALSERVICESOK`_ :sup:`10`                **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALSERVICESWARNING`_ :sup:`10`           **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALSERVICESCRITICAL`_ :sup:`10`          **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALSERVICESUNKNOWN`_ :sup:`10`           **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALSERVICESWARNINGUNHANDLED`_ :sup:`10`  **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALSERVICESCRITICALUNHANDLED`_ :sup:`10` **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALSERVICESUNKNOWNUNHANDLED`_ :sup:`10`  **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALSERVICEPROBLEMS`_ :sup:`10`           **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
`TOTALSERVICEPROBLEMSUNHANDLED`_ :sup:`10`  **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
=========================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

.. _user_configuration_macros_notification:

Notification Macros
-------------------

============================ ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                   Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
============================ ============== ===================== =========== ================== =============================== ============================ ================= ==============
`NOTIFICATIONTYPE`_          No             **Yes**               No          **Yes**            No                              No                           No                No
`NOTIFICATIONRECIPIENTS`_    No             **Yes**               No          **Yes**            No                              No                           No                No
`NOTIFICATIONISESCALATED`_   No             **Yes**               No          **Yes**            No                              No                           No                No
`NOTIFICATIONAUTHOR`_        No             **Yes**               No          **Yes**            No                              No                           No                No
`NOTIFICATIONAUTHORNAME`_    No             **Yes**               No          **Yes**            No                              No                           No                No
`NOTIFICATIONAUTHORALIAS`_   No             **Yes**               No          **Yes**            No                              No                           No                No
`NOTIFICATIONCOMMENT`_       No             **Yes**               No          **Yes**            No                              No                           No                No
`HOSTNOTIFICATIONNUMBER`_    No             **Yes**               No          **Yes**            No                              No                           No                No
`HOSTNOTIFICATIONID`_        No             **Yes**               No          **Yes**            No                              No                           No                No
`SERVICENOTIFICATIONNUMBER`_ No             **Yes**               No          **Yes**            No                              No                           No                No
`SERVICENOTIFICATIONID`_     No             **Yes**               No          **Yes**            No                              No                           No                No
============================ ============== ===================== =========== ================== =============================== ============================ ================= ==============

Date/Time Macros
----------------

========================= ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
========================= ============== ===================== =========== ================== =============================== ============================ ================= ==============
`LONGDATETIME`_           **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`SHORTDATETIME`_          **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`DATE`_                   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`TIME`_                   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`TIMET`_                  **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`ISVALIDTIME`_ :sup:`9`   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`NEXTVALIDTIME`_ :sup:`9` **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
========================= ============== ===================== =========== ================== =============================== ============================ ================= ==============

File Macros
-----------

====================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name             Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
====================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
`MAINCONFIGFILE`_      **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`STATUSDATAFILE`_      **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`COMMENTDATAFILE`_     **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`DOWNTIMEDATAFILE`_    **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`RETENTIONDATAFILE`_   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`OBJECTCACHEFILE`_     **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`TEMPFILE`_            **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`TEMPPATH`_            **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`LOGFILE`_             **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`RESOURCEFILE`_        **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`COMMANDFILE`_         **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`HOSTPERFDATAFILE`_    **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`SERVICEPERFDATAFILE`_ **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
====================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

.. _user_configuration_macros_misc:

Misc Macros
-----------

=================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
`PROCESSSTARTTIME`_ **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`EVENTSTARTTIME`_   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`ADMINEMAIL`_       **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`ADMINPAGER`_       **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`ARGn`_             **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`USERn`_            **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`POLLERNAME`_       **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
`POLLERID`_         **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
=================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Macro Descriptions
==================

Host Macros :sup:`3`
--------------------

============================== =========================================================================================================================
_`HOSTNAME`                    Short name for the host (i.e. "biglinuxbox"). This value is taken from the host_name directive in the
                               :ref:`host definition <obj_def_host>`.
_`HOSTDISPLAYNAME`             An alternate display name for the host. This value is taken from the display_name directive in the
                               :ref:`host definition <obj_def_host>`.
_`HOSTALIAS`                   Long name/description for the host. This value is taken from the alias directive in the
                               :ref:`host definition <obj_def_host>`.
_`HOSTADDRESS`                 Address of the host. This value is taken from the address directive in the
                               :ref:`host definition <obj_def_host>`.
_`HOSTSTATE`                   A string indicating the current state of the host ("UP", "DOWN", or "UNREACHABLE").
_`HOSTSTATEID`                 A number that corresponds to the current state of the host: 0=UP, 1=DOWN, 2=UNREACHABLE.
_`LASTHOSTSTATE`               A string indicating the last state of the host ("UP", "DOWN", or "UNREACHABLE").
_`LASTHOSTSTATEID`             A number that corresponds to the last state of the host: 0=UP, 1=DOWN, 2=UNREACHABLE.
_`HOSTSTATETYPE`               A string indicating the :ref:`state type <state_types>` for the current host check ("HARD" or "SOFT"). Soft states occur
                               when host checks return a non-OK (non-UP) state and are in the process of being retried. Hard states result when host
                               checks have been checked a specified maximum number of times.
_`HOSTATTEMPT`                 The number of the current host check retry. For instance, if this is the second time that the host is being rechecked,
                               this will be the number two. Current attempt number is really only useful when writing host event handlers for "soft"
                               states that take a specific action based on the host retry number.
_`MAXHOSTATTEMPTS`             The max check attempts as defined for the current host. Useful when writing host event handlers for "soft" states that
                               take a specific action based on the host retry number.
_`HOSTEVENTID`                 A globally unique number associated with the host's current state. Every time a host (or service) experiences a state
                               change, a global event ID number is incremented by one (1). If a host has experienced no state changes, this macro will
                               be set to zero (0).
_`LASTHOSTEVENTID`             The previous (globally unique) event number that was given to the host.
_`HOSTPROBLEMID`               A globally unique number associated with the host's current problem state. Every time a host (or service) transitions
                               from an UP or OK state to a problem state, a global problem ID number is incremented by one (1). This macro will be
                               non-zero if the host is currently a non-UP state. State transitions between non-UP states (e.g. DOWN to UNREACHABLE) do
                               not cause this problem id to increase. If the host is currently in an UP state, this macro will be set to zero (0).
                               Combined with event handlers, this macro could be used to automatically open trouble tickets when hosts first enter a
                               problem state.
_`LASTHOSTPROBLEMID`           The previous (globally unique) problem number that was given to the host. Combined with event handlers, this macro could
                               be used for automatically closing trouble tickets, etc. when a host recovers to an UP state.
_`HOSTLATENCY`                 A (floating point) number indicating the number of seconds that a scheduled host check lagged behind its scheduled check
                               time. For instance, if a check was scheduled for 03:14:15 and it didn't get executed until 03:14:17, there would be a
                               check latency of 2.0 seconds. On-demand host checks have a latency of zero seconds.
_`HOSTEXECUTIONTIME`           A (floating point) number indicating the number of seconds that the host check took to execute (i.e. the amount of time
                               the check was executing).
_`HOSTDURATION`                A string indicating the amount of time that the host has spent in its current state. Format is "XXh YYm ZZs", indicating
                               hours, minutes and seconds.
_`HOSTDURATIONSEC`             A number indicating the number of seconds that the host has spent in its current state.
_`HOSTDOWNTIME`                A number indicating the current "downtime depth" for the host. If this host is currently in a period of
                               :ref:`scheduled downtime <scheduled_downtime>`, the value will be greater than zero. If the host is not
                               currently in a period of downtime, this value will be zero.
_`HOSTPERCENTCHANGE`           A (floating point) number indicating the percent state change the host has undergone. Percent state change is used by the
                               :ref:`flap detection <flapping_detection>` algorithm.
_`HOSTGROUPNAME`               The short name of the hostgroup that this host belongs to. This value is taken from the hostgroup_name directive in the
                               :ref:`hostgroup definition <obj_def_hostgroup>`. If the host
                               belongs to more than one hostgroup this macro will contain the name of just one of them.
_`HOSTGROUPNAMES`              A comma separated list of the short names of all the hostgroups that this host belongs to.
_`LASTHOSTCHECK`               This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which a check of the host was
                               last performed.
_`LASTHOSTSTATECHANGE`         This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time the host last changed state.
_`LASTHOSTUP`                  This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the host was last
                               detected as being in an UP state.
_`LASTHOSTDOWN`                This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the host was last
                               detected as being in a DOWN state.
_`LASTHOSTUNREACHABLE`         This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the host was last
                               detected as being in an UNREACHABLE state.
_`HOSTOUTPUT`                  The first line of text output from the last host check (i.e. "Ping OK").
_`LONGHOSTOUTPUT`              The full text output (aside from the first line) from the last host check.
_`HOSTPERFDATA`                This macro contains any :ref:`performance data <performance_data>` that may have been returned by the last host
                               check.
_`HOSTCHECKCOMMAND`            This macro contains the name of the command (along with any arguments passed to it) used to perform the host check.
_`HOSTACKAUTHOR` :sup:`8`      A string containing the name of the user who acknowledged the host problem. This macro is only valid in notifications
                               where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
_`HOSTACKAUTHORNAME` :sup:`8`  A string containing the short name of the contact (if applicable) who acknowledged the host problem. This macro is only
                               valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
_`HOSTACKAUTHORALIAS` :sup:`8` A string containing the alias of the contact (if applicable) who acknowledged the host problem. This macro is only valid
                               in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
_`HOSTACKCOMMENT` :sup:`8`     A string containing the acknowledgement comment that was entered by the user who acknowledged the host problem. This
                               macro is only valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
_`HOSTACTIONURL`               Action URL for the host. This macro may contain other macros (e.g. $HOSTNAME$), which can be useful when you want to pass
                               the host name to a web page.
_`HOSTNOTESURL`                Notes URL for the host. This macro may contain other macros (e.g. $HOSTNAME$), which can be useful when you want to pass
                               the host name to a web page.
_`HOSTNOTES`                   Notes for the host. This macro may contain other macros (e.g. $HOSTNAME$), which can be useful when you want to
                               host-specific status information, etc. in the description.
_`TOTALHOSTSERVICES`           The total number of services associated with the host.
_`TOTALHOSTSERVICESOK`         The total number of services associated with the host that are in an OK state.
_`TOTALHOSTSERVICESWARNING`    The total number of services associated with the host that are in a WARNING state.
_`TOTALHOSTSERVICESUNKNOWN`    The total number of services associated with the host that are in an UNKNOWN state.
_`TOTALHOSTSERVICESCRITICAL`   The total number of services associated with the host that are in a CRITICAL state.
_`HOSTID`                      The id of the host.
============================== =========================================================================================================================

Host Group Macros
-----------------

============================== =========================================================================================================================
_`HOSTGROUPALIAS` :sup:`5`     The long name / alias of either 1) the hostgroup name passed as an on-demand macro argument or 2) the primary hostgroup
                               associated with the current host (if not used in the context of an on-demand macro). This value is taken from the alias
                               directive in the :ref:`hostgroup definition <obj_def_hostgroup>`.
_`HOSTGROUPMEMBERS` :sup:`5`   A comma-separated list of all hosts that belong to either 1) the hostgroup name passed as an on-demand macro argument or
                               2) the primary hostgroup associated with the current host (if not used in the context of an on-demand macro).
_`HOSTGROUPNOTES` :sup:`5`     The notes associated with either 1) the hostgroup name passed as an on-demand macro argument or 2) the primary hostgroup
                               associated with the current host (if not used in the context of an on-demand macro). This value is taken from the notes
                               directive in the :ref:`hostgroup definition <obj_def_hostgroup>`.
_`HOSTGROUPNOTESURL` :sup:`5`  The notes URL associated with either 1) the hostgroup name passed as an on-demand macro argument or 2) the primary
                               hostgroup associated with the current host (if not used in the context of an on-demand macro). This value is taken from
                               the notes_url directive in the :ref:`hostgroup definition <obj_def_hostgroup>`.
_`HOSTGROUPACTIONURL` :sup:`5` The action URL associated with either 1) the hostgroup name passed as an on-demand macro argument or 2) the primary
                               hostgroup associated with the current host (if not used in the context of an on-demand macro). This value is taken from
                               the action_url directive in the :ref:`hostgroup definition <obj_def_hostgroup>`.
============================== =========================================================================================================================

Service Macros
--------------

================================= ======================================================================================================================
_`SERVICEDESC`                    The long name/description of the service (i.e. "Main Website"). This value is taken from the service_description
                                  directive of the :ref:`service definition <obj_def_service>`.
_`SERVICEDISPLAYNAME`             An alternate display name for the service. This value is taken from the display_name directive in the
                                  :ref:`service definition <obj_def_service>`.
_`SERVICESTATE`                   A string indicating the current state of the service ("OK", "WARNING", "UNKNOWN", or "CRITICAL").
_`SERVICESTATEID`                 A number that corresponds to the current state of the service: 0=OK, 1=WARNING, 2=CRITICAL, 3=UNKNOWN.
_`LASTSERVICESTATE`               A string indicating the last state of the service ("OK", "WARNING", "UNKNOWN", or "CRITICAL").
_`LASTSERVICESTATEID`             A number that corresponds to the last state of the service: 0=OK, 1=WARNING, 2=CRITICAL, 3=UNKNOWN.
_`SERVICESTATETYPE`               A string indicating the :ref:`state type <state_types>` for the current service check ("HARD" or "SOFT"). Soft states
                                  occur when service checks return a non-OK state and are in the process of being retried. Hard states result when
                                  service checks have been checked a specified maximum number of times.
_`SERVICEATTEMPT`                 The number of the current service check retry. For instance, if this is the second time that the service is being
                                  rechecked, this will be the number two. Current attempt number is really only useful when writing service event
                                  handlers for "soft" states that take a specific action based on the service retry number.
_`MAXSERVICEATTEMPTS`             The max check attempts as defined for the current service. Useful when writing host event handlers for "soft" states
                                  that take a specific action based on the service retry number.
_`SERVICEISVOLATILE`              Indicates whether the service is marked as being volatile or not: 0 = not volatile, 1 = volatile.
_`SERVICEEVENTID`                 A globally unique number associated with the service's current state. Every time a a service (or host) experiences a
                                  state change, a global event ID number is incremented by one (1). If a service has experienced no state changes, this
                                  macro will be set to zero (0).
_`LASTSERVICEEVENTID`             The previous (globally unique) event number that given to the service.
_`SERVICEPROBLEMID`               A globally unique number associated with the service's current problem state. Every time a service (or host)
                                  transitions from an OK or UP state to a problem state, a global problem ID number is incremented by one (1). This
                                  macro will be non-zero if the service is currently a non-OK state. State transitions between non-OK states (e.g.
                                  WARNING to CRITICAL) do not cause this problem id to increase. If the service is currently in an OK state, this macro
                                  will be set to zero (0). Combined with event handlers, this macro could be used to automatically open trouble tickets
                                  when services first enter a problem state.
_`LASTSERVICEPROBLEMID`           The previous (globally unique) problem number that was given to the service. Combined with event handlers, this macro
                                  could be used for automatically closing trouble tickets, etc. when a service recovers to an OK state.
_`SERVICELATENCY`                 A (floating point) number indicating the number of seconds that a scheduled service check lagged behind its scheduled
                                  check time. For instance, if a check was scheduled for 03:14:15 and it didn't get executed until 03:14:17, there would
                                  be a check latency of 2.0 seconds.
_`SERVICEEXECUTIONTIME`           A (floating point) number indicating the number of seconds that the service check took to execute (i.e. the amount of
                                  time the check was executing).
_`SERVICEDURATION`                A string indicating the amount of time that the service has spent in its current state. Format is "XXh YYm ZZs",
                                  indicating hours, minutes and seconds.
_`SERVICEDURATIONSEC`             A number indicating the number of seconds that the service has spent in its current state.
_`SERVICEDOWNTIME`                A number indicating the current "downtime depth" for the service. If this service is currently in a period of
                                  :ref:`scheduled downtime <scheduled_downtime>`, the value will be greater than zero. If the service is not
                                  currently in a period of downtime, this value will be zero.
_`SERVICEPERCENTCHANGE`           A (floating point) number indicating the percent state change the service has undergone. Percent state change is used
                                  by the :ref:`flap detection <flapping_detection>` algorithm.
_`SERVICEGROUPNAME`               The short name of the servicegroup that this service belongs to. This value is taken from the servicegroup_name
                                  directive in the :ref:`servicegroup <obj_def_servicegroup>`
                                  definition". If the service belongs to more than one servicegroup this macro will contain the name of just one of
                                  them.
_`SERVICEGROUPNAMES`              A comma separated list of the short names of all the servicegroups that this service belongs to.
_`LASTSERVICECHECK`               This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which a check of the
                                  service was last performed.
_`LASTSERVICESTATECHANGE`         This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time the service last changed
                                  state.
_`LASTSERVICEOK`                  This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                  detected as being in an OK state.
_`LASTSERVICEWARNING`             This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                  detected as being in a WARNING state.
_`LASTSERVICEUNKNOWN`             This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                  detected as being in an UNKNOWN state.
_`LASTSERVICECRITICAL`            This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                  detected as being in a CRITICAL state.
_`SERVICEOUTPUT`                  The first line of text output from the last service check (i.e. "Ping OK").
_`LONGSERVICEOUTPUT`              The full text output (aside from the first line) from the last service check.
_`SERVICEPERFDATA`                This macro contains any :ref:`performance data <performance_data>` that may have been returned by the last
                                  service check.
_`SERVICECHECKCOMMAND`            This macro contains the name of the command (along with any arguments passed to it) used to perform the service check.
_`SERVICEACKAUTHOR` :sup:`8`      A string containing the name of the user who acknowledged the service problem. This macro is only valid in
                                  notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
_`SERVICEACKAUTHORNAME` :sup:`8`  A string containing the short name of the contact (if applicable) who acknowledged the service problem. This macro is
                                  only valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
_`SERVICEACKAUTHORALIAS` :sup:`8` A string containing the alias of the contact (if applicable) who acknowledged the service problem. This macro is only
                                  valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
_`SERVICEACKCOMMENT` :sup:`8`     A string containing the acknowledgement comment that was entered by the user who acknowledged the service problem.
                                  This macro is only valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
_`SERVICEACTIONURL`               Action URL for the service. This macro may contain other macros (e.g. $HOSTNAME$ or $SERVICEDESC$), which can be
                                  useful when you want to pass the service name to a web page.
_`SERVICENOTESURL`                Notes URL for the service. This macro may contain other macros (e.g. $HOSTNAME$ or $SERVICEDESC$), which can be
                                  useful when you want to pass the service name to a web page.
_`SERVICENOTES`                   Notes for the service. This macro may contain other macros (e.g. $HOSTNAME$ or $SERVICESTATE$), which can be useful
                                  when you want to service-specific status information, etc. in the description
_`SERVICEID`                      The id of the service.
================================= ======================================================================================================================

Service Group Macros
--------------------

================================= ======================================================================================================================
_`SERVICEGROUPALIAS` :sup:`6`     The long name / alias of either 1) the servicegroup name passed as an on-demand macro argument or 2) the primary
                                  servicegroup associated with the current service (if not used in the context of an on-demand macro). This value is
                                  taken from the alias directive in the :ref:`servicegroup <obj_def_servicegroup>` definition".
_`SERVICEGROUPMEMBERS` :sup:`6`   A comma-separated list of all services that belong to either 1) the servicegroup name passed as an on-demand macro
                                  argument or 2) the primary servicegroup associated with the current service (if not used in the context of an
                                  on-demand macro).
_`SERVICEGROUPNOTES` :sup:`6`     The notes associated with either 1) the servicegroup name passed as an on-demand macro argument or 2) the primary
                                  servicegroup associated with the current service (if not used in the context of an on-demand macro). This value is
                                  taken from the notes directive in the :ref:`servicegroup definition <obj_def_servicegroup>`.
_`SERVICEGROUPNOTESURL` :sup:`6`  The notes URL associated with either 1) the servicegroup name passed as an on-demand macro argument or 2) the primary
                                  servicegroup associated with the current service (if not used in the context of an on-demand macro). This value is
                                  taken from the notes_url directive in the :ref:`servicegroup definition <obj_def_servicegroup>`.
_`SERVICEGROUPACTIONURL` :sup:`6` The action URL associated with either 1) the servicegroup name passed as an on-demand macro argument or 2) the primary
                                  servicegroup associated with the current service (if not used in the context of an on-demand macro). This value is
                                  taken from the action_url directive in the :ref:`servicegroup definition <obj_def_servicegroup>`..
================================= ======================================================================================================================

Contact Macros
--------------

==================== ===================================================================================================================================
_`CONTACTNAME`       Short name for the contact (i.e. "jdoe") that is being notified of a host or service problem. This value is taken from the
                     contact_name directive in the :ref:`contact definition <obj_def_contact>`.
_`CONTACTALIAS`      Long name/description for the contact (i.e. "John Doe") being notified. This value is taken from the alias directive in the
                     :ref:`contact definition <obj_def_contact>`.
_`CONTACTEMAIL`      Email address of the contact being notified. This value is taken from the email directive in the
                     :ref:`contact definition <obj_def_contact>`.
_`CONTACTPAGER`      Pager number/address of the contact being notified. This value is taken from the pager directive in the
                     :ref:`contact definition <obj_def_contact>`.
_`CONTACTADDRESSn`   Address of the contact being notified. Each contact can have six different addresses (in addition to email address and pager
                     number). The macros for these addresses are $CONTACTADDRESS1$ - $CONTACTADDRESS6$. This value is taken from the addressx directive
                     in the :ref:`contact definition <obj_def_contact>`.
_`CONTACTGROUPNAME`  The short name of the contactgroup that this contact is a member of. This value is taken from the contactgroup_name directive in
                     the :ref:`contactgroup definition <obj_def_contactgroup>`. If the contact belongs to more than one contactgroup this macro will
                     contain the name of just one of them.
_`CONTACTGROUPNAMES` A comma separated list of the short names of all the contactgroups that this contact is a member of.
==================== ===================================================================================================================================

Contact Group Macros
--------------------

=============================== =========================================================================================================================
_`CONTACTGROUPALIAS` :sup:`7`   The long name / alias of either 1) the contactgroup name passed as an on-demand macro argument or 2) the primary
                                contactgroup associated with the current contact (if not used in the context of an on-demand macro). This value is taken
                                from the alias directive in the :ref:`contactgroup definition <obj_def_contactgroup>`.
_`CONTACTGROUPMEMBERS` :sup:`7` A comma-separated list of all contacts that belong to either 1) the contactgroup name passed as an on-demand macro
                                argument or 2) the primary contactgroup associated with the current contact (if not used in the context of an
                                on-demand macro).
=============================== =========================================================================================================================

Summary Macros
--------------

================================= =======================================================================================================================
_`TOTALHOSTSUP`                   This macro reflects the total number of hosts that are currently in an UP state.
_`TOTALHOSTSDOWN`                 This macro reflects the total number of hosts that are currently in a DOWN state.
_`TOTALHOSTSUNREACHABLE`          This macro reflects the total number of hosts that are currently in an UNREACHABLE state.
_`TOTALHOSTSDOWNUNHANDLED`        This macro reflects the total number of hosts that are currently in a DOWN state that are not currently being
                                  "handled". Unhandled host problems are those that are not acknowledged, are not currently in scheduled downtime, and
                                  for which checks are currently enabled.
_`TOTALHOSTSUNREACHABLEUNHANDLED` This macro reflects the total number of hosts that are currently in an UNREACHABLE state that are not currently being
                                  "handled". Unhandled host problems are those that are not acknowledged, are not currently in scheduled downtime, and
                                  for which checks are currently enabled.
_`TOTALHOSTPROBLEMS`              This macro reflects the total number of hosts that are currently either in a DOWN or an UNREACHABLE state.
_`TOTALHOSTPROBLEMSUNHANDLED`     This macro reflects the total number of hosts that are currently either in a DOWN or an UNREACHABLE state that are not
                                  currently being "handled". Unhandled host problems are those that are not acknowledged, are not currently in scheduled
                                  downtime, and for which checks are currently enabled.
_`TOTALSERVICESOK`                This macro reflects the total number of services that are currently in an OK state.
_`TOTALSERVICESWARNING`           This macro reflects the total number of services that are currently in a WARNING state.
_`TOTALSERVICESCRITICAL`          This macro reflects the total number of services that are currently in a CRITICAL state.
_`TOTALSERVICESUNKNOWN`           This macro reflects the total number of services that are currently in an UNKNOWN state.
_`TOTALSERVICESWARNINGUNHANDLED`  This macro reflects the total number of services that are currently in a WARNING state that are not currently being
                                  "handled". Unhandled services problems are those that are not acknowledged, are not currently in scheduled downtime,
                                  and for which checks are currently enabled.
_`TOTALSERVICESCRITICALUNHANDLED` This macro reflects the total number of services that are currently in a CRITICAL state that are not currently being
                                  "handled". Unhandled services problems are those that are not acknowledged, are not currently in scheduled downtime,
                                  and for which checks are currently enabled.
_`TOTALSERVICESUNKNOWNUNHANDLED`  This macro reflects the total number of services that are currently in an UNKNOWN state that are not currently being
                                  "handled". Unhandled services problems are those that are not acknowledged, are not currently in scheduled downtime,
                                  and for which checks are currently enabled.
_`TOTALSERVICEPROBLEMS`           This macro reflects the total number of services that are currently either in a WARNING, CRITICAL, or UNKNOWN state.
_`TOTALSERVICEPROBLEMSUNHANDLED`  This macro reflects the total number of services that are currently either in a WARNING, CRITICAL, or UNKNOWN state
                                  that are not currently being "handled". Unhandled services problems are those that are not acknowledged, are not
                                  currently in scheduled downtime, and for which checks are currently enabled.
================================= =======================================================================================================================

Notification Macros
-------------------

============================ ============================================================================================================================
_`NOTIFICATIONTYPE`          A string identifying the type of notification that is being sent ("PROBLEM", "RECOVERY", "ACKNOWLEDGEMENT", "FLAPPINGSTART",
                             "FLAPPINGSTOP", "FLAPPINGDISABLED", "DOWNTIMESTART", "DOWNTIMEEND", or "DOWNTIMECANCELLED").
_`NOTIFICATIONRECIPIENTS`    A comma-separated list of the short names of all contacts that are being notified about the host or service.
_`NOTIFICATIONISESCALATED`   An integer indicating whether this was sent to normal contacts for the host or service or if it was escalated. 0 = Normal
                             (non-escalated) notification , 1 = Escalated notification.
_`NOTIFICATIONAUTHOR`        A string containing the name of the user who authored the notification. If the $NOTIFICATIONTYPE$ macro is set to
                             "DOWNTIMESTART" or "DOWNTIMEEND", this will be the name of the user who scheduled downtime for the host or service. If the
                             $NOTIFICATIONTYPE$ macro is "ACKNOWLEDGEMENT", this will be the name of the user who acknowledged the host or service
                             problem. If the $NOTIFICATIONTYPE$ macro is "CUSTOM", this will be name of the user who initated the custom host or service
                             notification.
_`NOTIFICATIONAUTHORNAME`    A string containing the short name of the contact (if applicable) specified in the $NOTIFICATIONAUTHOR$ macro.
_`NOTIFICATIONAUTHORALIAS`   A string containing the alias of the contact (if applicable) specified in the $NOTIFICATIONAUTHOR$ macro.
_`NOTIFICATIONCOMMENT`       A string containing the comment that was entered by the notification author. If the $NOTIFICATIONTYPE$ macro is set to
                             "DOWNTIMESTART" or "DOWNTIMEEND", this will be the comment entered by the user who scheduled downtime for the host or
                             service. If the $NOTIFICATIONTYPE$ macro is "ACKNOWLEDGEMENT", this will be the comment entered by the user who acknowledged
                             the host or service problem. If the $NOTIFICATIONTYPE$ macro is "CUSTOM", this will be comment entered by the user who
                             initated the custom host or service notification.
_`HOSTNOTIFICATIONNUMBER`    The current notification number for the host. The notification number increases by one (1) each time a new notification is
                             sent out for the host (except for acknowledgements). The notification number is reset to 0 when the host recovers (after the
                             recovery notification has gone out). Acknowledgements do not cause the notification number to increase, nor do notifications
                             dealing with flap detection or scheduled downtime.
_`HOSTNOTIFICATIONID`        A unique number identifying a host notification. Notification ID numbers are unique across both hosts and service
                             notifications, so you could potentially use this unique number as a primary key in a notification database. Notification ID
                             numbers should remain unique across restarts of the Centreon Engine process, so long as you have state retention enabled. The
                             notification ID number is incremented by one (1) each time a new host notification is sent out, and regardless of how many
                             contacts are notified.
_`SERVICENOTIFICATIONNUMBER` The current notification number for the service. The notification number increases by one (1) each time a new notification
                             is sent out for the service (except for acknowledgements). The notification number is reset to 0 when the service recovers
                             (after the recovery notification has gone out). Acknowledgements do not cause the notification number to increase, nor do
                             notifications dealing with flap detection or scheduled downtime.
_`SERVICENOTIFICATIONID`     A unique number identifying a service notification. Notification ID numbers are unique across both hosts and service
                             notifications, so you could potentially use this unique number as a primary key in a notification database. Notification ID
                             numbers should remain unique across restarts of the Centreon Engine process, so long as you have state retention enabled.
                             The notification ID number is incremented by one (1) each time a new service notification is sent out, and regardless of how
                             many contacts are notified.
============================ ============================================================================================================================

Date/Time Macros
----------------

========================= ===============================================================================================================================
_`LONGDATETIME`           Current date/time stamp (i.e. Fri Oct 13 00:30:28 CDT 2000). Format of date is determined by
                          :ref:`date_format <main_cfg_opt_date_format>` directive.
_`SHORTDATETIME`          Current date/time stamp (i.e. 10-13-2000 00:30:28). Format of date is determined by
                          :ref:`date_format <main_cfg_opt_date_format>` directive.
_`DATE`                   Date stamp (i.e. 10-13-2000). Format of date is determined by :ref:`date_format <main_cfg_opt_date_format>` directive.
_`TIME`                   Current time stamp (i.e. 00:30:28).
_`TIMET`                  Current time stamp in time_t format (seconds since the UNIX epoch).
_`ISVALIDTIME` :sup:`9`   This is a special on-demand macro that returns a 1 or 0 depending on whether or not a particular time is valid within a
                          specified timeperiod. There are two ways of using this macro:

                            * $ISVALIDTIME:24x7$ will be set to "1" if the current time is valid within the "24x7" timeperiod. If not, it will be set to
                              "0".
                            * $ISVALIDTIME:24x7:timestamp$ will be set to "1" if the time specified by the "timestamp" argument (which must be in time_t
                              format) is valid within the "24x7" timeperiod. If not, it will be set to "0".
_`NEXTVALIDTIME` :sup:`9` This is a special on-demand macro that returns the next valid time (in time_t format) for a specified timeperiod. There are two
                          ways of using this macro:

                            * $NEXTVALIDTIME:24x7$ will return the next valid time from and including the current time in the "24x7" timeperiod.
                            * $NEXTVALIDTIME:24x7:timestamp$ will return the next valid time from and including the time specified by the "timestamp"
                              argument (which must be specified in time_t format) in the "24x7" timeperiod.If a next valid time cannot be found in the
                              specified timeperiod, the macro will be set to "0".
========================= ===============================================================================================================================

File Macros
-----------

====================== ==================================================================================================================================
_`MAINCONFIGFILE`      The location of the :ref:`main config file <main_cfg_opt>`.
_`STATUSDATAFILE`      The location of the :ref:`status data file <main_cfg_opt_status_file>`.
_`COMMENTDATAFILE`     The location of the comment data file.
_`DOWNTIMEDATAFILE`    The location of the downtime data file.
_`RETENTIONDATAFILE`   The location of the :ref:`retention data file <main_cfg_opt_state_retention_file>`.
_`OBJECTCACHEFILE`     The location of the :ref:`object cache file <main_cfg_opt_object_cache_file>`.
_`TEMPFILE`            The location of the :ref:`temp file <main_cfg_opt_temp_file>`.
_`TEMPPATH`            The directory specified by the temp path variable.
_`LOGFILE`             The location of the :ref:`log file <main_cfg_opt_log_file>`.
_`RESOURCEFILE`        The location of the :ref:`resource file <main_cfg_opt_resource_file>`.
_`COMMANDFILE`         The location of the :ref:`command file <main_cfg_opt_external_command_file>`.
_`HOSTPERFDATAFILE`    The location of the host performance data file (if defined).
_`SERVICEPERFDATAFILE` The location of the service performance data file (if defined).
====================== ==================================================================================================================================

Misc Macros
-----------

=================== =====================================================================================================================================
_`PROCESSSTARTTIME` Time stamp in time_t format (seconds since the UNIX epoch) indicating when the Centreon Engine process was last (re)started. You can
                    determine the number of seconds that Centreon Engine has been running (since it was last restarted) by subtracting $PROCESSSTARTTIME$
                    from `TIMET`_.
_`EVENTSTARTTIME`   Time stamp in time_t format (seconds since the UNIX epoch) indicating when the Centreon Engine process starting process events
                    (checks, etc.). You can determine the number of seconds that it took for Centreon Engine to startup by subtracting $PROCESSSTARTTIME$
                    from $EVENTSTARTTIME$.
_`ADMINEMAIL`       Global administrative email address. This value is taken from the :ref:`admin_email <main_cfg_opt_administrator_email_address>`.
                    directive.
_`ADMINPAGER`       Global administrative pager number/address. This value is taken from the :ref:`admin_pager <main_cfg_opt_administrator_pager>`
                    directive.
_`ARGn`             The nth argument passed to the command (notification, event handler, service check, etc.). Centreon Engine supports up to 32 argument
                    macros ($ARG1$ through $ARG32$).
_`USERn`            The nth user-definable macro. User macros can be defined in one or more :ref:`resource files <main_cfg_opt_resource_file>`.
                    Centreon Engine supports up to 256 user macros ($USER1$ through $USER256$).
_`POLLERNAME`       The poller_name macro. Retrieve the name of the poller (poller_name field in engine config file).
_`POLLERID`         The poller_id macro. Retrieve the id of the poller (poller_id field in engine config file).
=================== =====================================================================================================================================

Notes
=====

  * :sup:`1` These macros are not valid for the host they are
    associated with when that host is being checked (i.e. they make no
    sense, as they haven't been determined yet).
  * :sup:`2` These macros are not valid for the service they are
    associated with when that service is being checked (i.e. they make
    no sense, as they haven't been determined yet).
  * :sup:`3` When host macros are used in service-related commands
    (i.e. service notifications, event handlers, etc) they refer to they
    host that they service is associated with.
  * :sup:`4` When host and service summary macros are used in
    notification commands, the totals are filtered to reflect only those
    hosts and services for which the contact is authorized (i.e. hosts
    and services they are configured to receive notifications for).
  * :sup:`5` These macros are normally associated with the
    first/primary hostgroup associated with the current host. They could
    therefore be considered host macros in many cases. However, these
    macros are not available as on-demand host macros. Instead, they can
    be used as on-demand hostgroup macros when you pass the name of a
    hostgroup to the macro. For example: $HOSTGROUPMEMBERS:hg1$ would
    return a comma-delimited list of all (host) members of the hostgroup
    hg1.
  * :sup:`6` These macros are normally associated with the
    first/primary servicegroup associated with the current service. They
    could therefore be considered service macros in many cases. However,
    these macros are not available as on-demand service macros. Instead,
    they can be used as on-demand servicegroup macros when you pass the
    name of a servicegroup to the macro. For example:
    $SERVICEGROUPMEMBERS:sg1$ would return a comma-delimited list of all
    (service) members of the servicegroup sg1.
  * :sup:`7` These macros are normally associated with the
    first/primary contactgroup associated with the current contact. They
    could therefore be considered contact macros in many cases. However,
    these macros are not available as on-demand contact macros. Instead,
    they can be used as on-demand contactgroup macros when you pass the
    name of a contactgroup to the macro. For example:
    $CONTACTGROUPMEMBERS:cg1$ would return a comma-delimited list of all
    (contact) members of the contactgroup cg1.
  * :sup:`8` These acknowledgement macros are deprecated. Use the
    more generic $NOTIFICATIONAUTHOR$, $NOTIFICATIONAUTHORNAME$,
    $NOTIFICATIONAUTHORALIAS$ or $NOTIFICATIONAUTHORCOMMENT$ macros
    instead.
  * :sup:`9` These macro are only available as on-demand macros -
    e.g. you must supply an additional argument with them in order to
    use them. These macros are not available as environment variables.
  * :sup:`10` Summary macros are not available as environment
    variables if the
    :ref:`use_large_installation_tweaks <main_cfg_opt_large_installation_tweaks>`
    option is enabled, as they are quite CPU-intensive to calculate.
