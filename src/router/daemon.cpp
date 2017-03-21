/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file daemon.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include "daemon.h"

DaemonManager::DaemonManager(int assignedRole){
	//std::ifstream in("role");

	//if (in.is_open()) {
		//in >> assignedRole;
		role = assignedRole;
		//in.close();
	//}

	connectCU = false;
//    CUIP=inet_addr("10.0.15.20");
}

void DaemonManager::connectToCU(){
	//! Create socket
	socketCU = socket(AF_INET, SOCK_STREAM, 0);
	if (socketCU == -1) {
		std::cerr << "Could not create socket for incoming notifications"<< std::endl;
	}
	CUAddress.sin_addr.s_addr = CUIP;
	CUAddress.sin_family = AF_INET;
	CUAddress.sin_port = htons(CLOUD_COMMUNICATION_PORT);

	//! Connection TCP to the router
	std::cout << "Trying to connect to CU..." << std::endl;
	while (connect(socketCU, (struct sockaddr *) &CUAddress,sizeof(CUAddress)) < 0) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	std::cout << "Connected to CU" << std::endl;
}


void DaemonManager::CUCommunication(){
	connectToCU();

	newInfoCloudThread = std::thread(&DaemonManager::sendMyInfo, this);
	bool firstConnexion;
	int rcv = recv(socketCU, (void*)&firstConnexion, sizeof(firstConnexion), 0);

	if(firstConnexion==true && role!=0){
		firstUpdate();
	}

	std::cout << "wait for update" << std::endl;
	//! Wait message on behalf of the router
	InfoCloudPacket newCloud;
	while(true){
		int received = recv(socketCU, (void*)&newCloud, sizeof(newCloud), 0);
		std::cout << "update received" << std::endl;
		if (received == 0){
			std::cerr << "BC killed the connexion" << std::endl;
			close(this->socketCU);
			connectToCU();
		}
		updateInfo(newCloud);
	}
}


int DaemonManager::firstUpdate(){
	std::cout << "first Update " << std::endl;
	IPv4Address forwarderAddr, monitorAddr;
	bool firtUpdateDone = false;

	//!wait for the first update
	FirstUpdatePacket updatePacket;

	while(firtUpdateDone==false){
		int rcv = recv(socketCU, (void*)&updatePacket, sizeof(updatePacket), 0);

		forwarderAddr = updatePacket.forwarder;
		monitorAddr = updatePacket.monitor;

		std::cout << "forwarder : " << IPToString(forwarderAddr) << std::endl;
		std::cout << "monitor : " << IPToString(monitorAddr) << std::endl;

		if(updatePacket.destination == true){
			routingManager->index_destinations.push_back(forwarderAddr);
		}
		routingManager->index_nodes.push_back(monitorAddr);
		std::cout << "routingManager updated" << std::endl;

		if(updatePacket.length!=0){
			for(int i=0; i<updatePacket.length; i++){
				pathManager->mapClouds[forwarderAddr].push_back(updatePacket.client[i]);
				std::cout << "pathManager updated" << std::endl;
			}
		}
		pathManager->forwarderFromMonitor[monitorAddr] = forwarderAddr;
		pathManager->monitorFromForwarder[forwarderAddr] = monitorAddr;
		std::cout << "pathManager updated" << std::endl;

		firtUpdateDone = updatePacket.updateDone;
		std::cout << "firtUpdateDone "<< firtUpdateDone << std::endl;
	}
	std::cout << "First update done" << std::endl;
	return 0;
}


void DaemonManager::updateInfo(InfoCloudPacket newCloud){
	IPv4Address forwarderAddr, monitorAddr;

	forwarderAddr = newCloud.addr[0];
	if (newCloud.type == ROUTER) {
		monitorAddr = newCloud.addr[1];

		std::cout << "forwarder : " << IPToString(forwarderAddr) << std::endl;
		std::cout << "monitor : " << IPToString(monitorAddr) << std::endl;

		routingManager->index_nodes.push_back(monitorAddr);

		if (newCloud.destination==true){
			routingManager->index_destinations.push_back(forwarderAddr);
		}

		std::cout << "routingManager updated" << std::endl;

		pathManager->forwarderFromMonitor[monitorAddr] = forwarderAddr;
		pathManager->monitorFromForwarder[forwarderAddr] = monitorAddr;
		std::cout << "pathManager updated" << std::endl;
	}else{
		IPv4Address client = newCloud.addr[1];
		std::cout << "client : " << IPToString(client) << std::endl;
		pathManager->mapClouds[forwarderAddr].push_back(client);
		std::cout << "pathManager updated" << std::endl;
	}
}

void DaemonManager::sendMyInfo(){
	InfoCloudPacket firstPacket;

	while((pathManager->isMonitorConnected==false) || (pathManager->isForwarderConnected==false)){
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	firstPacket.type=ROUTER;
	firstPacket.role=role;
	std::cout << "firstPacket.type " << firstPacket.type << std::endl;

	std::cout << "forwarder : " << IPToString(pathManager->forwarderIPAddr) << std::endl;
	std::cout << "monitor : " << IPToString(pathManager->monitorManagerIPAddr) << std::endl;

	firstPacket.addr[0]=pathManager->forwarderIPAddr;
	firstPacket.addr[1]=pathManager->monitorManagerIPAddr;

	write(socketCU,(void*)&firstPacket, sizeof(firstPacket));

	std::cout << "Information sent" << std::endl;
	connectCU = true;

	InfoCloudPacket myCloud;
	IPv4Address addrIP;
	while(true){
		if(pathManager->isTransmissionAgentConnected==true){
			//myCloud.role=role;
			myCloud.type=TA;
			addrIP=pathManager->transmissionAgentIP;
			myCloud.addr[0]=addrIP;

			write(socketCU,(void*)&myCloud, sizeof(myCloud));
			std::cout << "write TransmissionAgentConnected " << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
}
