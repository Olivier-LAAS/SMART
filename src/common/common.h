/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 license            *
 *                                                                          *
 ****************************************************************************/

/**
 * @file common.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief Contain all the class, structures and parameters that are common for different executable
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <arpa/inet.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <ifaddrs.h>
#include <iostream>
#include <map>
#include <mutex>
#include <net/if.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>		// ipheader
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <set>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>			// memcpy
#include <sstream>
#include <sys/ioctl.h>
#include <sys/socket.h>		// socket
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

//! To be changed for IPv6
typedef in_addr_t IPv4Address;
typedef iphdr IPv4Header;

/************************
** Communication ports **
************************/
#define ROUTER_FORWARDER_COMMUNICATION_PORT 9549 //! PORT used for the communication between the router and the forwarder
#define ROUTER_TA_COMMUNICATION_PORT 9550        //! PORT used for the communication between the router and the TA
#define ROUTER_MONITOR_COMMUNICATION_PORT 9551   //! PORT used for communication between router and monitor
#define CLOUD_COMMUNICATION_PORT 9552            //! PORT used for communication between router and CU
#define DATA_PORT 9548				             //! PORT used for the transfer of data packet

#define DATA_MAX_PATH_LENGTH 2      //! maximum number of hops allowed between forwarder when searching for a path
#define BUFLENGTH 4096				//! define the size of the message 4096
#define PROBE_PACKETS_TO_SEND 5 	//! number of probe packet sent
#define MAX_CLIENTS_NUMBER 10       //! the maximum number of clients in each cloud (CU)
#define MAX_DESTINATIONS_NUMBER 10       //! the maximum number of destinations to be intercepted (TA)



typedef std::vector<IPv4Address> IPVector;
typedef IPVector IPPath;
typedef IPPath IPRoute; //! used when the path becomes selected


//! The type of metrics available for measurement
typedef enum {LATENCY_METRIC, BW_METRIC, LOSS_METRIC} MetricType;
typedef enum {ROUTER, TA} InfoType;



/*!
 *  @brief header of the panacea encapsulation
 *  @param destinationForwarderAddress The final forwarder address (destination cloud)
 *  @param sourceAddress The IP address of the source client machine
 *  @param path contains the path to get to destination
 *  @param pathLength
 *  @param nextHop it is incremented at each hop
 */
struct PanaceaHeader {
    IPv4Address destinationForwarderAddress;
    IPv4Address sourceAddress;
    IPv4Address path[DATA_MAX_PATH_LENGTH];
    unsigned int pathLength :DATA_MAX_PATH_LENGTH;
	unsigned int nextHop :DATA_MAX_PATH_LENGTH;
};


/*!
 * @brief Mother class of the packet sent over the Internet
 * @param msg A Table which that contains the message (transport level)
 * @param length Length of the message to be sent
 */
class Packet {
public:
	Packet();
	void display();

	unsigned char msg[BUFLENGTH];
	unsigned lenght;
};

//!
/* @brief Class data packet
 * @param originaliphdr Pointer towards the IP of the original packet
 *
 */
class DataPacket : public Packet {
public :
		DataPacket();
        PanaceaHeader * getPanaceaHeader();
        IPv4Header * getIPv4Header();

		IPv4Header *originaliphdr;
};

/*!
 * @brief Mother class of forwarders
 */
class ForwarderInterface {
public :
        ForwarderInterface(int PORT);

        //! Socket used for the sending of packet in transit
		int socketDescription;
        struct sockaddr_in serverAddress;
};


/*!
 * @brief the structure used by the router to communicate new routes to forwarder
 * @param Destination the destination for which we provide a path
 * @param route the path to attain Destination according *latency result*
 *
 */
typedef struct {
	IPv4Address destination;
	IPv4Address route[DATA_MAX_PATH_LENGTH];
	unsigned int Lenght :4;
}DestinationAndAssignedRoute;


