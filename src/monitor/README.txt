This folder contains the monitor code

########################
	MONITOR	
########################

The monitor contains two distinct entities:
- Monitor Manager (monitor.h, monitor.cpp). It pilots the communication with Probes and the router
- Probes (ProbeInterface.h, LatencyProbe.h,cpp, LossProbe.h,cpp....)
Probes are independent. Each probe performs one kind of measurements. It only communicates with Monitor Manager.


########################
	RUNNING	
########################

To execute monitor you could passe routerip as an argument otherwise you should provide it in a configuration file

./monitor -R <routerip> -L <monitorip> -m <mode>

- L : the monitor IP is usually the same as the router because they run on the same machine.
- m (option) : can be rtt (default), bw or loss.

