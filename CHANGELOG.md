# Changelog

## 20.10.3

### Bugfixes

*Retention*

Last time changes are checked when they are read from the retention files. So
if they have no meaning, they are replaced by a default value.

*Host/service status*

They could be sent twice. This new version fixes that.

fix update last_state_change and last_hard_state_change on host with no service

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
