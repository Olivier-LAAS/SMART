/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file PathManager.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 5 Apr 2015
 * @brief
 *
 */

#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include "common/common.h"
#include "RoutingManagerInterface.h"

/*!
 * @brief This class takes in charge all communications between Router -> Monitor, Router->Forwarder
 * @function setMonitorRoute Sends the corresponding forwarder route to the forwarder
 * @function doMonitorPaths Launch paths measurement
 * @param forwarderFromMonitor Contains the forwarder IP associated with monitor. Provides Forwarder IP address by Monitor IP address
 * @param monitorFromForwarder Contains the monitor IP associated with forwarder. Provides Monitor IP address by Forwarder IP address
 * @param forwarderIPAddr The IP address of the forwarder
 * @param monitorManagerIPAddr the IP address of the monitor
 * @param transmissionAgentIP the IP address of the last connected TA (it is used by the CU configuration mechanism)
 * @param mapClouds for each destination forwarder the list of TA behind
 * @param socketsTAs for each TA the number of socket associated with (used for interception purposes)
 *
 */
class PathManager  {
public:
    PathManager() : isForwarderConnected(false), isMonitorConnected(false), isTransmissionAgentConnected(false) {}

    // Configuration
    void setRoutingManager(RoutingManagerInterface *  routingManager) {this->routingManager = routingManager;}

    /**
     * @brief Start all the manager threads (communications with the other agents)
     * Returns when the configuration has been received from the PanaceaManager
     */
    void start();

    /**
     * @brief Set the best route obtained from monitor.
     * Send the corresponding forwarder route to the forwarder
     */
    void setMonitorRoute(IPv4Address destinationMonitor, IPRoute monitorRoute);

    /**
     * @brief Launch paths measurement
     * @param metric the metric that will be measured
     * @param paths the paths to explore
     */
    void doMonitorPaths(MetricType metric, std::vector<IPPath> paths);

    /**
     * @brief Set the probe timer.
     */
    void setProbeTimeout(int timeout){this->probeTimeout = timeout;}
    void setProbePeriod(int period){this->probePeriod = period;}

    std::map<IPv4Address, IPv4Address> forwarderFromMonitor;
    std::map<IPv4Address, IPv4Address> monitorFromForwarder;

    IPv4Address forwarderIPAddr;
    IPv4Address monitorManagerIPAddr;

    /**
     * Variables for probes
     */
    int probeTimeout;
    int probePeriod;  

    /**
     * These variables concern CU mechanism
     */
    IPv4Address transmissionAgentIP;
    bool isForwarderConnected;
    bool isMonitorConnected;
    bool isTransmissionAgentConnected;

    std::map<IPv4Address, std::vector<IPv4Address> > mapClouds;
    std::map<IPv4Address, int> socketsTAs;    /**< socket to send notifications to clients */
private:

    RoutingManagerInterface * routingManager;

    /*******************************************************
     *  Communications with Forwarder (for routing table updates)
     *******************************************************/
    int socketForwarder;
    std::thread forwarderCommunicationThread;
    void waitForForwarderConnection();
    void sendRouteToForwarder(IPv4Address destinationFowarder, IPRoute forwarderRoute);

    /*******************************************************
     *  Communications with MonitorManager
     *******************************************************/
    int socketMonitorManager;
    std::thread monitorCommunicationsThread;
    void monitorCommunications();

    /*******************************************************
     *  Communications with TransmissionAgent
     *******************************************************/
    int socketTAServer;
    std::thread TACommunicationsThread;
    void waitForTransmissionAgentConnection();
    /**
     * @brief sendInterceptionNotification
     * @param distantForwarder
     * @param doInterception
     */
    void sendInterceptionNotification(IPv4Address distantForwarder, bool doInterception, std::vector<IPv4Address> dest);


    // configuration functions (may be useful when having multiple forwarders per Monitor/Router)
    void addForwarderIP(IPv4Address ip);
    void addProbeIP();
    void addTransmissionAgent(IPv4Address ip);

};

#endif // PATHMANAGER_H
