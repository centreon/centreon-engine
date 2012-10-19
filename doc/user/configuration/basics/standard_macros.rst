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
  * Service :ref:`event handlers <advanced_event_handlers>` and/or a
    global service event handler
  * Host :ref:`event handlers <advanced_event_handlers>` and/or a global
    host event handler
  * :ref:`OCSP <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsiveserviceprocessorcommand>`
    command
  * :ref:`OCHP <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobsessivecompulsivehostprocessorcommand>`
    command
  * Service :ref:`performance data <advanced_performance_data>` commands
  * Host :ref:`performance data <advanced_performance_data>` commands

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

Host Macros :sup:`3`
------------------------

=================================== ============== ===================== ================ ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks      Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== ================ ================== =============================== ============================ ================= ==============
:ref:`HOSTNAME                      **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstname>`
:ref:`HOSTDISPLAYNAME               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstdisplayname>`
:ref:`HOSTALIAS                     **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstalias>`
:ref:`HOSTADDRESS                   **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstaddress>`
:ref:`HOSTSTATE                     **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hststate>`
:ref:`HOSTSTATEID                   **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hststateid>`
:ref:`LASTHOSTSTATE                 **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_lasthststate>`
:ref:`LASTHOSTSTATEID               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_lasthststateid>`
:ref:`HOSTSTATETYPE                 **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hststatetype>`
:ref:`HOSTATTEMPT                   **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstattempt>`
:ref:`MAXHOSTATTEMPTS               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_maxhstattempts>`
:ref:`HOSTEVENTID                   **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hsteventid>`
:ref:`LASTHOSTEVENTID               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_lasthsteventid>`
:ref:`HOSTPROBLEMID                 **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstpbid>`
:ref:`LASTHOSTPROBLEMID             **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_lasthstpbid>`
:ref:`HOSTLATENCY                   **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstlatency>`
:ref:`HOSTEXECUTIONTIME             **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstexecutiontime>`
:ref:`HOSTDURATION                  **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstduration>`
:ref:`HOSTDURATIONSEC               **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstdurationsec>`
:ref:`HOSTDOWNTIME                  **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstdowntime>`
:ref:`HOSTPERCENTCHANGE             **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstpercentchange>`
:ref:`HOSTGROUPNAME                 **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hgname>`
:ref:`HOSTGROUPNAMES                **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hgnames>`
:ref:`LASTHOSTCHECK                 **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_lasthstcheck>`
:ref:`LASTHOSTSTATECHANGE           **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_lasthststatechange>`
:ref:`LASTHOSTUP                    **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_lasthstup>`
:ref:`LASTHOSTDOWN                  **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_lasthstdown>`
:ref:`LASTHOSTUNREACHABLE           **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_lasthstunreachable>`
:ref:`HOSTOUTPUT                    **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstoutput>`
:ref:`LONGHOSTOUTPUT                **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_longhstoutput>`
:ref:`HOSTPERFDATA                  **Yes**        **Yes**               **Yes** :sup:`1` **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstperfdata>`
:ref:`HOSTCHECKCOMMAND              **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstcheckcommand>`
:ref:`HOSTACKAUTHOR                 No             No                    No               **Yes**            No                              No                           No                No
<macro_hstackauthor>` :sup:`8`
:ref:`HOSTACKAUTHORNAME             No             No                    No               **Yes**            No                              No                           No                No
<macro_hstackauthorname>` :sup:`8`
:ref:`HOSTACKAUTHORALIAS            No             No                    No               **Yes**            No                              No                           No                No
<macro_hstackauthoralias>` :sup:`8`
:ref:`HOSTACKCOMMENT                No             No                    No               **Yes**            No                              No                           No                No
<macro_hstackcomment>` :sup:`8`
:ref:`HOSTACTIONURL                 **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstactionurl>`
:ref:`HOSTNOTESURL                  **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstnotesurl>`
:ref:`HOSTNOTES                     **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstnotes>`
:ref:`TOTALHOSTSERVICES             **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsvc>`
:ref:`TOTALHOSTSERVICESOK           **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsvcok>`
:ref:`TOTALHOSTSERVICESWARNING      **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsvcwarning>`
:ref:`TOTALHOSTSERVICESUNKNOWN      **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsvcunknown>`
:ref:`TOTALHOSTSERVICESCRITICAL     **Yes**        **Yes**               **Yes**          **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsvccritical>`
=================================== ============== ===================== ================ ================== =============================== ============================ ================= ==============

Host Group Macros
-----------------

=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`HOSTGROUPALIAS                **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hgalias>` :sup:`5`
:ref:`HOSTGROUPMEMBERS              **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hgmembers>` :sup:`5`
:ref:`HOSTGROUPNOTES                **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hgnotes>` :sup:`5`
:ref:`HOSTGROUPNOTESURL             **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hgnoteurl>` :sup:`5`
:ref:`HOSTGROUPACTIONURL            **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hgactionurl>` :sup:`5`
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Service Macros
--------------

