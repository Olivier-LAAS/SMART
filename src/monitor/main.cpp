
/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/

/**
 * @file monitor/main.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */
#include <iostream>
#include <cstdlib>
#include "monitor.h"
#include "Probe.h"
//#include "LatencyProbe.h"
#include "BWProbe.h"
#include "LossProbe.h"
#include "cmdline.h"

using namespace std;


int main (int argc, char *argv[]) {
 string 		routerIP = "";
 string 		localIP = "";
 string 		metric = "rtt";
 MetricType	mode=LATENCY_METRIC;

 gengetopt_args_info  args_info;

 /* Call the cmdline parser */
 if (cmdline_parser (argc, argv, &args_info) != 0)
	exit(1) ;
 cerr << "SMART Monitor running..." << endl;

 //read parameters from args_info
 if ( args_info.router_ip_given && args_info.local_ip_given ){
	routerIP = string(args_info.router_ip_arg);
	localIP = string(args_info.local_ip_arg);	
 }
 cerr << "\tRouter IP : " << routerIP << ", Local IP : " << localIP << endl;
 if ( args_info.metric_given ){
	metric = string(args_info.metric_arg);
	mode = ((metric=="rtt")?LATENCY_METRIC:((metric=="bw")?BW_METRIC:LOSS_METRIC));
 }
 cerr << "\tMETRIC: " << mode << endl;

 MonitorManager manager(routerIP, localIP);


	 //Add the Probe agent measuring latencies
	 Probe Probe(&manager, mode);
	 Probe.startForwarder(); // start the latencyProbe forwarder
	 manager.addProbeAgent(&Probe);
	 manager.main(mode);


 /*if (mode == LATENCY_METRIC){
	 //Add the Probe agent measuring latencies
	 ProbeLatency latencyProbe(&manager);
	 latencyProbe.startForwarder(); // start the latencyProbe forwarder
	 manager.addProbeAgent(&latencyProbe);
	 manager.main(mode);
 }
 else if (mode == BW_METRIC){
	 //Add the Probe agent measuring BW
	 ProbeBW bwProbe(&manager);
	 bwProbe.startForwarder(); // start the bwProbe forwarder
	 manager.addProbeAgent(&bwProbe);
	 manager.main(mode);
 }
 else
 {
	 //Add the Probe agent measuring losses
	 ProbeLoss lossProbe(&manager);
	 lossProbe.startForwarder(); // start the lossProbe forwarder
	 manager.addProbeAgent(&lossProbe);
	 manager.main(mode);
 }*/

 return EXIT_SUCCESS;
}

