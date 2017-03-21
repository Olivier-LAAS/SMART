/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file monitor.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */
#ifndef MONITOR_H_
#define MONITOR_H_

#include "common/common.h"
#include "ProbeInterface.h"



/**
 * @brief MonitorManager in charge of communication with each probe agent, and the PathManager
 */
class MonitorManager {
public:
    /**
     * @brief Construct the MonitorManager.
     */
    MonitorManager(std::string routerip=" ", std::string monitorip=" ");

    /**
     * @brief Wait for PathManager instructions, which are send on a TCP port.
     * 
     * @param  metric the kind of metric choose
     */
    void main(MetricType mode);

    void readConfigurationFromFile();

    /**
     * @brief Ask the user for a path to test (test function, to be removed later)
     * @return
     */
    std::vector<IPPath> readPathsFromUserCommandLine();

    /**
     * @brief Set the monitoring results for the specified paths.
     *        This function is supposed to be called by the probe agents when they finish their measurements.
     * @param metric the kind of metric that was measured
     * @param paths the paths that were monitored
     */
    void putProbeResults(MetricType metric,const IPPath &path, const std::vector<double> & results);


    /**
     * @brief Add a probe agent to the MonitorManager
     * @param agent a pointer to the probe agent
     */
    void addProbeAgent(ProbeInterface * agent);

    /**
     * @brief Set the IP address of the local machine (used for configuration by the PanaceaPathManager)
     * @param ip the IPv4Address
     */
    void getIPadress();
    IPv4Address getMyAddr();
    IPv4Address getRouterIPAddr() const;

private:
    /**
     * @brief The list of probeagents available
     * Each kind of metric is associated with a single agent
     */
    std::map<MetricType, ProbeInterface* > probeAgents;

    /**
     * @brief the local machine IP address
     */
    IPv4Address myIPAddr;

    IPv4Address routerIPAddr;

    int socketRouter;

    /**
     * @brief Launch the path monitoring with the corresponding probe agent.
     *        This function is used when path monitoring instructions are received from the PathManager
     * @param metric the kind of metric to measure
     * @param paths the paths to monitor
     */
    void doMonitorPaths(MetricType metric, std::vector<IPPath> & paths);

    void connectToRouter();
};


#endif