=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`SERVICEDESC <macro_svcdesc>`  **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
:ref:`SERVICEDISPLAYNAME            **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcdisplayname>`
:ref:`SERVICESTATE                  **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcstate>` :sup:`2`
:ref:`SERVICESTATEID                **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcstateid>` :sup:`2`
:ref:`LASTSERVICESTATE              **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvcstate>`
:ref:`LASTSERVICESTATEID            **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvcstateid>`
:ref:`SERVICESTATETYPE              **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcstatetype>`
:ref:`SERVICEATTEMPT                **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcattempt>`
:ref:`MAXSERVICEATTEMPTS            **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_maxsvcattempts>`
:ref:`SERVICEISVOLATILE             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcisvolatile>`
:ref:`SERVICEEVENTID                **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svceventid>`
:ref:`LASTSERVICEEVENTID            **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvceventid>`
:ref:`SERVICEPROBLEMID              **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcpbid>`
:ref:`LASTSERVICEPROBLEMID          **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvcpbid>`
:ref:`SERVICELATENCY                **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svclatency>`
:ref:`SERVICEEXECUTIONTIME          **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcexecutiontime>` :sup:`2`
:ref:`SERVICEDURATION               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcduration>`
:ref:`SERVICEDURATIONSEC            **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcdurationsec>`
:ref:`SERVICEDOWNTIME               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcdowntime>`
:ref:`SERVICEPERCENTCHANGE          **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcpercentchange>`
:ref:`SERVICEGROUPNAME              **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_sgname>`
:ref:`SERVICEGROUPNAMES             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_sgnames>`
:ref:`LASTSERVICECHECK              **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvccheck>`
:ref:`LASTSERVICESTATECHANGE        **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvcstatechange>`
:ref:`LASTSERVICEOK                 **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvcok>`
:ref:`LASTSERVICEWARNING            **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvcwarning>`
:ref:`LASTSERVICEUNKNOWN            **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvcunknown>`
:ref:`LASTSERVICECRITICAL           **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_lastsvccritical>`
:ref:`SERVICEOUTPUT                 **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcoutput>` :sup:`2`
:ref:`LONGSERVICEOUTPUT             **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_longsvcoutput>` :sup:`2`
:ref:`SERVICEPERFDATA               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcperfdata>` :sup:`2`
:ref:`SERVICECHECKCOMMAND           **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svccheckcommand>`
:ref:`SERVICEACKAUTHOR              No             **Yes**               No          No                 No                              No                           No                No
<macro_svcackauthor>` :sup:`8`
:ref:`SERVICEACKAUTHORNAME          No             **Yes**               No          No                 No                              No                           No                No
<macro_svcackauthorname>` :sup:`8`
:ref:`SERVICEACKAUTHORALIAS         No             **Yes**               No          No                 No                              No                           No                No
<macro_svcackauthoralias>` :sup:`8`
:ref:`SERVICEACKCOMMENT             No             **Yes**               No          No                 No                              No                           No                No
<macro_svcackcomment>` :sup:`8`
:ref:`SERVICEACTIONURL              **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcactionurl>`
:ref:`SERVICENOTESURL               **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcnotesurl>`
:ref:`SERVICENOTES                  **Yes**        **Yes**               No          No                 **Yes**                         No                           **Yes**           No
<macro_svcnotes>`
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Service Group Macros
--------------------

=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`SERVICEGROUPALIAS             **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_sgalias>` :sup:`6`
:ref:`SERVICEGROUPMEMBERS           **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_sgmembers>` :sup:`6`
:ref:`SERVICEGROUPNOTES             **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_sgnotes>` :sup:`6`
:ref:`SERVICEGROUPNOTESURL          **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_sgnoteurl>` :sup:`6`
:ref:`SERVICEGROUPACTIONURL         **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_sgactionurl>` :sup:`6`
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Contact Macros
--------------

=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`CONTACTNAME                   No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_cntctname>`
:ref:`CONTACTALIAS                  No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_cntctalias>`
:ref:`CONTACTEMAIL                  No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_cntctemail>`
:ref:`CONTACTPAGER                  No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_cntctpager>`
::ref:`CONTACTADDRESSn              No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_cntctaddressn>`
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Contact Group Macros
--------------------

=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`CONTACTGROUPALIAS             **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_cgalias>` :sup:`7`
:ref:`CONTACTGROUPMEMBERS           **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_cgmembers>` :sup:`7`
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Summary Macros
--------------

================================================ ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                                       Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
================================================ ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`TOTALHOSTSUP                               **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsup>` :sup:`10`
:ref:`TOTALHOSTSDOWN                             **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsdown>` :sup:`10`
:ref:`TOTALHOSTSUNREACHABLE                      **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsunreachable>` :sup:`10`
:ref:`TOTALHOSTSDOWNUNHANDLED                    **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsdownunhandled>` :sup:`10`
:ref:`TOTALHOSTSUNREACHABLEUNHANDLED             **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstsunreachableunhandled>` :sup:`10`
:ref:`TOTALHOSTPROBLEMS                          **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstpb>` :sup:`10`
:ref:`TOTALHOSTPROBLEMSUNHANDLED                 **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalhstpbunhandled>`
:ref:`TOTALSERVICESOK                            **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalsvcok>` :sup:`10`
:ref:`TOTALSERVICESWARNING                       **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalsvcwarning>` :sup:`10`
:ref:`TOTALSERVICESCRITICAL                      **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalsvccritical>` :sup:`10`
:ref:`TOTALSERVICESUNKNOWN                       **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalsvcunknown>` :sup:`10`
:ref:`TOTALSERVICESWARNINGUNHANDLED              **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalsvcwarningunhandled>` :sup:`10`
:ref:`TOTALSERVICESCRITICALUNHANDLED             **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalsvccriticalunhandled>` :sup:`10`
:ref:`TOTALSERVICESUNKNOWNUNHANDLED              **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalsvcunknownunhandled>` :sup:`10`
:ref:`TOTALSERVICEPROBLEMS                       **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalsvcepb>` :sup:`10`
:ref:`TOTALSERVICEPROBLEMSUNHANDLED              **Yes**        **Yes** :sup:`4`      **Yes**     **Yes** :sup:`4`   **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_totalsvcpbunhandled>` :sup:`10`
================================================ ============== ===================== =========== ================== =============================== ============================ ================= ==============

Notification Macros
-------------------

