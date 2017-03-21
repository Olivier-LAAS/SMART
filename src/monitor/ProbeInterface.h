/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file ProbeInterface.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 3 Apr 2015
 * @brief
 *
 */

#ifndef PROBEINTERFACE_H
#define PROBEINTERFACE_H

#include "common/common.h"

/**
 * @brief Defines the interfaces that each probe agent must implement
 */
class ProbeInterface {
public:

    /**
     * @brief Constructs the probe agent
     * @param metric the metric associated with this agent
     */
    ProbeInterface(MetricType metric) : metric(metric) {}

    /**
     * @brief Get the kind of metric associated with this agent
     * @return the metric
     */
    MetricType getMetric() const { return metric;}

    /**
     * @brief Start the forwarder required by the agent
     */
    virtual void startForwarder() = 0;

    /**
     * @brief Start measurements on a list of routes
     * @param routes the routes to measures
     */
    virtual void doMonitorPaths(const std::vector<IPPath> & routes) = 0;


protected:
    /**
     * @brief the kind of metric associated with the probe agent
     */
    MetricType metric;
};



#endif // PROBEINTERFACE_H
