/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file LatencyProbe.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 3 Apr 2015
 * @brief
 *
 */

#ifndef LATENCYPROBE_H
#define LATENCYPROBE_H

#include "monitor.h"


#define LATENCY_PROBE_PORT 9547				//! PORT used for the transfer of latency probe packets

/*!
 * @brief The header of a probe packet
 * @param pathLength The size of a path
 * @param nextHop the index of next hop
 * @param nextStamp the index of stamps
 * @param direction 0 for forward and 1 for backward
 * @param path path nodes addresses + source
 * @param stamps two stamps (forward and backward) per node + source
 */
struct LatencyProbeHeader {
    unsigned int pathLength :DATA_MAX_PATH_LENGTH; /*!before it was 4*/
    unsigned int nextHop :DATA_MAX_PATH_LENGTH;/*! To be verified, before it was 4*/
    unsigned int nextStamp :DATA_MAX_PATH_LENGTH * 2; /*! To be verified, before it was 4*/
    unsigned int direction; /*! forward or backward (before it was 4)*/
    IPv4Address path[DATA_MAX_PATH_LENGTH + 1]; /*! +1 for the source address*/
    struct timeval stamps[(2 * (DATA_MAX_PATH_LENGTH + 1)) - 1];
};


/**
 * @brief The probe packet structure
 */
class LatencyProbePacket : public Packet {
public :
        LatencyProbePacket();

        /**
         * @brief Get a pointer to the ProbeHeader
         * @return
         */
        struct LatencyProbeHeader * getLatencyProbeHeader() const;
};


class ProbeLatency;

/**
 * @brief Forwarder dedicated to latency measurement.
 * Implements the ForwarderInterface
 */
class ProbeLatencyForwarder : public ForwarderInterface {
public :
    ProbeLatencyForwarder();

    /**
     * @brief Loop waiting for packets to forward.
     * Forward the packets according to their state.
     */
    void process();

    /**
     * @brief Set the latency probe agent associated with this forwarder
     * @param agent pointer to the latency probe agent
     */
    void setProbeAgent(ProbeLatency* agent) {this->agent = agent;}

private:
    //! Stores the current packet
    LatencyProbePacket packet;

    /**
     * @brief pointer to the latency probe agent
     */
    ProbeLatency* agent;
};


/**
 * @brief Probe agent dedicated to latency measurement
 * Implements the ProbeInterface
 */
class ProbeLatency : public ProbeInterface {
public:
    ProbeLatency(MonitorManager *  monitorManager);

    /**
     * @brief Starts the forwarder thread
     */
    virtual void startForwarder();
    virtual void doMonitorPaths(const std::vector<IPPath> & routes);

    /**
     * @brief Send the measurements results to the monitorManager
     * @param route
     * @param latencies
     */
    void sendResultsToMonitorManager(const IPPath & route, std::vector<double> latencies);

private:
    /**
     * @brief The latency forwarder agent
     */
    ProbeLatencyForwarder forwarder;

    /**
     * @brief the thread that runs the forwarder
     */
    std::thread forwarderThread;

    /**
     * @brief Pointer to the MonitorManager
     */
    MonitorManager * monitorManager;

    /**
     * @brief Socket used to send the probe packets
     */
    int socketDescription;
};

#endif // LATENCYPROBE_H
