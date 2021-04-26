# Changelog

## 21.04.1

### Bugs

*Macros*

If a service / host is not in a service/host group, then the HOSTGROUPNAME/
SERVICEGROUPNAME macros can lead to segfault. This patch fixes this issue.

## 21.04.0

`To be released in april 2021`

### New features

*External commands*

Engine hosts a gRPC server now. All the external commands can be executed
through this new server now.

*Flapping*

The internal flapping object does not reference notifiers by host name and
service description but by host id and service id.
