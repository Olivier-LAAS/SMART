#!/bin/bash

# -C : CU IP address, -a : algorithm, -K : max paths, -m : rtt, bw or loss, -r : role number, --probe-timeout : duration of measurement (s), --probe-period : total duration of a measurement cycle (s) 
(./router -C <cu_ip> -aexp3 -K2 -mrtt -r1 --probe-timeout=<timeout> --probe-period=<period> ) & 
# modify sleep if necessary, all routers must be run before monitors and forwarders
sleep  10
# put ip address of router (above) as argument + local ip (ip of machine where monitor is launched, in this case the same as router)
# -R : router IP, -L : loca IP, -m : rtt, bw or loss 
# For the moment the 2 last option are unavailable
(./monitor -R <routerip> -L <monitorip> -m <mode> &> /dev/null) &
sleep .5
(sudo nice --20 ./forwarder <routerip> >& /dev/null)&
