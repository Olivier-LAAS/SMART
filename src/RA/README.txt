This folder contains the reception agent code (previously vm_destinations)

########################
	RA: Reception Agent	
########################

It receives the transmitted packet from the overlay network, desencapsulates it and delivers it to the application

########################
	RUNNING	
########################

To execute RA, a configuration file is needed "mylocalip". It should contain the private IP address of RA. To get the private IP address use the command "ifconfig" and find your default port (usually eth0).

./RA 
