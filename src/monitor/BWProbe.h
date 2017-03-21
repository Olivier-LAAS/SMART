/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/


#ifndef BWPROBE_H
#define BWPROBE_H

#include "monitor.h"

#define BW_PROBE				//! Tell that a loss probe is available 

// The port number may need to be changed, this one is the Latency port
#define BW_PROBE_PORT 9547				//! PORT used for the transfer of BW probe packets
#define IPERF_PORT 5201				//! PORT used for iperf

#define IPERF_FAIL_SLEEP_IN_MS 400 //! if iperf test fails then sleep
#define WAIT_RESULT_SLEEP_IN_MS 400 //! if probe is waiting for iperf to finish test
#define WAIT_RESULT_TIMEOUT_IN_MS 20000 //! if probe is waiting for iperf to finish test
#define IPERF_BUSY_SERVER_TIMEOUT_IN_MS 120000 //! if probe is waiting for iperf to finish another test
#define MAX_FRESHNESS_IN_MS 130000 //! freshness of result


/**
 * @brief Probe agent dedicated to BW measurement
 * Implements the ProbeInterface
 */
class ProbeBW { // : public ProbeInterface {
public:
    ProbeBW();

    /**
     * @brief Starts the iperf client
     */
    void runIperfServer();

    /**
     * @brief Launch a BW test using iperf with destination address
     * @param destAdd ip address of destination node
     */
    double getBw(struct sockaddr_in *add);

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

#endif // BWPROBE_H
