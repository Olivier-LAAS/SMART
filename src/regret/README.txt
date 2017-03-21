This module calculates the regret value for all routes.
It is completely independent from other parts of code.

Unlike SMART, it needs configuration files

############################
    CONFIGURATION FILES
############################

1.File "nodes" :
forwarder1,monitor1
forwarder2,monitor2

2.File "destinations":
forwarder1
forwarder2

3.File "myip":
forwarder0

4.File "ip_adress":
forwarder0
forwarder1
forwarder2

############################
  	  RUNNING
############################

To start regret you need three arguments :

- the name of the file which contains the matrix result (fileMatrix) (it is an ip source / ip destination matrix)
- the name of the file which contains the results of the simulation (fileSimulation)
- the maximum number of iterations you want (nb) /optional : If you do not input nb ./regret will run until the end of fileMatrix

./regret filematrix fileSimulation nb



