# Changelog

## 20.04.14

### Bugs

*Downtime*

Change downtime limits start/end dates to timestamp 2145916799
(31/12/2037 23:59:59) max.

*logs*

There was a possible deadlock in logging::broker due to creation of logs when
sending logs, it was then possible to lock a mutex twice.

*Downtime*

Change Limit downtime start date, end date to 2145916799 (31/12/2037 23:59:59) max
## 20.04.13

### Bugs

*gRPC*

The reflection is removed because of an issue during the compilation.

*notification*

Recovery notifications forgotten when engine is stopped during incident.
This patch fixes this issue.

### Build

repair the compilation for Raspberry PI

## 20.04.12

### Bugfixes

*Build*

centengine builds again using packages provided by conan-center.

*Check*

if host or service with check\_period set to none, Engine burns 100% of the CPU.
This patch fixes this issue.

## 20.04.11

### Bugfixes

*Macros*

If a service/host is not in a service/host group, then the HOSTGROUPNAME/
SERVICEGROUPNAME macros can lead to segfault. This patch fixes this issue.

## 20.04.10

### Bugfixes

*Host/service status*

They could be sent twice. This new version fixes that.

*Notification macros*

The url encode action is removed from macros $HOSTACTIONURL*$, $HOSTNOTESURL*$,
$SERVICEACTIONURL*$, $SERVICENOTESURL*$, $HOSTGROUPNOTESURL*$,
$HOSTGROUPACTIONURL*$, $SERVICEGROUPNOTESURL*$, $SERVICEGROUPACTIONURL*$.

these macro are now available without parameters ($HOSTGROUPNAME$, $CONTACTGROUPNAME$, $SERVICEGROUPNAME$)

*Notification Period*

The timeperiods work on contacts and notifications are not sent anymore if the
time period is empty. The notification recovery are now sent when entering in
timeperiod notification.

*Downtime*

Add Limit downtime start date, end date to 4102441200 (31/12/209 23:59) max and duration to 31622400 (366 days) max

## 20.04.9

`January 20, 2021`

### Bugfixes

*Notification macros*

The macros in which notification information can be found have been fixed
(ie $NOTIFICATION*$, $HOSTNOTIFICATION*$, $SERVICENOTIFICATION*$)

### Enhancements

*Instance updates*

There is a minimal delay specified in seconds between two instance updates.
By default, its value is 30s. It can be set with the variable
instance_heartbeat_interval in the centengine.cfg file.

## 20.04.8

`December 16, 2020`

### Bugfixes

*Stalking option*

The stalking option works again, it has been fixed. Make sure you are not
enabling volatile option at the same time to really get an output
stalking.

*Macros filters*

Macros can be filtered. This was possible before and there was a
regression breaking this functionality. So now, we can activate the
macros filtering and then we can specify which macros to send to broker.

*Notifications*

Host/service status field 'Last Notification' was filled when
state was HARD even if no notification is configured nor sent.

## 20.04.7

`Octobre 12, 2020`

### Bugfixes

*Recovery notifications*

Users were receiving recovery notifications even though they weren't notified
for a problem. This is now fixed.

## 20.04.6

`September 3, 2020`

### Bugfixes

*PROBLEMID macros*

It was buggy on hosts and on services (ie HOSTPROBLEMID, LASTHOSTPROBLEMID,
SERVICEPROBLEMID and LASTSERVICEPROBLEMID). This new version fixes this point.

## 20.04.5

`August 18, 2020`

### Bugfixes

*Unicode check was buggy*

The code that validates the UTF-8 strings was buggy and could keep as is some
characters that were not UTF-8. It is fixed and moved to the cbmod module.

## 20.04.4

`July 6, 2020`

### Bugfixes

*On-demand macros on services do not work*

On-demand on service did not work, and most of the time crashed.
This is fixed with this new version.

## 20.04.3

`June 23, 2020`

### Bugfixes

*Windows checks can be CP1252 encoded*

To write into the database such strings is impossible unless we convert the
string to utf-8. This is what is done in this new Engine version. Each time
a check is done, we verify its output is in UTF-8 format, if it is not the
case, it is converted. Supported input encodings are ISO-8859-15, CP-1252 and
UTF-8.

## 20.04.2

`June 16, 2020`

### Bugfixes

*If a host is disabled, it should also be the case for its services*

If a host with several services is disabled, its services are removed from
the monitoring. But a query in centreon_storage shows that those services
are still there. With this new version, it is fixed.

## 20.04.1

`May 12, 2020`

### Bugfixes

*debug_lvl=-1*

Engine was stuck when we put -1 as debug_lvl in centengine.cfg.

## 20.04.0

`April 22, 2020`

### Bugfixes

*Perfdata truncated when read from retention*

Values containing a ';' character were truncated when read from the retention
file. This new release fixes this issue.

*Notifications between two fixed contiguous downtimes*

It was possible to have notifications sent between the two downtimes even if
the space duration is 0.

*Macros replacements*

Host macros and several global macros containing numbers were badly replaced.

### Enhancements

*Support for POLLERNAME macro*

You can now use $POLLERNAME$ macro to retrieve the name of your poller in
a check_command. It will use the poller_name field of your config.

*Support for POLLERID macro*

You can now use $POLLERID$ macro to retrieve the name of your poller in
a check_command. It will use the poller_id field of your config.
