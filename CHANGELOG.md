# Changelog

## 21.04.3

### Bugs

*external commands*

The new C++ standard is not compatible with pthread\_cancel(). This last one has
been removed from the code.

*gRPC*

The reflection module has been removed because of an issue in the compilation.

*notification*

Recovery notifications forgotten when engine is stopped during incident.
This patch fixes this issue.

*Check*

If a service check is forced, two service status are sent to broker while only
one would be enough.

### Build

repair the compilation for Raspberry PI. Link between cbmod and neb module is
also rectified.

## 21.04.2

### Bugs

*Build*

Since the closure of bintray, we could no more download our dependencies. We
changed them to use the conan-center. Now centengine builds again.

*loop*

Sometimes, centengine can not stop, this has been seen when engine is compiled
using gcc-4.8.5, because of a bug in the function sleep\_until().

*Check*

if host or service with check_period set to none, Engine burns 100% of the CPU.
This patch fixes this issue.

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
