/****************************************************************************
 * Panacea Overlay Network v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of PON (Panacea Overlay Network)                       *
 *                                                                          *
 * PON is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file SimulationManager.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include <stdio.h>
#include <iomanip>
#include <math.h>
#include "SimulationManager.h"

#define MIN(x,y)  ((x<y)?(x):(y))


SimulationManager::SimulationManager(string fileName) : file(fileName){
    rows=0;
    cols=0;
	if (!file.is_open()){
		cout << "file open failed" << endl;
	}
	endMatrixResult = false;
	inst=1;
	reward=0.0;
}

SimulationManager::~SimulationManager(){
	file.close();
}


int SimulationManager::readNumbers( const string & s ) {
	istringstream is( s );
    double n;
    while( is >> n ) {
        matrix.push_back( n );
    }
    return matrix.size();
}



int SimulationManager::readResultsFromFile(){
  string line;

  int i=0;
  getline(file, line );

  cols =readNumbers( line );
  //cout << "cols:" << cols << endl;
  //cout << "line:" << line << endl;
  
  
  for ( i=1;i<dim;i++){
    if ( getline(file, line) == 0 ) {
      return 0;
    }
    readNumbers( line );
  }
  rows=i;
  return 1;
    //cout << "rows :" << rows << endl;
}

void SimulationManager::writeMatrice(){
	cout << "Matrix:" << endl;
    for (int i=0;i<rows;i++){
        for (int j=0;j<cols;j++){
        	cout << matrix[i*cols+j] << "\t" ;
        }
        cout << endl;
    }
}


void SimulationManager::readListIP(){
  string    line;
  ifstream  in("ip_address");
  int       dimension = 0;

  if (in.is_open()){
    while (getline(in, line)) {
      IPv4Address Addr = inet_addr(line.c_str());
      tableIP.push_back(Addr);
      id_[Addr] = dimension;
      routingManager->index_nodes.push_back(Addr);
      dimension ++;
    }
    dim = dimension;
    in.close();
  }
}

void SimulationManager::readConfigurationFromFile(){
  cout << "Reading configuration from files..." << endl;
  string line;

  ifstream in("destinations");
  if (in.is_open()) {
    routingManager->index_destinations.clear();
    while (getline(in, line)) {
      // gives the destination address to the routingManager
      cerr << "Adding " << line << " as a new destionation" << endl;
      routingManager->index_destinations.push_back(inet_addr(line.c_str()));
    }
    in.close();
  }

  in.open("myip");
  if (in.is_open()) {
    routingManager->index_origin.clear();
    while (getline(in, line)) {
      // gives the destination address to the routingManager
      cerr << "Adding " << line << " as a new origin" << endl;
      routingManager->index_origin.push_back(inet_addr(line.c_str()));
    }
    in.close();
    //getline(in, line);
    // origin = inet_addr(line.c_str());
    // in.close();
  }
}


void SimulationManager::lookUpForResult(){

  PathEdgesResult result;
  int x, y;
  double metric;

  //copy the path in result.path
  for (size_t i = 0; i< monitoringPacket.pathLength; i++) 
    result.path.push_back(monitoringPacket.path[i]);

  //set result.metric
  result.metric = monitoringPacket.metric;

  //initialize the path metric with the metric from origin to first intermediate node
  metric = matrixValue(routingManager->origin, monitoringPacket.path[0]);
  result.edgesRTT.push_back(metric);

  for(int i=0; i<monitoringPacket.pathLength-1;i++){
    metric = matrixValue(monitoringPacket.path[i], monitoringPacket.path[i+1]);
    result.edgesRTT.push_back(metric);
  }

  this->routingManager->putProbeResults(result);
}


int SimulationManager::run(MetricType metric, vector<IPPath> paths){

  int n = 1;

  //  if ( metric == LATENCY_METRIC )
    //    cerr << "Optimizing RTT" << endl;
  //  else
  //    cerr << "Optimizing Bw" << endl;
  //  cerr << "Number of paths : " << paths.size() << endl;

  for (const IPPath & path : paths) {

    monitoringPacket.metric = metric;
    //    cerr << "Building monitoring pck for path " << n << " : ";
    for (size_t i=0; i< path.size(); i++) {
      monitoringPacket.path[i] = path.at(i);
      //      cerr << IPToString(path[i]) << " ";
    }
    //    cerr << endl;

    monitoringPacket.pathLength = path.size();
    lookUpForResult();
    n++;
  }
  matrix.clear();
  //  cerr << "Fin run" << endl;
  return 1;
}

double SimulationManager::calculateLatency(MetricType metric, IPPath path){

  double pathCost, value;
  int x, y;

  //initialize pathCost with the metric from origin to first intermediate node
  pathCost = matrixValue(routingManager->origin, path.at(0));
  
  //Loop on the intermediate nodes in the path
  for(int i=0; i<path.size()-1;i++) {
    
    //get the metric between i and i+1
    value = matrixValue(path.at(i), path.at(i+1));
    
    //if latency, add this to pathCost, otherwise take the min
    if(metric==LATENCY_METRIC)
      pathCost += matrixValue(path.at(i), path.at(i+1));
    else if(metric == BW_METRIC) {
      if( value < pathCost )
	pathCost = value;
    }
  }

  return pathCost;
}


void SimulationManager::writeResultInFile(MetricType mode, map<IPv4Address, IPPath> bestPath, int time){

  double      ip, metric, opt, opt_two_hops;
  ofstream    resultText("simulationResult.txt", ios::app);

  if(resultText){

    resultText.precision(10);


    map< IPv4Address, IPPath>::iterator it = bestPath.begin();
    for ( ; it != bestPath.end(); ++it){

      const IPv4Address theDestination = it->first;
      const IPv4Address theSource = routingManager->origin; 
      const IPPath      thePath = it->second;

      

      string  name_destination = IPToString(theDestination);
      string  name_source = IPToString(theSource);
      string  ori_dest = name_source+"_"+name_destination;
      
      resultText << time << "  ";
      ip = matrixValue(theSource,theDestination);
      metric = calculateLatency(mode, thePath);
      opt_two_hops = compute_optimal_two_hop_distance(theSource, theDestination, mode);
      opt = compute_optimal_distance(theSource, theDestination, mode);
      resultText << ip << "\t" << metric  << "\t"  << opt << "\t" << opt_two_hops << endl;
    }
  }
}


void SimulationManager::getError(MetricType mode,
				 IPv4Address & src,
				 IPv4Address & dst,
				 IPPath & path, 
				 vector<double> & errors) {

  double      ip, metric, opt, opt_two_hops;

  ip = matrixValue(src,dst);
  metric = calculateLatency(mode, path);
  opt_two_hops = compute_optimal_two_hop_distance(src, dst, mode);
  opt = compute_optimal_distance(src, dst, mode);
  errors[0] = fabs(ip-metric)/metric;
  errors[1] = fabs(ip-opt_two_hops)/opt_two_hops;
  errors[2] = fabs(metric-opt_two_hops)/opt_two_hops;
  errors[3] = fabs(opt_two_hops-opt)/opt;
}






void SimulationManager::writeLenght(){
  ofstream resultText("len.txt", ios::app);

  if(resultText){
    resultText.precision(12);
    
    resultText << reward << endl;
  }
}


//----------------------------------------------------------------//                                                                                                                                       
// Public Method: compute_optimal_two_hop_distance                //                                                                                                                                       
// compute the length of the optimal 2-hop path from i to j       //                                                                                                                                       
//----------------------------------------------------------------//

double SimulationManager::compute_optimal_two_hop_distance(IPv4Address src, IPv4Address dst, MetricType mode) {
  int    i = id_[src];
  int    j = id_[dst];
  int    k;
  double opt = matrixValue(i,j);
  double length;

  //consider all nodes k different from i and j
  for ( k=0; k<dim; k++) {
    if ( (k==i) || (k==j) ) continue;

    if ( mode == LATENCY_METRIC ) { //find minimum latency path 
      length = matrixValue(i,k) + matrixValue(k,j);
      if ( length < opt )
	opt = length;
    }
    else if ( mode == BW_METRIC) {
      length = MIN(matrixValue(i,k), matrixValue(k,j));
      if ( length > opt )
	opt = length;
    }
  }

  return opt;
}


//----------------------------------------------------------------// 
// Public Method: compute_optimal_distance                        //
// returns the length of the optimal path from i to j             //
//----------------------------------------------------------------//

double SimulationManager::compute_optimal_distance(IPv4Address src, IPv4Address dst, MetricType mode) {
  int             i = id_[src];
  int             j = id_[dst];
  int             n, k, h;
  double          x;
  vector<double>  distance(dim);

  //initialization
  for (k=0; k<dim; k++) 
    distance[k] = ((mode==LATENCY_METRIC)?1.0e10:0.0);
  distance[j] = ((mode==LATENCY_METRIC)?0.0:1.0e10);

  //Bellman-Ford algorithm
  for (n=0; n<dim; n++) {
    for (k=0; k<dim; k++) {
      for (h=0; h<dim; h++) {
        if ( mode == LATENCY_METRIC ) { //LATENCY_METRIC
          x = matrixValue(k,h) + distance[h];
          if ( x < distance[k] ) 
            distance[k] = x;
        }
        else { //BANDWIDTH_METRIC
          x = MIN(matrixValue(k,h), distance[h]);
          if ( x > distance[k] ) 
            distance[k] = x;
        }
      }
    }
  }

  return distance[i];
}