=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`NOTIFICATIONTYPE              No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_notiftype>`
:ref:`NOTIFICATIONRECIPIENTS        No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_notifrecipients>`
:ref:`NOTIFICATIONISESCALATED       No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_notifisescalated>`
:ref:`NOTIFICATIONAUTHOR            No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_notifauthor>`
:ref:`NOTIFICATIONAUTHORNAME        No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_notifauthorname>`
:ref:`NOTIFICATIONAUTHORALIAS       No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_notifauthoralias>`
:ref:`NOTIFICATIONCOMMENT           No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_notifcomment>`
:ref:`HOSTNOTIFICATIONNUMBER        No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_hstnotifnumber>`
:ref:`HOSTNOTIFICATIONID            No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_hstnotifid>`
:ref:`SERVICENOTIFICATIONNUMBER     No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_svcnotifnumber>`
:ref:`SERVICENOTIFICATIONID         No             **Yes**               No          **Yes**            No                              No                           No                No
<macro_svcnotifid>`
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Date/Time Macros
----------------

=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`LONGDATETIME                  **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_longdatetime>`
:ref:`SHORTDATETIME                 **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_shortdatetime>`
:ref:`DATE                          **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_date>`
:ref:`TIME                          **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_time>`
:ref:`TIMET                         **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_timet>`
:ref:`ISVALIDTIME                   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_isvalidtime>` :sup:`9`
:ref:`NEXTVALIDTIME                 **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_nextvalidtime>` :sup:`9`
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

File Macros
-----------

=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`MAINCONFIGFILE                **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_mainconfigfile>`
:ref:`STATUSDATAFILE                **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_statusdatafile>`
:ref:`COMMENTDATAFILE               **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_commentdatafile>`
:ref:`DOWNTIMEDATAFILE              **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_downtimedatafile>`
:ref:`RETENTIONDATAFILE             **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_retentiondatafile>`
:ref:`OBJECTCACHEFILE               **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_objectcachefile>`
:ref:`TEMPFILE                      **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_tempfile>`
:ref:`TEMPPATH                      **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_temppath>`
:ref:`LOGFILE                       **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_logfile>`
:ref:`RESOURCEFILE                  **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_resourcefile>`
:ref:`COMMANDFILE                   **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_commandfile>`
:ref:`HOSTPERFDATAFILE              **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_hstperfdatafile>`
:ref:`SERVICEPERFDATAFILE           **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_svcperfdatafile>`
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Misc Macros
-----------

=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
Macro Name                          Service Checks Service Notifications Host Checks Host Notifications Service Event Handlers and OCSP Host Event Handlers and OCHP Service Perf Data Host Perf Data
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============
:ref:`PROCESSSTARTTIME              **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_processstarttime>`
:ref:`EVENTSTARTTIME                **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_eventstarttime>`
:ref:`ADMINEMAIL                    **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_adminemail>`
:ref:`ADMINPAGER                    **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
<macro_adminpager>`
:ref:`ARGn <macro_argn>`            **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
:ref:`USERn <macro_usern>`          **Yes**        **Yes**               **Yes**     **Yes**            **Yes**                         **Yes**                      **Yes**           **Yes**
=================================== ============== ===================== =========== ================== =============================== ============================ ================= ==============

Macro Descriptions
==================

Host Macros :sup:`3`
--------------------

============================ =========================================================================================================================
HOSTNAME                     Short name for the host (i.e. "biglinuxbox"). This value is taken from the host_name directive in the
                             :ref:`host definition <basics_object_definitions#object_definitionsobjecttypeshostdefinition>`.
HOSTDISPLAYNAME              An alternate display name for the host. This value is taken from the display_name directive in the
                             :ref:`host definition <basics_object_definitions#object_definitionsobjecttypeshostdefinition>`.
HOSTALIAS                    Long name/description for the host. This value is taken from the alias directive in the
                             :ref:`host <basics_object_definitions#object_definitionsobjecttypeshostdefinition>` definition".
HOSTADDRESS                  Address of the host. This value is taken from the address directive in the
                             :ref:`host definition <basics_object_definitions#object_definitionsobjecttypeshostdefinition>`.
HOSTSTATE                    A string indicating the current state of the host ("UP", "DOWN", or "UNREACHABLE").
HOSTSTATEID                  A number that corresponds to the current state of the host: 0=UP, 1=DOWN, 2=UNREACHABLE.
LASTHOSTSTATE                A string indicating the last state of the host ("UP", "DOWN", or "UNREACHABLE").
LASTHOSTSTATEID              A number that corresponds to the last state of the host: 0=UP, 1=DOWN, 2=UNREACHABLE.
HOSTSTATETYPE                A string indicating the :ref:`state type <state_types>` for the current host check ("HARD" or "SOFT"). Soft states occur
                             when host checks return a non-OK (non-UP) state and are in the process of being retried. Hard states result when host
                             checks have been checked a specified maximum number of times.
HOSTATTEMPT                  The number of the current host check retry. For instance, if this is the second time that the host is being rechecked,
                             this will be the number two. Current attempt number is really only useful when writing host event handlers for "soft"
                             states that take a specific action based on the host retry number.
MAXHOSTATTEMPTS              The max check attempts as defined for the current host. Useful when writing host event handlers for "soft" states that
                             take a specific action based on the host retry number.
HOSTEVENTID                  A globally unique number associated with the host's current state. Every time a host (or service) experiences a state
                             change, a global event ID number is incremented by one (1). If a host has experienced no state changes, this macro will
                             be set to zero (0).
LASTHOSTEVENTID              The previous (globally unique) event number that was given to the host.
HOSTPROBLEMID                A globally unique number associated with the host's current problem state. Every time a host (or service) transitions
                             from an UP or OK state to a problem state, a global problem ID number is incremented by one (1). This macro will be
                             non-zero if the host is currently a non-UP state. State transitions between non-UP states (e.g. DOWN to UNREACHABLE) do
                             not cause this problem id to increase. If the host is currently in an UP state, this macro will be set to zero (0).
                             Combined with event handlers, this macro could be used to automatically open trouble tickets when hosts first enter a
                             problem state.
