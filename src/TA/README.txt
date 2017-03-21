This folder contains the Transmission Agent code (vm_source previously)

########################
TA: Transmission Agent	
########################

It communicates with Router Manager to know which packets to intercept.
It intercepts packets, encapsulates them, and send them to the Forwarder.

########################
	RUNNING	
########################

Before running the agent add the following iptables (ONLY ONCE):

sudo iptables -A INPUT -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1410


To execute TA you could passe routerip as an argument otherwise you should provide it in a configuration file

./TA <routerip> <local_TA_ip>

- local_TA_ip : the public IP of the machine where TA is running

########################
	NFQUEUE	- Important
########################

If many TAs share the VM the nfqueueid value should be different for each TA.  Before a configuration file for nfqueueid allowed setting up this value.
It is no more the case. There could be only one TA per VM.
Otherwise TA code should be changed
