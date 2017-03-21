This folder contains CU (central unit) files

The central unit takes in charge the configuration of router, monitor, forwarder and clients. It is composed mainly of:

- CU files: CU/CU.cpp, CU/CU.h, CU/main.cpp
- daemon files: src/router/daemon.h, daemon.cpp

The CU ip address is hardcoded into the daemon.cpp, this is a universal address that should be used by all SMART components. 
Normally we have one CU per service, but at this stage, we consider one CU per SMART.

The CU need a configuration file

############################
   CONFIGURATION FILES
############################
With a communication graph such as :

           1 <------------> 2 

File "graphCom" :
1 2
2 1

With a communication graph such as :

   1 <--------- 2 -------> 3
   
File "graphCom" :
1 
2 1 3
3  

############################

The router executable does not need any configuration file

Monitor, Forwarder and TA executables need the ip address of their associated router. Actually this address is provided by a configuration file.
For simplicity, at first we put Router, Monitor and Forwarder on the same machine, so they all have the same address. (meanwhile we need the configuration file in this version of code)
TA should get the router ip address upon creation provided by the cloud manager.

RA executable needs its own private address. Note that RA is only capable to obtain its public ip address.

Normally we should launch:

1- CU
2- Router(s)
3- Monitor, Forwarder
4- Clients

For now order is important, in normal functioning secondary entities should wait until connection with CU (or Router) is established.


######################################
             Important
######################################

If any machine (forwarder, router, monitor or TA) leaves the SMART network the CU does not update the network topology.
To be handled ....