LASTHOSTPROBLEMID            The previous (globally unique) problem number that was given to the host. Combined with event handlers, this macro could
                             be used for automatically closing trouble tickets, etc. when a host recovers to an UP state.
HOSTLATENCY                  A (floating point) number indicating the number of seconds that a scheduled host check lagged behind its scheduled check
                             time. For instance, if a check was scheduled for 03:14:15 and it didn't get executed until 03:14:17, there would be a
                             check latency of 2.0 seconds. On-demand host checks have a latency of zero seconds.
HOSTEXECUTIONTIME            A (floating point) number indicating the number of seconds that the host check took to execute (i.e. the amount of time
                             the check was executing).
HOSTDURATION                 A string indicating the amount of time that the host has spent in its current state. Format is "XXh YYm ZZs", indicating
                             hours, minutes and seconds.
HOSTDURATIONSEC              A number indicating the number of seconds that the host has spent in its current state.
HOSTDOWNTIME                 A number indicating the current "downtime depth" for the host. If this host is currently in a period of
                             :ref:`scheduled downtime <advanced_scheduled_downtime>`, the value will be greater than zero. If the host is not
                             currently in a period of downtime, this value will be zero.
HOSTPERCENTCHANGE            A (floating point) number indicating the percent state change the host has undergone. Percent state change is used by the
                             :ref:`flap detection <advanced_detection_and_handling_of_state_flapping>` algorithm.
HOSTGROUPNAME                The short name of the hostgroup that this host belongs to. This value is taken from the hostgroup_name directive in the
                             :ref:`hostgroup definition <basics_object_definitions#object_definitionsobjecttypeshostgroupdefinition>`. If the host
                             belongs to more than one hostgroup this macro will contain the name of just one of them.
HOSTGROUPNAMES               A comma separated list of the short names of all the hostgroups that this host belongs to.
LASTHOSTCHECK                This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which a check of the host was
                             last performed.
LASTHOSTSTATECHANGE          This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time the host last changed state.
LASTHOSTUP                   This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the host was last
                             detected as being in an UP state.
LASTHOSTDOWN                 This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the host was last
                             detected as being in a DOWN state.
LASTHOSTUNREACHABLE          This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the host was last
                             detected as being in an UNREACHABLE state.
HOSTOUTPUT                   The first line of text output from the last host check (i.e. "Ping OK").
LONGHOSTOUTPUT               The full text output (aside from the first line) from the last host check.
HOSTPERFDATA                 This macro contains any :ref:`performance data <advanced_performance_data>` that may have been returned by the last host
                             check.
HOSTCHECKCOMMAND             This macro contains the name of the command (along with any arguments passed to it) used to perform the host check.
HOSTACKAUTHOR :sup:`8`       A string containing the name of the user who acknowledged the host problem. This macro is only valid in notifications
                             where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
HOSTACKAUTHORNAME :sup:`8`   A string containing the short name of the contact (if applicable) who acknowledged the host problem. This macro is only
                             valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
HOSTACKAUTHORALIAS :sup:`8`  A string containing the alias of the contact (if applicable) who acknowledged the host problem. This macro is only valid
                             in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
HOSTACKCOMMENT :sup:`8`      A string containing the acknowledgement comment that was entered by the user who acknowledged the host problem. This
                             macro is only valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
HOSTACTIONURL                Action URL for the host. This macro may contain other macros (e.g. $HOSTNAME$), which can be useful when you want to pass
                             the host name to a web page.
HOSTNOTESURL                 Notes URL for the host. This macro may contain other macros (e.g. $HOSTNAME$), which can be useful when you want to pass
                             the host name to a web page.
HOSTNOTES                    Notes for the host. This macro may contain other macros (e.g. $HOSTNAME$), which can be useful when you want to
                             host-specific status information, etc. in the description.
TOTALHOSTSERVICES            The total number of services associated with the host.
TOTALHOSTSERVICESOK          The total number of services associated with the host that are in an OK state.
TOTALHOSTSERVICESWARNING     The total number of services associated with the host that are in a WARNING state.
TOTALHOSTSERVICESUNKNOWN     The total number of services associated with the host that are in an UNKNOWN state.
TOTALHOSTSERVICESCRITICAL    The total number of services associated with the host that are in a CRITICAL state.
============================ =========================================================================================================================

Host Group Macros
-----------------

============================ =========================================================================================================================
HOSTGROUPALIAS :sup:`5`      The long name / alias of either 1) the hostgroup name passed as an on-demand macro argument or 2) the primary hostgroup
                             associated with the current host (if not used in the context of an on-demand macro). This value is taken from the alias
                             directive in the
                             :ref:`hostgroup definition <basics_object_definitions#object_definitionsobjecttypeshostgroupdefinition>`.
HOSTGROUPMEMBERS :sup:`5`    A comma-separated list of all hosts that belong to either 1) the hostgroup name passed as an on-demand macro argument or
                             2) the primary hostgroup associated with the current host (if not used in the context of an on-demand macro).
HOSTGROUPNOTES :sup:`5`      The notes associated with either 1) the hostgroup name passed as an on-demand macro argument or 2) the primary hostgroup
                             associated with the current host (if not used in the context of an on-demand macro). This value is taken from the notes
                             directive in the
                             :ref:`hostgroup definition <basics_object_definitions#object_definitionsobjecttypeshostgroupdefinition>`.
HOSTGROUPNOTESURL :sup:`5`   The notes URL associated with either 1) the hostgroup name passed as an on-demand macro argument or 2) the primary
                             hostgroup associated with the current host (if not used in the context of an on-demand macro). This value is taken from
                             the notes_url directive in the
                             :ref:`hostgroup definition <basics_object_definitions#object_definitionsobjecttypeshostgroupdefinition>`.
