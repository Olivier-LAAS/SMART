/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RoutingManagerInterface.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 5 Apr 2015
 * @brief
 *
 */

#ifndef ROUTINGMANAGERINTERFACE_H
#define ROUTINGMANAGERINTERFACE_H

#include <vector>
#include "common/common.h"


using namespace std;

class PathManager;
class SimulationManager;

/*!
 * @brief A pure virtual class. RoutingManager derives from this class, and all algorithms derive from RoutingManager
 * @param myMonitorIP the IP address of the Monitor
 * @param index_nodes this vector contains all (SMART) monitor addresses
 * @param index_destinations this vector contains only destination forwarder IP addresses.
 */
class RoutingManagerInterface{
public:

    void setPathManager(PathManager* manager) { if (manager) pathManager = manager;}

    void setSimulationManager(SimulationManager* simulation) { if(simulation) simulationManager = simulation; }

    /**
     * @brief Manage the whole measurement process
     */
    virtual void run() = 0;

    virtual void runSimulation(int & inst, vector<double> & avg_err) = 0;

    /**
     * @brief setLatencyProbeResults (provided by Monitor Manager)
     */
    virtual void putProbeResults(const PathEdgesResult & result) = 0;

    IPv4Address myMonitorIP;
    /**
     * Should disappear. Actually used only by simulation
     */
    IPv4Address origin;
    IPVector index_origin;

    IPVector index_nodes;          /**< contains the hops */
    IPVector index_destinations;   /**< contains the destinations */

protected:
    PathManager       * pathManager;
    SimulationManager * simulationManager;
    MetricType          mode_;
};

#endif // ROUTINGMANAGERINTERFACE_H
