/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file forwarder.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include "forwarder.h"
#include "common/common.h"


DataForwarder::DataForwarder(std::string routerip): ForwarderInterface(DATA_PORT), routingTable(routerip){

}

//! Dispatch the packet
/*!
 * According to the value of the nextHop this function will either send it to route
 * or forward.
 */
void DataForwarder::dispatch(){
        if (packet.getPanaceaHeader()->nextHop == 0){
			routingTable.lookupforpath(packet);
		}
        forward();

}

//! Transfer the packet to the next destination
void DataForwarder::forward() {
    if ((packet.getPanaceaHeader()->nextHop - 1) == packet.getPanaceaHeader()->pathLength){
		// transfer to the reception agent
//        std::cout << "transfert to RA = " << std::endl;
		serverAddress.sin_addr.s_addr = packet.getIPv4Header()->daddr;
    } else {
		// Transfer to the next proxy
//        std::cout << "nextHop= " << packet.getPanaceaHeader()->nextHop << std::endl;
        serverAddress.sin_addr.s_addr = packet.getPanaceaHeader()->path[packet.getPanaceaHeader()->nextHop - 1];
//        std::cout << "transfert to next forwarder" << IPToString(serverAddress.sin_addr.s_addr) << std::endl;
    }

    packet.getPanaceaHeader()->nextHop++;

    //std::cout << "packet.length() =" << packet.lenght << std::endl;

	/*if (sendto(socketDescription, (char*) packet.msg, packet.lenght, 0,
			(struct sockaddr*) &serverAddress, sizeof(sockaddr_in)) < 0) {
		std::cerr << "sendto() failed fd here" << std::endl;
	} else {
        //std::cout << "sent" << std::endl;
	}*/
	
	sendto(socketDescription, (char*) packet.msg, packet.lenght, 0, (struct sockaddr*) &serverAddress, sizeof(sockaddr_in));
}


//! Wait for a data packet from the client and process it
void DataForwarder::process(){
    //std::cout << "enter DataForwarder::process() function" << std::endl;
	while(1){
        packet.lenght = recvfrom(socketDescription, packet.msg, BUFLENGTH /*sizeof(DataPacket)*/, 0, NULL, 0);
//        std::cout << "receive packet from " << IPToString(packet.getPanaceaHeader()->sourceAddress) << " of size=" << packet.lenght << std::endl;
        dispatch();
	}
}

//! start the forwarder process function and the router communication function in a separated thread
void DataForwarder::run(){
	routingTableThread = std::thread(&RoutingTable::routerComunication, &routingTable);
	process();
}
//-------------------------------------------------------------------------------


RoutingTable::RoutingTable(std::string routerip){
	if (routerip==" "){
		readConfigurationFromFile();
	}else{
		routerIP = inet_addr(routerip.c_str());
		std::cout << "router IP " << IPToString(routerIP) << std::endl;
	}
}

/*!
 * \brief Method used to update an entry in the routing table
 *
 * It is important to note that the structure map does not
 * authorize redundancy of key.
 */
void RoutingTable::setPath(IPv4Address dest, IPRoute route) {
	routingTable[dest]=route;
}

/**
 * Print the routing table.
 * Used only for test.
 *
 */
void RoutingTable::routingTablePrint(){
	std::cout << "Routing Table ==================================" << std::endl;
	for(std::map <IPv4Address, IPRoute >::iterator it=routingTable.begin();it!=routingTable.end();it++){
		std::cout << "Destination " << IPToString(it->first )<< " : ";
		for(size_t i=0;i<it->second.size();i++){
			std::cout << IPToString(it->second[i]) << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "=================================================" << std::endl;
}

/*!
 * \brief Method used to get a path leading towards a given destination.
 *
 * Depending on the value of destinationForwarderAddress contained in the package, we seek
 * the corresponding optimal path . If the destination is not in
 * the routing table we take the direct route.
 */
void RoutingTable::lookupforpath(DataPacket &p) {
	std::vector<IPv4Address> path;
    //std::cout << "looking for path toward dest=" << IPToString(p.getPanaceaHeader()->destinationForwarderAddress) << std::endl;
    if (routingTable.find(p.getPanaceaHeader()->destinationForwarderAddress) == routingTable.end()) {
        p.getPanaceaHeader()->path[0] = p.getPanaceaHeader()->destinationForwarderAddress;
        p.getPanaceaHeader()->pathLength = 1;
        p.getPanaceaHeader()->nextHop = 1;
        //std::cout << "path not available in routing table" << std::endl;
	}
	else {
        //std::cout << "path available in routing table" << std::endl;
        path = routingTable.find(p.getPanaceaHeader()->destinationForwarderAddress)->second;
        for(size_t i=0; i<path.size();i++){
        	p.getPanaceaHeader()->path[i]=path[i];
        }
        p.getPanaceaHeader()->pathLength = path.size();
        p.getPanaceaHeader()->nextHop = 1;
	}
}


void RoutingTable::readConfigurationFromFile()
{
    std::string line;
    std::ifstream in("routerip");
    if (in.is_open()){
        getline(in, line);
        routerIP = inet_addr(line.c_str());
        in.close();
    }

}


void RoutingTable::connectToRouter(){
	//! Create socket
	socketRouter = socket(AF_INET, SOCK_STREAM, 0);
	if (socketRouter == -1) {
		std::cerr << "Could not create socket for incoming router notifications"
				<< std::endl;
	}
	routerAddress.sin_addr.s_addr = routerIP;
	routerAddress.sin_family = AF_INET;
	routerAddress.sin_port = htons(ROUTER_FORWARDER_COMMUNICATION_PORT);

	//! Connection TCP to the router
	std::cout << "Trying to connect to router..." << std::endl;
	while (connect(socketRouter, (struct sockaddr *) &routerAddress,sizeof(routerAddress)) < 0) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	std::cout << "RoutingTable connected to router" << std::endl;
}

/*!
 * \brief listen to the socket connected to the router executable
 *
 * This function listen to the socket connected to the router executable and when it receive a new
 * update destination-route start the function setPath to update the table
 *
 */


void RoutingTable::routerComunication() {
	connectToRouter();

	//! Wait message on behalf of the router
	DestinationAndAssignedRoute newRoute;

	while (true) {
		int received = recv(socketRouter, (void*)&newRoute, sizeof(newRoute), 0);
		if (received == 0){
			std::cerr << "Router killed the connexion" << std::endl;
			close(this->socketRouter);
			connectToRouter();
		}

		IPRoute route;
		//std::cout << "New route for destination: " << IPToString(newRoute.destination) << " ";
		for(size_t i=0; i<newRoute.Lenght;i++){
			route.push_back(newRoute.route[i]);
			//std::cout << IPToString(route[i]) <<", ";
		}
		//std::cout << std::endl;

		setPath(newRoute.destination, route);

		routingTablePrint();
	}
	close(socketRouter);
}

