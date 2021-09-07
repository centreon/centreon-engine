# Changelog

## 20.10.7

### Bugs

*logs*

There was a possible deadlock in logging::broker due to creation of logs when
sending logs, it was then possible to lock a mutex twice.

*Downtime*

Change Limit downtime start date, end date to 2145916799 (31/12/2037 23:59:59) max

*notification*

now notification is not send if the host is on soft state_down and "Soft Service Dependencies Option" is activate.

the notification recovery now is send of all contact notified for normal notifcation (critical,warning,unknown).

## 20.10.6

### Bugs

*external commands*

the new C++ standard is not compatible with pthread\_cancel(). This last one is
removed.

*gRPC*

Reflection module has been removed because of an issue in the compilation.

*notification*

Recovery notifications forgotten when engine is stopped during incident.
This patch fixes this issue.

*Check*

If a service check is forced, two service status are sent to broker while only
one would be enough.

### Build

repair the compilation for Raspberry PI

## 20.10.5

### Bugfixes

*Build*

centengine builds again, using dependency packages available on conan-center.

*Check*

if host or service with check\_period set to none, Engine burns 100% of the CPU.
This patch fixes this issue.

## 20.10.4

### Bugfixes

*Macros*

If a service / host is not in a service/host group, then the HOSTGROUPNAME/
SERVICEGROUPNAME macros can lead to segfault. This patch fixes this issue.

## 20.10.3

### Bugfixes

*Retention*

Last time changes are checked when they are read from the retention files. So
if they have no meaning, they are replaced by a default value.

*Host/service status*

They could be sent twice. This new version fixes that.

fix update last\_state\_change and last\_hard\_state\_change on host with no
service

*Notification macros*

The url encode action is removed from macros $HOSTACTIONURL*$, $HOSTNOTESURL*$,
$SERVICEACTIONURL*$, $SERVICENOTESURL*$, $HOSTGROUPNOTESURL*$,
$HOSTGROUPACTIONURL*$, $SERVICEGROUPNOTESURL*$, $SERVICEGROUPACTIONURL*$.

*Notification Period*

Timeperiods are now filtered for contacts
and notifications are not pushed with an empty one.

*Macros*

These macros are now available without parameters
($HOSTGROUPNAME$, $CONTACTGROUPNAME$, $SERVICEGROUPNAME$)

*Connectors*

When a Perl connector or an SSH connector is used, engine could crash when we
stop it.

When a connector is destroyed, a deadlock could appear while waiting its
destruction.

*Connector restart*

If a connector stops working because of a crash or whatever else, it was not
restarted automatically. It is fixed now.

*Check commands*

When many commands are running, engine could crash when we stop it.

*Downtime*

Add Limit downtime start date, end date to 4102441200 (31/12/209 23:59) max and duration to 31622400 (366 days) max
## 20.10.2

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

## 20.10.1

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

## 20.10.0

`October 21, 2020`

### Bugfixes

- Contains all fixes up to version 20.04.7
