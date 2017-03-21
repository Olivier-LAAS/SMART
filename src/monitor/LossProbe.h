/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/


#ifndef LOSSPROBE_H
#define LOSSPROBE_H

#include "monitor.h"

#define LOSS_PROBE				//! Tell that a loss probe is available

// The port number may need to be changed, this one is the Latency port
#define BW_PROBE_PORT 9547				//! PORT used for the transfer of BW probe packets
#define IPERF_PORT 5201				//! PORT used for iperf

#define IPERF_FAIL_SLEEP_IN_MS 400 //! if iperf test fails then sleep
#define WAIT_RESULT_SLEEP_IN_MS 400 //! if probe is waiting for iperf to finish test
#define WAIT_RESULT_TIMEOUT_IN_MS 20000 //! if probe is waiting for iperf to finish test
#define IPERF_BUSY_SERVER_TIMEOUT_IN_MS 120000 //! if probe is waiting for iperf to finish another test
#define MAX_FRESHNESS_IN_MS 130000 //! freshness of result


/**
 * @brief Probe agent dedicated to Loss measurement
 * Implements the ProbeInterface
 */
class ProbeLoss {// : public ProbeInterface {
public:
    ProbeLoss();

    /**
     * @brief Starts the iperf client
     */
    void runIperfServer();

    /**
     * @brief Launch a Loss test using iperf with dertination address
     * @param destAdd ip address of destination node
     */
    double getLoss(struct sockaddr_in *add);	

private:
    /**
     * @brief the thread that runs the iperf server
     */
    std::thread iperfServerThread;

    /**
     * @brief freshness of bw test to dest nodes
     */
    std::map<IPv4Address, timeval> iperfTestFreshness;
    
    /**
     * @brief last bw results
     */
    std::map<IPv4Address, double> iperfResults;
    
};

#endif // LOSSPROBE_H
