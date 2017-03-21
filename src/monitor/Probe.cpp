/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/


#include "Probe.h"


ProbePacket::ProbePacket() : Packet() {
}

Probe::~Probe(){
#ifdef LOSS_PROBE
    if (metric == LOSS_METRIC)
        delete probeLoss;
#endif

#ifdef BW_PROBE
    if (metric == BW_METRIC)
	delete probeBW;
#endif

#ifdef LATENCY_PROBE
    if (metric == LATENCY_METRIC)
	delete probeLatency;
#endif
}

Probe::Probe(MonitorManager * monitorManager, MetricType metric_) : ProbeInterface(metric_)
{
std::cout<<"constru probe"<<std::endl;
    this->monitorManager = monitorManager;
    forwarder.setProbeAgent(this);

    socketDescription = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketDescription == -1) {
        std::cout << "Error creating UDP socket\n" << std::endl;
        exit(1);
    }

    metric = metric_;

#ifdef LOSS_PROBE
std::cout<<"constru probe"<<std::endl;
    if (metric_ == LOSS_METRIC)
        probeLoss = new ProbeLoss();
#endif

#ifdef BW_PROBE
    if (metric_ == BW_METRIC)
	probeBW = new ProbeBW();
#endif

#ifdef LATENCY_PROBE
    if (metric_ == LATENCY_METRIC)
	probeLatency = new ProbeLatency();
#endif

}

void Probe::startForwarder()
{
    // start the forwarder process() function in a separated thread
    forwarderThread = std::thread(&ProbeForwarder::process, forwarder);
}

void Probe::doMonitorPaths(const std::vector<IPPath> &routes)
{
    ProbePacket packet;

    struct sockaddr_in add;   /**< the address */
    bzero(&add, sizeof(add));
    add.sin_family = AF_INET;
    add.sin_port = htons(PROBE_PORT);

    for (const IPPath& route : routes) {
        memset(packet.msg, 0, BUFLENGTH);
        int pathLength = route.size() + 1;
        ProbeHeader* header = packet.getProbeHeader();

        // copy the route to the network packet
        header->pathLength = pathLength;
        header->path[0] = monitorManager->getMyAddr();
        for (size_t ii = 0; ii < route.size(); ii++) {
            header->path[ii+1] = route[ii];
        }

        // send PROBE_PACKETS_TO_SEND probe packets
        header->direction = 0;
        header->nextHop = 1;
        header->nextStamp = 1;
        add.sin_addr.s_addr = header->path[header->nextHop];
        for (int j = 0; j < 2 /*PROBE_PACKETS_TO_SEND*/; j++) { // send same result if still fresh, sending several packets might help if one is lost TODO
            double measure = getMeasure(&add);

            header->stamps[0] = measure;

            if (sendto(socketDescription, (char*) packet.msg, sizeof(ProbeHeader), 0, (struct sockaddr*) &add,  sizeof(add)) < 0) {
                perror("failed to send the probe packet");
            }
        }
    }
}

void Probe::sendResultsToMonitorManager(const IPPath &path, std::vector<double> bws)
{
    monitorManager->putProbeResults(metric, path, bws);
}

// FIXME should it be in the specific probe ?
// Creates a UDP socket on port 'PROBE_PORT'
ProbeForwarder::ProbeForwarder() : ForwarderInterface(PROBE_PORT)
{
}

void ProbeForwarder::process()
{
    struct sockaddr_in nextForwarderAddr;
    bzero(&nextForwarderAddr, sizeof(sockaddr));
    nextForwarderAddr.sin_family = AF_INET;
    nextForwarderAddr.sin_port = htons(PROBE_PORT);

    std::cout << "Probe forwarder running..." << std::endl;

    while (1) {

        // wait for incoming probe packet
        packet.lenght = recvfrom(socketDescription, packet.msg, sizeof(ProbePacket), 0, NULL, NULL);

        //! defines if the packet has came back to its source
        bool reached = false;
        ProbeHeader * header = packet.getProbeHeader();

        if (header->nextHop < header->pathLength-1) { // -1 to get rid of the source
            if (header->direction == 0) {
                // Forward
                std::cout << "forward" << std::endl;
                header->nextHop++;
                nextForwarderAddr.sin_addr.s_addr = header->path[header->nextHop];
                char *dest_addr = strdup(inet_ntoa(nextForwarderAddr.sin_addr)); // return IP
                std::cout << "### forwarding to "<<dest_addr<< " -- origin = "<<strdup(inet_ntoa(*(struct in_addr*)&header->path[0]))<<std::endl;
                double measure = agent->getMeasure(&nextForwarderAddr);
                header->stamps[header->nextStamp] = measure; 
                header->nextStamp += 1;
            } else {
                //backward
                if (header->nextHop == 0) {
                    // if the packet has came back to its source, get the final result
                    std::cout << "### back to origin" << std::endl;

                    // extract each edge measure 
                    std::vector<double> measures;
                    IPPath path;
                    int i = header->pathLength - 2;
                    while (i>=0) {
                        double edgeMeasure = header->stamps[i];

                        measures.insert(measures.begin(), edgeMeasure);
                        i--;
                    }

                    for (size_t j = 1; j < header->pathLength; j++) {
                        path.push_back(header->path[j]);
                    }

                    // send back to monitor
                    agent->sendResultsToMonitorManager(path, measures);

                    reached = true;
                } else {
                    std::cout << "keep going backward" << std::endl;
                    // keep going backward
                    header->nextHop--;
                    nextForwarderAddr.sin_addr.s_addr = header->path[header->nextHop];
                    header->nextStamp -= 1; 
                }
            }
        } else {
            // last node of the path, the packet must go backward
            std::cout << "last node, must go backward" << std::endl;
            header->direction = 1;
            header->nextHop--;
            nextForwarderAddr.sin_addr.s_addr = header->path[header->nextHop];
            header->nextStamp--;
        }

        // if the packet is in transit
        if (!reached) {
            if (sendto(socketDescription, (char*) packet.msg, sizeof(ProbeHeader), 0, (struct sockaddr*) &nextForwarderAddr, sizeof(sockaddr_in)) < 0) {
                perror("failed to forward latency probe packet");
            }
        }
    }
}

double Probe::getMeasure(struct sockaddr_in *add){
	double value;
	std::cerr<<"Start measure"<<std::endl;
#ifdef LOSS_PROBE
	if (metric == LOSS_METRIC) 
		return probeLoss->getLoss(add);
#endif

#ifdef BW_PROBE
	if (metric == BW_METRIC)
		return probeBW->getBw(add);
#endif

#ifdef LATENCY_PROBE
	if (metric == LATENCY_METRIC)
		return probeLatency->getLatency(add);
#endif
	std::cerr << "No probe found for this metric : " << metric << std::endl;
	return -1; //FIXME exit ?
}