HOSTGROUPACTIONURL :sup:`5`  The action URL associated with either 1) the hostgroup name passed as an on-demand macro argument or 2) the primary
                             hostgroup associated with the current host (if not used in the context of an on-demand macro). This value is taken from
                             the action_url directive in the
                             :ref:`hostgroup definition <basics_object_definitions#object_definitionsobjecttypeshostgroupdefinition>`.
============================ =========================================================================================================================

Service Macros
--------------

=============================== ======================================================================================================================
SERVICEDESC                     The long name/description of the service (i.e. "Main Website"). This value is taken from the service_description
                                directive of the
                                :ref:`service definition <basics_object_definitions#object_definitionsobjecttypesservicedefinitionservice>`.
SERVICEDISPLAYNAME              An alternate display name for the service. This value is taken from the display_name directive in the
                                :ref:`service definition <basics_object_definitions#object_definitionsobjecttypesservicedefinitionservice>`.
SERVICESTATE                    A string indicating the current state of the service ("OK", "WARNING", "UNKNOWN", or "CRITICAL").
SERVICESTATEID                  A number that corresponds to the current state of the service: 0=OK, 1=WARNING, 2=CRITICAL, 3=UNKNOWN.
LASTSERVICESTATE                A string indicating the last state of the service ("OK", "WARNING", "UNKNOWN", or "CRITICAL").
LASTSERVICESTATEID              A number that corresponds to the last state of the service: 0=OK, 1=WARNING, 2=CRITICAL, 3=UNKNOWN.
SERVICESTATETYPE                A string indicating the :ref:`state type <state_types>` for the current service check ("HARD" or "SOFT"). Soft states
                                occur when service checks return a non-OK state and are in the process of being retried. Hard states result when
                                service checks have been checked a specified maximum number of times.
SERVICEATTEMPT                  The number of the current service check retry. For instance, if this is the second time that the service is being
                                rechecked, this will be the number two. Current attempt number is really only useful when writing service event
                                handlers for "soft" states that take a specific action based on the service retry number.
MAXSERVICEATTEMPTS              The max check attempts as defined for the current service. Useful when writing host event handlers for "soft" states
                                that take a specific action based on the service retry number.
SERVICEISVOLATILE               Indicates whether the service is marked as being volatile or not: 0 = not volatile, 1 = volatile.
SERVICEEVENTID                  A globally unique number associated with the service's current state. Every time a a service (or host) experiences a
                                state change, a global event ID number is incremented by one (1). If a service has experienced no state changes, this
                                macro will be set to zero (0).
LASTSERVICEEVENTID              The previous (globally unique) event number that given to the service.
SERVICEPROBLEMID                A globally unique number associated with the service's current problem state. Every time a service (or host)
                                transitions from an OK or UP state to a problem state, a global problem ID number is incremented by one (1). This
                                macro will be non-zero if the service is currently a non-OK state. State transitions between non-OK states (e.g.
                                WARNING to CRITICAL) do not cause this problem id to increase. If the service is currently in an OK state, this macro
                                will be set to zero (0). Combined with event handlers, this macro could be used to automatically open trouble tickets
                                when services first enter a problem state.
LASTSERVICEPROBLEMID            The previous (globally unique) problem number that was given to the service. Combined with event handlers, this macro
                                could be used for automatically closing trouble tickets, etc. when a service recovers to an OK state.
SERVICELATENCY                  A (floating point) number indicating the number of seconds that a scheduled service check lagged behind its scheduled
                                check time. For instance, if a check was scheduled for 03:14:15 and it didn't get executed until 03:14:17, there would
                                be a check latency of 2.0 seconds.
SERVICEEXECUTIONTIME            A (floating point) number indicating the number of seconds that the service check took to execute (i.e. the amount of
                                time the check was executing).
SERVICEDURATION                 A string indicating the amount of time that the service has spent in its current state. Format is "XXh YYm ZZs",
                                indicating hours, minutes and seconds.
SERVICEDURATIONSEC              A number indicating the number of seconds that the service has spent in its current state.
SERVICEDOWNTIME                 A number indicating the current "downtime depth" for the service. If this service is currently in a period of
                                :ref:`scheduled downtime <advanced_scheduled_downtime>`, the value will be greater than zero. If the service is not
                                currently in a period of downtime, this value will be zero.
SERVICEPERCENTCHANGE            A (floating point) number indicating the percent state change the service has undergone. Percent state change is used
                                by the :ref:`flap detection <advanced_detection_and_handling_of_state_flapping>` algorithm.
SERVICEGROUPNAME                The short name of the servicegroup that this service belongs to. This value is taken from the servicegroup_name
                                directive in the :ref:`servicegroup <basics_object_definitions#object_definitionsobjecttypesservicegroupdefinition>`
                                definition". If the service belongs to more than one servicegroup this macro will contain the name of just one of
                                them.
SERVICEGROUPNAMES               A comma separated list of the short names of all the servicegroups that this service belongs to.
LASTSERVICECHECK                This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which a check of the
                                service was last performed.
LASTSERVICESTATECHANGE          This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time the service last changed
                                state.
LASTSERVICEOK                   This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                detected as being in an OK state.
LASTSERVICEWARNING              This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                detected as being in a WARNING state.
LASTSERVICEUNKNOWN              This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                detected as being in an UNKNOWN state.
LASTSERVICECRITICAL             This is a timestamp in time_t format (seconds since the UNIX epoch) indicating the time at which the service was last
                                detected as being in a CRITICAL state.
SERVICEOUTPUT                   The first line of text output from the last service check (i.e. "Ping OK").
LONGSERVICEOUTPUT               The full text output (aside from the first line) from the last service check.
SERVICEPERFDATA                 This macro contains any :ref:`performance data <advanced_performance_data>` that may have been returned by the last
                                service check.
