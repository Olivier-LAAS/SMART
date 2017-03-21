
############################
	DEPENDENCIES
############################

To install the required dependencies on an Ubuntu Linux system :

	sudo ./utils/install_dependencies_ubuntu.sh


############################
	COMPILATION
############################

Generate the Makefile:
	cd build
	cmake ..

To build the whole SMART system, enter the build directory and type :
	make

You can choose to build each agent separately  : 
	make router
	make monitor
	make RA
	make TA
	make forwarder

Generated executables can then be found in build/exe/ directory.

############################
   DOXYGEN DOCUMENTATION
############################
To generate the documentation, enter the build directory and type :
	make doc

The generated documentation can be found in build/doc/html/index.html

############################
   CONFIGURATION FILES
############################


Monitor configuration :
----------------------

1.File 'routerip'
routerip

Or you can passe routerip as parameter to the monitor executable (see monitor README file for more explanations)


Forwarder configuration :
-------------------------

1.File 'routerip'
routerip

Or you can passe routerip as parameter to the forwarder executable (see forwarder README file for more explanations)


TA configuration  :
---------------------

1. 'routerip' :
routerip

Or you can passe routerip as parameter to the TA executable (see TA README file for more explanations)

/////////////////////////////////////////////////////////////////////////////////////
//This configuration file is no more useful as only one TA could be used in a VM.
//We keep it only for 
2. 'nfqueueid'
nfqueueid
/////////////////////////////////////////////////////////////////////////////////////
  
RA configuration :
---------------------

1. 'mylocalip' :
mylocalip

It is the private ip of the RA VM. if public IP is used (no NAT) mylocalip=mypublicip

############################
        RUNNING
############################

Each module can be run separatly or you can use the scripts located in /utils

To run with the scripts you first need to configure them (see README file of each module)
Then you need to run them in the following order :
 - CU : on the CU
 - runProxy.sh : on each proxy (killProxy.sh to stop)
 - runTARA.sh : on each client/server (killTARA.sh to stop)

For the script killTARA.sh you need to replace <dest_RA_TA_ip> by the IP address of the other end machine (if several end point you need to copy the line for each of them)


