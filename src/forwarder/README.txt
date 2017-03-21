This folder contains the forwarder code

########################
	FORWARDER	
########################

The forwarder main function is to pass data packet to the next destination.

It listens on the DATA_PORT with a socket for incoming packet. When a packet is received it looks at the panacea header for the next destination.
If the header is empty,meaning it has just been intercepted by the transmission agent,
the forwarder will look up a path for the original destination in its routing table and write it in the panacea header before sending the packet to the next forwarder.
If there is no path in the routing table for the destination the forwarder will send the packet on the direct path.

To be able to update the routing table the forwarder is connected to a router as the client.
When a new path is calculated for a destination the router sends it to the forwarder which updates its routing table.

########################
	RUNNING	
########################

To execute forwarder you could passe routerip as an argument otherwise you should provide it in a configuration file

./forwarder (routerip)


