/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/



#ifndef PROBE_H
#define PROBE_H

#include "monitor.h"
#include "LossProbe.h"
#include "BWProbe.h"
//#include "LatencyProbe."


#define PROBE_PORT 9547			//! PORT used for the transfer of probe packets

/*!
 * @brief The header of a probe packet
 * @param pathLength The size of a path
 * @param nextHop the index of next hop
 * @param nextStamp the index of stamps
 * @param direction 0 for forward and 1 for backward
 * @param path path nodes addresses + source
 * @param stamps two stamps (forward and backward) per node + source
 */
struct ProbeHeader {
    unsigned int pathLength :DATA_MAX_PATH_LENGTH; /*!before it was 4*/
    unsigned int nextHop :DATA_MAX_PATH_LENGTH;/*! To be verified, before it was 4*/
    unsigned int nextStamp :DATA_MAX_PATH_LENGTH * 2; /*! To be verified, before it was 4*/
    unsigned int direction; /*! forward or backward (before it was 4)*/
    IPv4Address path[DATA_MAX_PATH_LENGTH + 1]; /*! +1 for the source address*/
    double stamps[DATA_MAX_PATH_LENGTH]; // for a path only one measure per node, except last node on path as reverse value is not necessary TO VERIFY
};


/**
 * @brief The probe packet structure
 */
class ProbePacket : public Packet {
public :
        ProbePacket();

        /**
         * @brief Get a pointer to the ProbeHeader, the load of the probe packet is entirely defined by the structure
	 * probhdr, so the attribute probeheader will point to the beginning of the message.
         * @return
	 */
        struct ProbeHeader * getProbeHeader() const {return (ProbeHeader *) (msg);}
};


class Probe;

/**
 * @brief Forwarder dedicated to probe measurement.
 * Implements the ForwarderInterface
 */
class ProbeForwarder : public ForwarderInterface {
public :
    ProbeForwarder();

    /**
     * @brief Loop waiting for packets to forward.
     * Forward the packets according to their state.
     */
    void process();

    /**
     * @brief Set the probe agent associated with this forwarder
     * @param agent pointer to the probe agent
     */
    void setProbeAgent(Probe* agent) {this->agent = agent;}

private:
    //! Stores the current packet
    ProbePacket packet;

    /**
     * @brief pointer to the probe agent
     */
    Probe* agent;
};


/**
 * @brief Probe agent dedicated to measurement
 * Implements the ProbeInterface
 */
class Probe : public ProbeInterface {
public:
    Probe(MonitorManager *  monitorManager, MetricType metric_);

    ~Probe();    

    /**
     * @brief Starts the forwarder thread
     */
    virtual void startForwarder();
    virtual void doMonitorPaths(const std::vector<IPPath> & routes);

    /**
     * @brief Send the measurements results to the monitorManager
     * @param route
     * @param measure 
     */
    void sendResultsToMonitorManager(const IPPath & route, std::vector<double> measure);

    /**
     * @brief Get the kind of metric associated with this agent
     * @return the metric
     */
//    MetricType getMetric() const { return metric;}

    /**
     * @brief Launch a measure using destination address
     * @param destAdd ip address of destination node
     */
    double getMeasure(struct sockaddr_in* add);

private:
    /**
     * @brief The forwarder agent
     */
    ProbeForwarder forwarder;

    /**
     * @brief the thread that runs the forwarder
     */
    std::thread forwarderThread;

    /** 
     * @brief the thread that runs the selected probe
     */
    std::thread probeThread;

    /**
     * @brief Pointer to the MonitorManager
     */
    MonitorManager * monitorManager;

    /**
     * @brief Socket used to send the probe packets
     */
    int socketDescription;

    /**
     * @brief Type of measurement
     */
    MetricType metric;
    
    /** TODO:keep?
     * @brief freshness of test to dest nodes
     */
//    std::map<IPv4Address, timeval> iperfTestFreshness;
   
#ifdef BW_PROBE
    ProbeBW* probeBW;
#endif

#ifdef LATENCY_PROBE
    ProbeLatency* probeLatency;
#endif

#ifdef LOSS_PROBE
    ProbeLoss* probeLoss;
#endif
 
};


#endif // PROBE_H
