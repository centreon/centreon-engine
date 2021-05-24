# Documentation gRPC for usage #

## Installation of gRPC ##

Verify that you have installed theses packages on your system else install them. On Centos 7 you can execute this command below :

```yum install python3-devel
   yum install gcc-c++
   yum install gcc
   yum install epel-release
   yum install net-tools
   yum install wget```

Now you have to install gRPC, so you can do these commands below (on Centos 7) :

```pip3 install grpcio==1.33.2```

<img src="https://zupimages.net/up/21/18/dkto.png" />

```pip3 install grpcio-tools=1.33.2```

<img src="https://zupimages.net/up/21/18/900w.png" />

/!\ Note the 1.33.2 version, highter version may not work (on Centos 7). /!\

## Download the python script ##

`mkdir ~/install-grpc/ && cd ~/install-grpc/` </br>
`wget https://raw.githubusercontent.com/centreon/centreon-engine/master/enginerpc/engine.proto` </br>
`mkdir ~/install-grpc/script/ && cd ~/install-grpc/script/` </br>
`wget https://raw.githubusercontent.com/centreon/centreon-engine/master/enginerpc/examples/init-proto.sh` </br>
`wget https://raw.githubusercontent.com/centreon/centreon-engine/master/enginerpc/examples/engine-rpc-client.py`

## Execution ##

Before executing a python script like *client.py* for testing purpose. 
Execute *init-proto.sh* to generate *engine_pb2_grpc.py* and *engine_pb2.py* files.

```cd ~/install-grpc/script/ && ./init-proto.sh```

When you execute your script, please specify the good engine IPv6 port.
To know engine port :

```netstat -paunt | grep centengine```

<img src="https://zupimages.net/up/21/18/bba2.png" />

To verify that the script is working well, you can run this command as an example : 

```python3 engine-rpc-client.py --port={your-current-engine-port} --exe=GetVersion```


## Infos ##

I tested this guide on Centreon 20.04, 20.10 and 21.04, that i downloaded from this link : https://download.centreon.com/. </br>
I did not encounter any particular errors.