SERVICECHECKCOMMAND             This macro contains the name of the command (along with any arguments passed to it) used to perform the service check.
SERVICEACKAUTHOR :sup:`8`       A string containing the name of the user who acknowledged the service problem. This macro is only valid in
                                notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
SERVICEACKAUTHORNAME :sup:`8`   A string containing the short name of the contact (if applicable) who acknowledged the service problem. This macro is
                                only valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
SERVICEACKAUTHORALIAS :sup:`8`  A string containing the alias of the contact (if applicable) who acknowledged the service problem. This macro is only
                                valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
SERVICEACKCOMMENT :sup:`8`      A string containing the acknowledgement comment that was entered by the user who acknowledged the service problem.
                                This macro is only valid in notifications where the $NOTIFICATIONTYPE$ macro is set to "ACKNOWLEDGEMENT".
SERVICEACTIONURL                Action URL for the service. This macro may contain other macros (e.g. $HOSTNAME$ or $SERVICEDESC$), which can be
                                useful when you want to pass the service name to a web page.
SERVICENOTESURL                 Notes URL for the service. This macro may contain other macros (e.g. $HOSTNAME$ or $SERVICEDESC$), which can be
                                useful when you want to pass the service name to a web page.
SERVICENOTES                    Notes for the service. This macro may contain other macros (e.g. $HOSTNAME$ or $SERVICESTATE$), which can be useful
                                when you want to service-specific status information, etc. in the description
=============================== ======================================================================================================================

Service Group Macros
--------------------

============================== =======================================================================================================================
SERVICEGROUPALIAS :sup:`6`     The long name / alias of either 1) the servicegroup name passed as an on-demand macro argument or 2) the primary
                               servicegroup associated with the current service (if not used in the context of an on-demand macro). This value is
                               taken from the alias directive in the
                               :ref:`servicegroup <basics_object_definitions#object_definitionsobjecttypesservicegroupdefinition>` definition".
SERVICEGROUPMEMBERS :sup:`6`   A comma-separated list of all services that belong to either 1) the servicegroup name passed as an on-demand macro
                               argument or 2) the primary servicegroup associated with the current service (if not used in the context of an
                               on-demand macro).
SERVICEGROUPNOTES :sup:`6`     The notes associated with either 1) the servicegroup name passed as an on-demand macro argument or 2) the primary
                               servicegroup associated with the current service (if not used in the context of an on-demand macro). This value is
                               taken from the notes directive in the
                               :ref:`servicegroup <basics_object_definitions#object_definitionsobjecttypesservicegroupdefinition>` definition".
SERVICEGROUPNOTESURL :sup:`6`  The notes URL associated with either 1) the servicegroup name passed as an on-demand macro argument or 2) the primary
                               servicegroup associated with the current service (if not used in the context of an on-demand macro). This value is
                               taken from the notes_url directive in the
                               :ref:`servicegroup definition <basics_object_definitions#object_definitionsobjecttypesservicegroupdefinition>`.
SERVICEGROUPNOTES :sup:`6`     The action URL associated with either 1) the servicegroup name passed as an on-demand macro argument or 2) the primary
                               servicegroup associated with the current service (if not used in the context of an on-demand macro). This value is
                               taken from the action_url directive in the
                               :ref:`servicegroup definition <basics_object_definitions#object_definitionsobjecttypesservicegroupdefinition>`.
============================== =======================================================================================================================

Contact Macros
--------------

================== ===================================================================================================================================
CONTACTNAME        Short name for the contact (i.e. "jdoe") that is being notified of a host or service problem. This value is taken from the
                   contact_name directive in the :ref:`contact definition <basics_object_definitions#object_definitionsobjecttypescontactdefinition>`.
CONTACTALIAS       Long name/description for the contact (i.e. "John Doe") being notified. This value is taken from the alias directive in the
                   :ref:`contact definition <basics_object_definitions#object_definitionsobjecttypescontactdefinition>`.
CONTACTEMAIL       Email address of the contact being notified. This value is taken from the email directive in the
                   :ref:`contact definition <basics_object_definitions#object_definitionsobjecttypescontactdefinition>`.
CONTACTPAGER       Pager number/address of the contact being notified. This value is taken from the pager directive in the
                   :ref:`contact definition <basics_object_definitions#object_definitionsobjecttypescontactdefinition>`.
CONTACTADDRESSn    Address of the contact being notified. Each contact can have six different addresses (in addition to email address and pager
                   number). The macros for these addresses are $CONTACTADDRESS1$ - $CONTACTADDRESS6$. This value is taken from the addressx directive
                   in the :ref:`contact definition <basics_object_definitions#object_definitionsobjecttypescontactdefinition>`.
CONTACTGROUPNAME   The short name of the contactgroup that this contact is a member of. This value is taken from the contactgroup_name directive in
                   the :ref:`contactgroup <basics_object_definitions#object_definitionsobjecttypescontactgroupdefinition>` definition. If the contact
                   belongs to more than one contactgroup this macro will contain the name of just one of them.
CONTACTGROUPNAMES  A comma separated list of the short names of all the contactgroups that this contact is a member of.
================== ===================================================================================================================================

Contact Group Macros
--------------------

============================ =========================================================================================================================
CONTACTGROUPALIAS :sup:`7`   The long name / alias of either 1) the contactgroup name passed as an on-demand macro argument or 2) the primary
                             contactgroup associated with the current contact (if not used in the context of an on-demand macro). This value is taken
                             from the alias directive in the
                             :ref:`contactgroup <basics_object_definitions#object_definitionsobjecttypescontactgroupdefinition>` definition".
CONTACTGROUPMEMBERS :sup:`7` A comma-separated list of all contacts that belong to either 1) the contactgroup name passed as an on-demand macro
                             argument or 2) the primary contactgroup associated with the current contact (if not used in the context of an
                             on-demand macro).
