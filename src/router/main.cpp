/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file router/main.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "router.h"
#include "cmdline.h"


using namespace std;


void  run_simulation(string & routingManager, int role, size_t maxPathsToTest, string fileName,
		     int & inst, vector<double> & avg_err, MetricType mode, int timeout, int period) {

  Router    router(routingManager, role, maxPathsToTest, mode, timeout, period, fileName);
  
  //run the simulation
  router.simulation(inst, avg_err);
}


// this method performs the simulation for all origin/destination pairs
// For each OD pair, the learning algorithm (routingManager=EXP3,RNN) selects at each measurement epoch
// some paths to monitor. The simulation reads the metrics (latency, bw) of the links in the path in
// the file fileName. It then computes some performance metrics. The interpretation is as follows:
// avg_err[0] is the average relative gap between IP routing and SMART
// avg_err[1] is the average relative gap between IP routing and optimal two-hop routing
// avg_err[2] is the average relative gap between SMART and optimal two-hop routing
// avg_err[3] is the average relative gap between optimal two-hop routing and optimal routing
// avg_err[4] is the frequency at which IP routing differs from optimal two-hop routing
// avg_err[5] is the frequency at which SMART routing differs from optimal two-hop routing

int simulation(string & routingManager, int role, size_t maxPathsToTest, string fileName, MetricType mode, int timeout, int period) {

  IPVector        nodes;
  string          line;
  int             nb_nodes = 0;
  IPv4Address     Addr;
  int             inst = 1;
  int             nb_od = 0;
  int             i;
  vector<double>  avg_err(6,0.0);
  string          err_Names[] = {"Gap IP/SMART", 
				 "Gap IP/TWO HOP", 
				 "Gap SMART/TWO HOP", 
				 "Gap TWO HOP/OPT",
				 "Suboptimality freq. of IP routing", 
				 "Suboptimality freq. of SMART"};

  //first read the file "ip_address" to know the nodes of the overlay
  ifstream    in("ip_address");
  if (in.is_open()){
    while (getline(in, line)) {
      Addr = inet_addr(line.c_str());
      nodes.push_back(Addr);
      nb_nodes++;
    }
    in.close();
  }
  else {
    cerr << "Cannot open file ip_address." << endl;
    return -1;
  }

  //loop on the source node
  for (const IPv4Address src : nodes) {

    //write the IP address of the source node in file "myip"
    ofstream out_src("myip");
    if ( out_src ) {
      out_src << IPToString(src);
      out_src.close(); 
    }
    else {
      cerr << "Cannot open file myip." << endl;
      return -1;
    }

    //loop on the destination node
    for (const IPv4Address dst : nodes) {

      //skip the src node
      if ( dst == src ) continue;

      //write the IP address of the destination node in file "destinations"
      ofstream out_dst("destinations");
      if ( out_dst ) {
	out_dst << IPToString(dst);
	out_dst.close();
      }
      else {
	cerr << "Cannot open file destinations." << endl;
	return -1;
      }

      //run the simulation
      inst = 1;
      cerr << "\nSimulation for " << IPToString(src) << "/" << IPToString(dst) << endl;
      run_simulation(routingManager, role, maxPathsToTest, fileName, inst, avg_err, mode, timeout, period);
      nb_od++;

      //display average results
      for (i=0; i<4; i++)
	cerr << err_Names[i]
	     << " : "
	   << setprecision(4)
	   << scientific
	   << avg_err[i] * 100.0 / ((double) inst) / ((double) nb_od)
	   << " %"
	   << endl;
    }
  }

  //display average results
  cerr << "\n\n--------------------------------------------------" << endl;
  cerr << "Number of origin/destination pairs: " << nb_od << endl;
  for (i=0; i<6; i++) {
    avg_err[i] /= (double) inst;
    avg_err[i] /= (double) nb_od;
    cerr << err_Names[i]
	 << " : "
	 << setprecision(4)
	 << scientific
	 << avg_err[i] * 100.0
	 << " %"
	 << endl;
  }
}




int main (int argc, char *argv[]) {
  string	       CUIP = "";
  string               routingManager = "exp3";
  string               metric = "rtt";
  size_t               maxPathsToTest = 5;
  int                  role = 1;
  MetricType           mode = LATENCY_METRIC;
  gengetopt_args_info  args_info;
  int                  timeout;
  int                  period;

  /* Call the cmdline parser */
  if (cmdline_parser (argc, argv, &args_info) != 0)
	  exit(1) ;

  //read parameters from args_info 
  cout << "SMART Router running..." << endl;
  if ( args_info.number_paths_given )
	  maxPathsToTest = args_info.number_paths_arg;
  cerr << "\tLimit on the number of paths to test at each measurement epoch: " << maxPathsToTest << endl;
  if ( args_info.role_given )
	  role = args_info.role_arg;
  cerr << "\tRole: " << role << endl;
  if ( args_info.algorithm_given )
	  routingManager = string(args_info.algorithm_arg);
  cerr << "\tRouting manager: " << routingManager << endl;
  if ( args_info.metric_given ) {
	  metric = string(args_info.metric_arg);
	  mode = ((metric=="rtt")?LATENCY_METRIC:((metric=="bw")?BW_METRIC:LOSS_METRIC));
  }
  cerr << "\tMETRIC: " << metric << endl;
  if ( args_info.probe_timeout_given )
	  timeout = args_info.probe_timeout_arg * 1000;
  if ( args_info.probe_period_given ) 
	  period = args_info.probe_period_arg;
  cerr << "\tProbe timeout (s) : " << timeout / 1000 << endl;
  cerr << "\tProbe period (s) : " << period << endl;
  if (args_info.cu_ip_given) {
	  CUIP = string(args_info.cu_ip_arg);
  }
  cerr << "\tCU IP: " << CUIP << endl;


  if (args_info.file_given){ // user want to read the result from a file
	  cerr << "Running in simulation mode..." << endl;

	  simulation(routingManager, role, maxPathsToTest, string(args_info.file_arg), mode, timeout, period);
  }
  else {
	  cerr << "Running in live mode..." << endl;

	  Router router(routingManager, role, maxPathsToTest, mode, timeout, period);
	  router.setCUIP(CUIP);
	  router.run();
  }

    return EXIT_SUCCESS;
}


