# Documentation gRPC for usage #

## Installation ##

Verify that you have installed theses packages on your system else 
	$> yum install python3-devel
	$> yum install gcc
	$> yum install epel-release

Check that you have at least theses modules installed by executing :
	pip3 list

<img src="https://zupimages.net/up/21/02/8ls1.png"/>

Of course grpcs and protobuf modules are not yet installed so you can do :
	$> pip3 install grpcio==1.33.2

Note the 1.33.2 version, highter version may not work.

## Execution ##

Before executing a python script like *client.py* for testing purpose. 
Execute *init-proto.sh* to generate *engine_pb2_grpc.py* and *engine_pb2.py* files.

When you execute your script, please specify the good engine IPv6 port.
To know engine port :
        $> netstat -paunt