/*!
 * @brief A structure used to inform client TA whether to intercept or not application packets
 * @param proxy the IP address of the destination forwarder (proxy is the old name)
 * @param destIP a table that contains all clients in the destination cloud that may be intercepted or not.
 * At the beginning doInterception is set to false for all destinations. Then only for destinations
 * that should be intercepted doInterception is set to true and destIP contains the addresses of these destinations.
 * Finally to stop interception, doInterception should be set to false and destIP contains the corresponding destinations
 */
typedef struct {
	bool doInterception;
	IPv4Address proxy; //! it should be renamed to forwarder
	IPv4Address destIP[MAX_DESTINATIONS_NUMBER];
	unsigned int lenght :MAX_DESTINATIONS_NUMBER;
}InterceptionNotificationPacket;



/************************
**       LATENCY       **
************************/

/**
 * @brief Contains a raw latency result (average RTT + number of packet received).
 * @param metric for the moment only LATENCY is implemented
 */
class Result{
public:
    Result(){
        averageResult = 0.0;
        packetsReceived = 0;
    }
    double averageResult;
    size_t packetsReceived;
    u_int32_t metric;
};


/**
 * @brief Contains the results of the edges on a path edges, used between monitor and path manager
 * It should be virtual class that each metric should implement
 */
class PathEdgesResult {
public:
    IPPath path;
    std::vector<double> edgesRTT;
	u_int32_t metric;

    double getTotalRTT() const { //should be renamed when other metric are implemented
        double totalRTT = 0.0;
        for (double l:edgesRTT)
            totalRTT += l;
        return totalRTT;
    }
};

/**
 * @brief The packet containing the order from Router to Monitor to do measures, used between router and monitor
 */

typedef struct {
public:
    u_int32_t metric;
    IPv4Address path[DATA_MAX_PATH_LENGTH];
    unsigned int pathLength :DATA_MAX_PATH_LENGTH;
} MonitoringInstructionPacket;


/**
 * @brief The packet storing path edges results, used between monitor and path manager
 */
typedef struct {
	u_int32_t metric;
    IPv4Address path[DATA_MAX_PATH_LENGTH];
    long edgesRTT[DATA_MAX_PATH_LENGTH]; //Should be renamed
    unsigned int pathLength :DATA_MAX_PATH_LENGTH;
} PathEdgesResultPacket;


/**
* @brief Get time difference in ms between two timeval
*/
long time_diff(struct timeval x , struct timeval y);

/**
 * @brief Get the string representation of an IPv4Address
 * @param addr the IPv4Address
 * @return string representation of addr
 */
std::string IPToString(const IPv4Address  addr);



/************************
**       CU            **
************************/
/*!
 * @brief Partial update of cloud information. This can be used either by the cloud or by the CU for minor updates
 * @param role the role of the cloud. Only useful when not client
 * @param destination indicates whether the IP is one of its destination or not
 * @param type the type of the entity (client, router(includes monitor and forwarder)
 * @param addr contains either forwarder then monitor (order is important) IP addresses, either one value if it is a client
 */
typedef struct {
	int role;
	u_int32_t type;
	bool destination;
	IPv4Address addr[2];
}InfoCloudPacket;

/*!
 * @brief a full update packet. It contains all cloud information. To be used when updating a whole cloud
 * @param destination indicates whether the IP is one of its destination or not
 * @param forwarder the IP address of forwarder
 * @param monitor the IP address of monitor
 * @param the IP addresses of all cloud clients
 * @param length the size of client array
 * @param updateDone a boolean to notify that is the last full update packet. All subsequent packets will be of partial type.
 */
typedef struct {
	//u_int32_t role;
	bool destination;
	IPv4Address forwarder ;
	IPv4Address monitor ;
	IPv4Address client[MAX_CLIENTS_NUMBER];
	unsigned int length :MAX_CLIENTS_NUMBER;
	bool updateDone;
}FirstUpdatePacket;

#endif