============================ =========================================================================================================================

Summary Macros
--------------

============================== =======================================================================================================================
TOTALHOSTSUP                   This macro reflects the total number of hosts that are currently in an UP state.
TOTALHOSTSDOWN                 This macro reflects the total number of hosts that are currently in a DOWN state.
TOTALHOSTSUNREACHABLE          This macro reflects the total number of hosts that are currently in an UNREACHABLE state.
TOTALHOSTSDOWNUNHANDLED        This macro reflects the total number of hosts that are currently in a DOWN state that are not currently being
                               "handled". Unhandled host problems are those that are not acknowledged, are not currently in scheduled downtime, and
                               for which checks are currently enabled.
TOTALHOSTSUNREACHABLEUNHANDLED This macro reflects the total number of hosts that are currently in an UNREACHABLE state that are not currently being
                               "handled". Unhandled host problems are those that are not acknowledged, are not currently in scheduled downtime, and
                               for which checks are currently enabled.
TOTALHOSTPROBLEMS              This macro reflects the total number of hosts that are currently either in a DOWN or an UNREACHABLE state.
TOTALHOSTPROBLEMSUNHANDLED     This macro reflects the total number of hosts that are currently either in a DOWN or an UNREACHABLE state that are not
                               currently being "handled". Unhandled host problems are those that are not acknowledged, are not currently in scheduled
                               downtime, and for which checks are currently enabled.
TOTALSERVICESOK                This macro reflects the total number of services that are currently in an OK state.
TOTALSERVICESWARNING           This macro reflects the total number of services that are currently in a WARNING state.
TOTALSERVICESCRITICAL          This macro reflects the total number of services that are currently in a CRITICAL state.
TOTALSERVICESUNKNOWN           This macro reflects the total number of services that are currently in an UNKNOWN state.
TOTALSERVICESWARNINGUNHANDLED  This macro reflects the total number of services that are currently in a WARNING state that are not currently being
                               "handled". Unhandled services problems are those that are not acknowledged, are not currently in scheduled downtime,
                               and for which checks are currently enabled.
TOTALSERVICESCRITICALUNHANDLED This macro reflects the total number of services that are currently in a CRITICAL state that are not currently being
                               "handled". Unhandled services problems are those that are not acknowledged, are not currently in scheduled downtime,
                               and for which checks are currently enabled.
TOTALSERVICESUNKNOWNUNHANDLED  This macro reflects the total number of services that are currently in an UNKNOWN state that are not currently being
                               "handled". Unhandled services problems are those that are not acknowledged, are not currently in scheduled downtime,
                               and for which checks are currently enabled.
TOTALSERVICEPROBLEMS           This macro reflects the total number of services that are currently either in a WARNING, CRITICAL, or UNKNOWN state.
TOTALSERVICEPROBLEMSUNHANDLED  This macro reflects the total number of services that are currently either in a WARNING, CRITICAL, or UNKNOWN state
                               that are not currently being "handled". Unhandled services problems are those that are not acknowledged, are not
                               currently in scheduled downtime, and for which checks are currently enabled.
============================== =======================================================================================================================

Notification Macros
-------------------

========================= ============================================================================================================================
NOTIFICATIONTYPE          A string identifying the type of notification that is being sent ("PROBLEM", "RECOVERY", "ACKNOWLEDGEMENT", "FLAPPINGSTART",
                          "FLAPPINGSTOP", "FLAPPINGDISABLED", "DOWNTIMESTART", "DOWNTIMEEND", or "DOWNTIMECANCELLED").
NOTIFICATIONRECIPIENTS    A comma-separated list of the short names of all contacts that are being notified about the host or service.
NOTIFICATIONISESCALATED   An integer indicating whether this was sent to normal contacts for the host or service or if it was escalated. 0 = Normal
                          (non-escalated) notification , 1 = Escalated notification.
NOTIFICATIONAUTHOR        A string containing the name of the user who authored the notification. If the $NOTIFICATIONTYPE$ macro is set to
                          "DOWNTIMESTART" or "DOWNTIMEEND", this will be the name of the user who scheduled downtime for the host or service. If the
                          $NOTIFICATIONTYPE$ macro is "ACKNOWLEDGEMENT", this will be the name of the user who acknowledged the host or service
                          problem. If the $NOTIFICATIONTYPE$ macro is "CUSTOM", this will be name of the user who initated the custom host or service
                          notification.
NOTIFICATIONAUTHORNAME    A string containing the short name of the contact (if applicable) specified in the $NOTIFICATIONAUTHOR$ macro.
NOTIFICATIONAUTHORALIAS   A string containing the alias of the contact (if applicable) specified in the $NOTIFICATIONAUTHOR$ macro.
NOTIFICATIONCOMMENT       A string containing the comment that was entered by the notification author. If the $NOTIFICATIONTYPE$ macro is set to
                          "DOWNTIMESTART" or "DOWNTIMEEND", this will be the comment entered by the user who scheduled downtime for the host or
                          service. If the $NOTIFICATIONTYPE$ macro is "ACKNOWLEDGEMENT", this will be the comment entered by the user who acknowledged
                          the host or service problem. If the $NOTIFICATIONTYPE$ macro is "CUSTOM", this will be comment entered by the user who
                          initated the custom host or service notification.
HOSTNOTIFICATIONNUMBER    The current notification number for the host. The notification number increases by one (1) each time a new notification is
                          sent out for the host (except for acknowledgements). The notification number is reset to 0 when the host recovers (after the
                          recovery notification has gone out). Acknowledgements do not cause the notification number to increase, nor do notifications
                          dealing with flap detection or scheduled downtime.
