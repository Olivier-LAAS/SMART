/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file LatencyProbe.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 3 Apr 2015
 * @brief
 *
 */

#include "LatencyProbe.h"


LatencyProbePacket::LatencyProbePacket() : Packet() {
}

/*!
 * The load of the probe packet is entirely defined by the structure
 * probhdr, so the attribute probeheader will point to the beginning of the message.
 */
struct LatencyProbeHeader * LatencyProbePacket::getLatencyProbeHeader() const {
    return (LatencyProbeHeader *) (msg);
}



ProbeLatency::ProbeLatency(MonitorManager * monitorManager) : ProbeInterface(LATENCY_METRIC)
{
    this->monitorManager = monitorManager;
    forwarder.setProbeAgent(this);

    socketDescription = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketDescription == -1) {
        std::cout << "Error creating UDP socket\n" << std::endl;
        exit(1);
    }
}


void ProbeLatency::startForwarder()
{
    // start the forwarder process() function in a separated thread
    forwarderThread = std::thread(&ProbeLatencyForwarder::process, forwarder);
}

void ProbeLatency::doMonitorPaths(const std::vector<IPPath> &routes)
{
    LatencyProbePacket packet;

    struct sockaddr_in add;   /**< the address */
    bzero(&add, sizeof(add));
    add.sin_family = AF_INET;
    add.sin_port = htons(LATENCY_PROBE_PORT);

    for (const IPPath& route : routes) {
        memset(packet.msg, 0, BUFLENGTH);
        int pathLength = route.size() + 1;
        LatencyProbeHeader* header = packet.getLatencyProbeHeader();

        // copy the route to the network packet
        header->pathLength = pathLength;
        header->path[0] = monitorManager->getMyAddr();
        for (size_t ii = 0; ii < route.size(); ii++) {
            header->path[ii+1] = route[ii];
        }

        // send PROBE_PACKETS_TO_SEND probe packets
        header->direction = 0;
        header->nextHop = 1;
        header->nextStamp = 2;
        add.sin_addr.s_addr = header->path[header->nextHop];
        for (int j = 0; j < PROBE_PACKETS_TO_SEND; j++) {
            struct timeval start;
            gettimeofday(&start, NULL);
            header->stamps[0] = start;

            if (sendto(socketDescription, (char*) packet.msg, sizeof(LatencyProbeHeader), 0, (struct sockaddr*) &add,  sizeof(add)) < 0) {
                perror("failed to send the latency probe packet");
            }
        }
    }
}

void ProbeLatency::sendResultsToMonitorManager(const IPPath &path, std::vector<double> latencies)
{
	monitorManager->putProbeResults(LATENCY_METRIC, path, latencies);
}


// Creates a UDP socket on port 'LATENCY_PROBE_PORT'
ProbeLatencyForwarder::ProbeLatencyForwarder() : ForwarderInterface(LATENCY_PROBE_PORT)
{
}


void ProbeLatencyForwarder::process()
{
    struct sockaddr_in nextForwarderAddr;
    bzero(&nextForwarderAddr, sizeof(sockaddr));
    nextForwarderAddr.sin_family = AF_INET;
    nextForwarderAddr.sin_port = htons(LATENCY_PROBE_PORT);

    std::cout << "Latency probe forwarder running..." << std::endl;

    while (1) {
        // wait for incoming probe packet
        packet.lenght = recvfrom(socketDescription, packet.msg, sizeof(LatencyProbePacket), 0, NULL, NULL);

        //! defines if the packet has came back to its source
        bool reached = false;
        LatencyProbeHeader * header = packet.getLatencyProbeHeader();
        struct timeval stamp;

        if (header->nextHop < header->pathLength-1) { // -1 to get rid of the source
            if (header->direction == 0) {
                // forward
                std::cout << "forward" << std::endl;
                header->nextHop++;
                nextForwarderAddr.sin_addr.s_addr =	header->path[header->nextHop];
                gettimeofday(&stamp, NULL);
                header->stamps[header->nextStamp] = stamp;
                header->nextStamp += 2;
            } else {
                //backward
                if (header->nextHop == 0) {
                    // if the packet has came back to its source, get the final result
                    gettimeofday(&stamp, NULL);
                    header->stamps[header->nextStamp] = stamp;

                    // extract each edge latency
                    std::vector<double> latencies;
                    IPPath path;
                    int i = header->pathLength - 2;
                    long sum = 0;
                    while (i>=0) {
                        int u = header->path[i];
                        int v = header->path[i+1];
                        struct timeval start = header->stamps[2*i];
                        struct timeval finish = header->stamps[2*i+1];
                        long mtime = time_diff(start,finish) - sum;

                        latencies.insert(latencies.begin(), mtime);
                        sum += mtime;
                        i--;
                    }

                    for (size_t j = 1; j < header->pathLength; j++) {
                        path.push_back(header->path[j]);
                    }

                    // send back to monitor
                    agent->sendResultsToMonitorManager(path, latencies);


                    reached = true;
                } else {
                    std::cout << "keep going backward" << std::endl;
                    // keep going backward
                    header->nextHop--;
                    nextForwarderAddr.sin_addr.s_addr = header->path[header->nextHop];
                    gettimeofday(&stamp, NULL);
                    header->stamps[header->nextStamp] = stamp;
                    header->nextStamp -= 2;
                }
            }
        } else {
            // last node of the path, the packet must go backward
            std::cout << "last node, must go backward" << std::endl;
            header->direction = 1;
            header->nextHop--;
            nextForwarderAddr.sin_addr.s_addr = header->path[header->nextHop];
            gettimeofday(&stamp, NULL);
            header->stamps[header->nextStamp] = stamp;
            header->nextStamp--;
        }

        // if the packet is in transit
        if (!reached) {
            if (sendto(socketDescription, (char*) packet.msg, sizeof(LatencyProbeHeader), 0, (struct sockaddr*) &nextForwarderAddr, sizeof(sockaddr_in)) < 0) {
                perror("failed to forward latency probe packet");
            }
        }
    }
}
