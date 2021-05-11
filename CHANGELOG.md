# Changelog
## 21.04.3

### Bugfixes

*Check*

if host or service with check_period set to none, Engine burns 100% of the CPU.
This patch fixes this issue.

## 21.04.2

### Bugs

*Build*

Since the closure of bintray, we could no more download our dependencies. We
changed them to use the conan-center. Now centengine builds again.

*loop*

Sometimes, centengine can not stop, this has been seen when engine is compiled
using gcc-4.8.5, because of a bug in the function sleep\_until().

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