HOSTNOTIFICATIONID        A unique number identifying a host notification. Notification ID numbers are unique across both hosts and service
                          notifications, so you could potentially use this unique number as a primary key in a notification database. Notification ID
                          numbers should remain unique across restarts of the Centreon Engine process, so long as you have state retention enabled. The
                          notification ID number is incremented by one (1) each time a new host notification is sent out, and regardless of how many
                          contacts are notified.
SERVICENOTIFICATIONNUMBER The current notification number for the service. The notification number increases by one (1) each time a new notification
                          is sent out for the service (except for acknowledgements). The notification number is reset to 0 when the service recovers
                          (after the recovery notification has gone out). Acknowledgements do not cause the notification number to increase, nor do
                          notifications dealing with flap detection or scheduled downtime.
SERVICENOTIFICATIONID     A unique number identifying a service notification. Notification ID numbers are unique across both hosts and service
                          notifications, so you could potentially use this unique number as a primary key in a notification database. Notification ID
                          numbers should remain unique across restarts of the Centreon Engine process, so long as you have state retention enabled.
                          The notification ID number is incremented by one (1) each time a new service notification is sent out, and regardless of how
                          many contacts are notified.
========================= ============================================================================================================================

Date/Time Macros
----------------

====================== ===============================================================================================================================
LONGDATETIME           Current date/time stamp (i.e. Fri Oct 13 00:30:28 CDT 2000). Format of date is determined by
                       :ref:`date_format <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesdateformat>`
                       directive.
SHORTDATETIME          Current date/time stamp (i.e. 10-13-2000 00:30:28). Format of date is determined by
                       :ref:`date_format <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesdateformat>`
                       directive.
DATE                   Date stamp (i.e. 10-13-2000). Format of date is determined by
                       :ref:`date_format <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesdateformat>`
                       directive.
TIME                   Current time stamp (i.e. 00:30:28).
TIMET                  Current time stamp in time_t format (seconds since the UNIX epoch).
ISVALIDTIME :sup:`9`   This is a special on-demand macro that returns a 1 or 0 depending on whether or not a particular time is valid within a
                       specified timeperiod. There are two ways of using this macro:
                         * $ISVALIDTIME:24x7$ will be set to "1" if the current time is valid within the "24x7" timeperiod. If not, it will be set to
                           "0".
                         * $ISVALIDTIME:24x7:timestamp$ will be set to "1" if the time specified by the "timestamp" argument (which must be in time_t
                           format) is valid within the "24x7" timeperiod. If not, it will be set to "0".
NEXTVALIDTIME :sup:`9` This is a special on-demand macro that returns the next valid time (in time_t format) for a specified timeperiod. There are two
                       ways of using this macro:
                         * $NEXTVALIDTIME:24x7$ will return the next valid time from and including the current time in the "24x7" timeperiod.
                         * $NEXTVALIDTIME:24x7:timestamp$ will return the next valid time from and including the time specified by the "timestamp"
                           argument (which must be specified in time_t format) in the "24x7" timeperiod.If a next valid time cannot be found in the
                           specified timeperiod, the macro will be set to "0".
====================== ===============================================================================================================================

File Macros
-----------

=================== ==================================================================================================================================
MAINCONFIGFILE      The location of the :ref:`main config file <main_configuration_file_options>`.
STATUSDATAFILE      The location of the :ref:`status data file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstatusfile>`.
COMMENTDATAFILE     The location of the comment data file.
DOWNTIMEDATAFILE    The location of the downtime data file.
RETENTIONDATAFILE   The location of the :ref:`retention data file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesstateretentionfile>`.
OBJECTCACHEFILE     The location of the :ref:`object cache file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesobjectcachefile>`.
TEMPFILE            The location of the :ref:`temp file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablestempfile>`.
TEMPPATH            The directory specified by the temp path variable.
LOGFILE             The location of the :ref:`log file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableslogfile>`.
RESOURCEFILE        The location of the :ref:`resource file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesresourcefile>`.
COMMANDFILE         The location of the :ref:`command file <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesexternalcommandfile>`.
HOSTPERFDATAFILE    The location of the host performance data file (if defined).
SERVICEPERFDATAFILE The location of the service performance data file (if defined).
=================== ==================================================================================================================================

Misc Macros
-----------

================ =====================================================================================================================================
PROCESSSTARTTIME Time stamp in time_t format (seconds since the UNIX epoch) indicating when the Centreon Engine process was last (re)started. You can
                 determine the number of seconds that Centreon Engine has been running (since it was last restarted) by subtracting $PROCESSSTARTTIME$
                 from :ref:`$TIMET$ <basics_standard_macros#standard_macrosmacroavailabilitycharttimet>`.
EVENTSTARTTIME   Time stamp in time_t format (seconds since the UNIX epoch) indicating when the Centreon Engine process starting process events
                 (checks, etc.). You can determine the number of seconds that it took for Centreon Engine to startup by subtracting $PROCESSSTARTTIME$
                 from $EVENTSTARTTIME$.
ADMINEMAIL       Global administrative email address. This value is taken from the
                 :ref:`admin_email <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesadministratoremailaddress>`
                 directive.
ADMINPAGER       Global administrative pager number/address. This value is taken from the
                 :ref:`admin_pager <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesadministratorpager>`
                 directive.
ARGn             The nth argument passed to the command (notification, event handler, service check, etc.). Centreon Engine supports up to 32 argument
                 macros ($ARG1$ through $ARG32$).
USERn            The nth user-definable macro. User macros can be defined in one or more
                 :ref:`resource files <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariablesresourcefile>`.
                 Centreon Engine supports up to 256 user macros ($USER1$ through $USER256$).
================ =====================================================================================================================================

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
    :ref:`use_large_installation_tweaks <basics_main_configuration_file_options#main_configuration_file_optionsconfigurationfilevariableslarge_installation_tweaksoption>`
    option is enabled, as they are quite CPU-intensive to calculate.
