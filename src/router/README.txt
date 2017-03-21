Router contains the main intelligence of SMART.
For detailed informations about the implemented algorithms please refer to joined manuscripts.

############################
  	  RUNNING
############################

You can start the router in two distinct modes :
- normal (real network operation)
- simulation (predefined measures)

In the normal mode the CU takes care of the deployment configuration. While in the simulation mode configuration files are still needed.

To start router you need three arguments :

- C : the IP address of the CU
- a : the algorithm (algo for example euler, exhaustive...)
- r : the role (see graphCom)
- m : the mode, rtt or bw
- K : the max number of hops in a path (nb)(optional)
- probe-timeout (option) : duration of measurement for the probe (default 10s)
- probe-period (option) : total duration of a cycle of measurement (measure + sleep, default 30s)

./router -C <CU_IP> -a <algo> -r <role> -m <mode> -K <nb> --probe-timeout=<timeout_in_s> --probe-period=<period_in_s> 

############################
  	  SIMULATION
############################
The simulation mode use the measurements from a matrix to determine the best path and doesn't need real measurements on a network.


To start router you need three arguments :

- the algorithm (algo for example euler, exhaustive...)
- The role (see graphCom)
- the max number of hops in a path (nb)
- the name of the file which contains the results matrix (fileMatrix)

./router algo role nb fileMatrix


You can do the simulation for different metric. To do so you need to change BW_METRIC <--> LATENCY_METRIC in RoutingManager.cpp, runSimulation()
																			                              and SimulationManager.cpp, writeResultInFile()

For the simulation you need configuration files

############################
   CONFIGURATION FILES
############################
Please note that in simulation mode not all results are used. A sub group of destinations may only be used
in the following example only forwarder3 

1. File "myip" :
forwarder0

2.File "destinations":
forwarder1
forwarder3

4.File "ip_adress":
forwarder0
forwarder1
forwarder2
forwarder3

!! myip can contains multiple ip for multiple sources but only for exhaustif

############################
   RESULTAT SIMULATION
############################

The simulation write the result in a file for each instant as following :

forwarder0_forwarder1	result01	optimalPath01
forwarder0_forwarder3	result03	optimalPath03

