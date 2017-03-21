/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file router.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include "router.h"

#include "RoutingManagerEXP3.h"
#include "RNN.h"



Router::Router(string askedAlgo, int role, size_t maxPathsToTest, MetricType mode, int timeout, int period, string fileName) : simulationManager(fileName), daemon(role){
  if (askedAlgo=="exp3") {
    routingManager = new RoutingManagerEXP3(maxPathsToTest, mode);
    cout << "Using EXP3 probe algorithm" << endl;
  } else if (askedAlgo=="rnn") {
    routingManager = new RNN(maxPathsToTest, mode);
    cout << "Using RNN probe algorithm" << endl;
  } else {
    cerr << askedAlgo << " is not a valid algorithm" << endl;
    cerr << "Valid algorithms are : " << "'exp3', 'rnn' " << endl;
    exit(-1);
  }
  
  // Set the probe timers
  pathManager.setProbeTimeout(timeout);
  pathManager.setProbePeriod(period);
}

void Router::run() {
    routingManager->setPathManager(&pathManager);
    daemon.setRoutingManager(routingManager);
    daemon.setPathManager(&pathManager);
    pathManager.setRoutingManager(routingManager);

    // starts the daemon thread (return when the configuration has been received)
    daemonThread = thread(&DaemonManager::CUCommunication, &daemon);

    // starts the pathManager threads
    pathManager.start();

    while (daemon.connectCU == false) {
    	this_thread::sleep_for(chrono::seconds(2));
    }

    // wait 3 second while the path manager init all its connections and threads
    this_thread::sleep_for(chrono::seconds(3));

    // launch the measurement loop
    cout << "Starts measurement loop" << endl;
    routingManager->run();
}

void Router::simulation(int & inst, vector<double> & avg_err) {

  routingManager->setSimulationManager(&simulationManager);
  simulationManager.setRoutingManager(routingManager);

  cout << "Simulation running..." << endl;
  simulationManager.readListIP();
  simulationManager.readConfigurationFromFile();
  routingManager->runSimulation(inst, avg_err);
}

Router::~Router() {
    delete routingManager;
}
